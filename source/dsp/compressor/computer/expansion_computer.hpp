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
            para_below_g0_ = other.para_below_g0_;
        }

        FloatType eval(FloatType x) override {
            if (x >= high_th_) {
                if constexpr (OutputDiff) {
                    return FloatType(0);
                }
                else {
                    return x;
                }
            } else if (x > low_th_) {
                return (para_mid_g0_[0] * x + para_mid_g0_[1]) * x + para_mid_g0_[2];
            } else if (x > FloatType(-120)) {
                return (para_low_g0_[0] * x + para_low_g0_[1]) * x + para_low_g0_[2];
            } else {
                return para_below_g0_;
            }
        }

        inline void setThreshold(FloatType v) {
            threshold_.store(v, std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getThreshold() const { return threshold_.load(std::memory_order::relaxed); }

        inline void setRatio(FloatType v) {
            ratio_.store(std::max(FloatType(1), v), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getRatio() const { return ratio_.load(std::memory_order::relaxed); }

        inline void setKneeW(FloatType v) {
            knee_w_.store(std::max(v, FloatType(0.01)), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getKneeW() const { return knee_w_.load(std::memory_order::relaxed); }

        inline void setCurve(FloatType v) {
            curve_.store(std::clamp(v, FloatType(-1), FloatType(1)), std::memory_order::relaxed);
            to_interpolate_.store(true, std::memory_order::release);
        }

        inline FloatType getCurve() const { return curve_.load(std::memory_order::relaxed); }

    private:
        struct LinearCurve {
            FloatType b, c;

            void setPara(FloatType t, FloatType r, FloatType) {
                b = r;
                c = t * (r - FloatType(1));
            }
        };

        struct DownCurve {
            FloatType a, b, c;

            void setPara(FloatType t_, FloatType r_, FloatType w_) {
                const auto t = static_cast<double>(t_);
                const auto r = static_cast<double>(r_);
                const auto w = static_cast<double>(w_);
                const auto a_ = (r - 100.0) / (2.0 * (120.0 + t - w));
                const auto b_ = 100.0 - 240.0 * a_;
                const auto temp = t - w;
                const auto c_ = t - r * w - (a_ * temp + b_) * temp;
                a = static_cast<FloatType>(a_);
                b = static_cast<FloatType>(b_);
                c = static_cast<FloatType>(c_);
            }
        };

        struct UpCurve {
            FloatType a, b, c;

            void setPara(FloatType t_, FloatType r_, FloatType w_) {
                const auto t = static_cast<double>(t_);
                const auto r = static_cast<double>(r_);
                const auto w = static_cast<double>(w_);

                const auto a_ = (r - 1.0) / (2.0 * (120.0 + t - w));
                const auto b_ = 1.0 - 240.0 * a_;
                const auto temp = t - w;
                const auto c_ = t - r * w - (a_ * temp + b_) * temp;
                a = static_cast<FloatType>(a_);
                b = static_cast<FloatType>(b_);
                c = static_cast<FloatType>(c_);
            }
        };

        LinearCurve linear_curve_;
        DownCurve down_curve_;
        UpCurve up_curve_;
        std::atomic<FloatType> threshold_{-18}, ratio_{2};
        std::atomic<FloatType> knee_w_{FloatType(0.25)}, curve_{0};
        FloatType low_th_{0}, high_th_{0};
        std::array<FloatType, 3> para_low_g0_{}, para_mid_g0_{};
        FloatType para_below_g0_{};
        std::atomic<bool> to_interpolate_{true};

        void interpolate() {
            const auto current_threshold = threshold_.load(std::memory_order::relaxed);
            const auto current_knee_w = knee_w_.load(std::memory_order::relaxed);
            const auto current_ratio = ratio_.load(std::memory_order::relaxed);
            const auto current_curve = curve_.load(std::memory_order::relaxed);
            low_th_ = current_threshold - current_knee_w;
            high_th_ = current_threshold + current_knee_w;
            {
                // update mid curve parameters
                const auto a0 = (FloatType(1) / current_ratio - FloatType(1)) / (current_knee_w * FloatType(4));
                const auto a1 = -low_th_;
                para_mid_g0_[0] = a0;
                const auto a0a1 = a0 * a1;
                if constexpr (OutputDiff) {
                    para_mid_g0_[1] = FloatType(2) * a0a1;
                }
                else {
                    para_mid_g0_[1] = FloatType(2) * a0a1 + FloatType(1);
                }
                para_mid_g0_[2] = a0a1 * a1;
            }
            {
                // update low curve parameters
                if (current_curve >= FloatType(0)) {
                    const auto alpha = FloatType(1) - current_curve, beta = current_curve;
                    linear_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    down_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    para_low_g0_[2] = alpha * linear_curve_.c + beta * down_curve_.c;
                    para_low_g0_[1] = alpha * linear_curve_.b + beta * down_curve_.b;
                    para_low_g0_[0] = beta * down_curve_.a;
                }
                else {
                    const auto alpha = FloatType(1) + current_curve, beta = -current_curve;
                    linear_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    up_curve_.setPara(current_threshold, current_ratio, current_knee_w);
                    para_low_g0_[2] = alpha * linear_curve_.c + beta * up_curve_.c;
                    para_low_g0_[1] = alpha * linear_curve_.b + beta * up_curve_.b;
                    para_low_g0_[0] = beta * up_curve_.a;
                }
                if constexpr (OutputDiff) {
                    para_low_g0_[1] -= FloatType(1);
                }
            }
            para_below_g0_ = (para_low_g0_[0] * FloatType(-120) + para_low_g0_[1]) * FloatType(-120) + para_low_g0_[2];
        }
    };
}