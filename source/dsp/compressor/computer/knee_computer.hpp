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
     * a computer that computes the current compression
     * @tparam FloatType
     */
    template<typename FloatType, bool OutputDiff = false>
    class KneeComputer final : public ComputerBase<FloatType> {
    public:
        KneeComputer() = default;

        bool prepareBuffer() override {
            if (to_interpolate_.exchange(false)) {
                interpolate();
                return true;
            }
            return false;
        }

        void copyFrom(KneeComputer &other) {
            low_th_ = other.low_th_;
            high_th_ = other.high_th_;
            para_mid_g0_ = other.para_mid_g0_;
            para_high_g0_ = other.para_high_g0_;
        }

        FloatType eval(FloatType x) override {
            if (x <= low_th_) {
                return OutputDiff ? FloatType(0) : x;
            } else if (x >= high_th_) {
                x = std::min(x, FloatType(0));
                return (para_high_g0_[0] * x + para_high_g0_[1]) * x + para_high_g0_[2];
            } else {
                return (para_mid_g0_[0] * x + para_mid_g0_[1]) * x + para_mid_g0_[2];
            }
        }

        inline void setThreshold(FloatType v) {
            threshold_.store(v);
            to_interpolate_.store(true);
        }

        inline FloatType getThreshold() const { return threshold_.load(); }

        inline void setRatio(FloatType v) {
            ratio_.store(std::max(FloatType(1), v));
            to_interpolate_.store(true);
        }

        inline FloatType getRatio() const { return ratio_.load(); }

        inline void setKneeW(FloatType v) {
            knee_w_.store(std::max(v, FloatType(0.01)));
            to_interpolate_.store(true);
        }

        inline FloatType getKneeW() const { return knee_w_.load(); }

        inline void setCurve(FloatType v) {
            curve_.store(std::clamp(v, FloatType(-1), FloatType(1)));
            to_interpolate_.store(true);
        }

        inline FloatType getCurve() const { return curve_.load(); }

    private:
        struct LinearCurve {
            FloatType b, c;

            void setPara(FloatType t, FloatType r, FloatType) {
                b = FloatType(1) / r;
                c = t * (FloatType(1) - FloatType(1) / r);
            }
        };

        struct DownCurve {
            FloatType a, c;
            static constexpr FloatType b{FloatType(0)};

            void setPara(FloatType t, FloatType r, FloatType w) {
                a = FloatType(0.5) / (r * std::min(t + w, FloatType(-0.0001)));
                c = FloatType(0.5) * (w - t) / r + t;
            }
        };

        struct UpCurve {
            FloatType a, c;
            static constexpr FloatType b{FloatType(1)};

            void setPara(FloatType t, FloatType r, FloatType w) {
                a = FloatType(0.5) * (FloatType(1) - r) / (r * std::min(t + w, FloatType(-0.0001)));
                c = FloatType(0.5) * (FloatType(1) - r) * (w - t) / r;
            }
        };

        LinearCurve linear_curve_;
        DownCurve down_curve_;
        UpCurve up_curve_;
        std::atomic<FloatType> threshold_{-18}, ratio_{2};
        std::atomic<FloatType> knee_w_{FloatType(0.25)}, curve_{0};
        FloatType low_th_{0}, high_th_{0};
        std::array<FloatType, 3> para_mid_g0_, para_high_g0_;
        std::atomic<bool> to_interpolate_{true};

        void interpolate() {
            const auto currentThreshold = threshold_.load();
            const auto currentKneeW = knee_w_.load();
            const auto currentRatio = ratio_.load();
            const auto currentCurve = curve_.load();
            low_th_ = currentThreshold - currentKneeW;
            high_th_ = currentThreshold + currentKneeW; {
                // update mid curve parameters
                const auto a0 = (FloatType(1) / currentRatio - FloatType(1)) / (currentKneeW * FloatType(4));
                const auto a1 = -low_th_;
                para_mid_g0_[0] = a0;
                const auto a0a1 = a0 * a1;
                if (OutputDiff) {
                    para_mid_g0_[1] = FloatType(2) * a0a1;
                } else {
                    para_mid_g0_[1] = FloatType(2) * a0a1 + FloatType(1);
                }
                para_mid_g0_[2] = a0a1 * a1;
            }
            if (currentCurve >= FloatType(0)) {
                const auto alpha = FloatType(1) - currentCurve, beta = currentCurve;
                linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                down_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                para_high_g0_[2] = alpha * linear_curve_.c + beta * down_curve_.c;
                para_high_g0_[1] = alpha * linear_curve_.b + beta * down_curve_.b;
                para_high_g0_[0] = beta * down_curve_.a;
            } else {
                const auto alpha = FloatType(1) + currentCurve, beta = -currentCurve;
                linear_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                up_curve_.setPara(currentThreshold, currentRatio, currentKneeW);
                para_high_g0_[2] = alpha * linear_curve_.c + beta * up_curve_.c;
                para_high_g0_[1] = alpha * linear_curve_.b + beta * up_curve_.b;
                para_high_g0_[0] = beta * up_curve_.a;
            }
            if (OutputDiff) {
                para_high_g0_[1] -= FloatType(1);
            }
        }
    };
} // KneeComputer
