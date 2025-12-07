// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <vector>
#include <array>
#include <algorithm>

#include "computer_base.hpp"

namespace zldsp::compressor {
    /**
     * a computer that computes the current expansion
     * @tparam FloatType
     * @tparam OutputDiff
     */
    template <typename FloatType, bool OutputDiff = false>
    class ExpansionComputer final : public ComputerBase<FloatType> {
    public:
        ExpansionComputer() = default;

        bool prepareBuffer() override {
            if (to_interpolate_.exchange(false, std::memory_order::acquire)) {
                interpolate();
                return true;
            }
            return false;
        }

        void copyFrom(ExpansionComputer& other) {
            low_th_ = other.low_th_;
            high_th_ = other.high_th_;
            para_mid_g0_ = other.para_mid_g0_;
            para_low_g0_ = other.para_low_g0_;
        }

        FloatType eval(FloatType x) override {
            if (x >= high_th_) {
                if constexpr (OutputDiff) {
                    return FloatType(0);
                } else {
                    return x;
                }
            } else if (x > low_th_) {
                return (para_mid_g0_[0] * x + para_mid_g0_[1]) * x + para_mid_g0_[2];
            } else if (x > c_floor_) {
                const auto x_s = x - c_floor_;
                if constexpr (OutputDiff) {
                    return (para_low_g0_[0] * x_s + para_low_g0_[1]) * x_s * x_s;
                } else {
                    return (para_low_g0_[0] * x_s + para_low_g0_[1]) * x_s * x_s + x;
                }
            } else {
                if constexpr (OutputDiff) {
                    return FloatType(0);
                } else {
                    return x;
                }
            }
        }

        void setThreshold(const FloatType v) {
            threshold_.store(v, std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        FloatType getThreshold() const { return threshold_.load(std::memory_order::relaxed); }

        void setRatio(const FloatType v) {
            ratio_.store(std::max(FloatType(1), v), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        FloatType getRatio() const { return ratio_.load(std::memory_order::relaxed); }

        void setKneeW(const FloatType v) {
            knee_w_.store(std::max(v, FloatType(0.01)), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        FloatType getKneeW() const { return knee_w_.load(std::memory_order::relaxed); }

        void setFloor(const FloatType v) {
            floor_.store(v, std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

    private:
        std::atomic<FloatType> threshold_{-18}, ratio_{2};
        std::atomic<FloatType> knee_w_{FloatType(0.25)};
        std::atomic<FloatType> floor_{-140};
        FloatType low_th_{0}, high_th_{0}, c_floor_{-140};
        std::array<FloatType, 3> para_mid_g0_{};
        std::array<FloatType, 2> para_low_g0_{};
        std::atomic<bool> to_interpolate_{true};

        void interpolate() {
            const auto t = threshold_.load(std::memory_order::relaxed);
            const auto w = knee_w_.load(std::memory_order::relaxed);
            const auto r = ratio_.load(std::memory_order::relaxed);
            low_th_ = t - w;
            high_th_ = t + w;
            c_floor_ = std::min(floor_.load(std::memory_order::relaxed), low_th_ - FloatType(1.f));
            {
                // update mid curve parameters
                const auto a0 = (1.0 - static_cast<double>(r)) / (
                    4.0 * static_cast<double>(w));
                const auto a1 = -static_cast<double>(high_th_);
                para_mid_g0_[0] = static_cast<FloatType>(a0);
                const auto a0a1 = a0 * a1;
                if constexpr (OutputDiff) {
                    para_mid_g0_[1] = static_cast<FloatType>(2.0 * a0a1);
                } else {
                    para_mid_g0_[1] = static_cast<FloatType>(2.0 * a0a1 + 1.0);
                }
                para_mid_g0_[2] = static_cast<FloatType>(a0a1 * a1);
            }
            {
                const auto scale = r - FloatType(1.0);
                const auto common_base = -c_floor_ + t;
                const auto denom_base = common_base - w;
                const auto inv_denom2 = FloatType(1.0) / (denom_base * denom_base);
                const auto inv_denom3 = inv_denom2 / denom_base;
                para_low_g0_[0] = scale * (common_base + w) * inv_denom3;
                para_low_g0_[1] = -scale * (common_base + w + w) * inv_denom2;
            }
        }
    };
}
