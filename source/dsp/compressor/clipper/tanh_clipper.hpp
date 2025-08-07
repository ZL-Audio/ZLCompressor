// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <iostream>

#include "../../chore/chore.hpp"
#include "../computer/knee_computer.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    template<typename FloatType>
    class TanhClipper {
    public:
        TanhClipper() = default;

        void update(FloatType x1, FloatType y1, FloatType y2) {
            const auto k1 = getK(static_cast<double>(x1), static_cast<double>(y1));
            const auto k2 = getK(1.0, static_cast<double>(y2));
            k_ = std::clamp(std::min(k1, k2), kKLeft * 1.01, kKRight * 0.99);
            updateActualK();
        }

        void prepareBuffer() {
            if (to_update_wet_.exchange(false, std::memory_order::acquire)) {
                c_wet_ = wet_.load(std::memory_order::relaxed) / 100.0;
                is_on_ = c_wet_ > 1e-5;
                if (is_on_) {
                    updateActualK();
                }
            }
        }

        void process(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto v = static_cast<double>(buffer[i]);
                buffer[i] = static_cast<FloatType>(std::tanh(v * k1_) * k2_);
            }
        }

        FloatType processSample(FloatType x) const {
            return static_cast<FloatType>(std::tanh(static_cast<double>(x) * k1_) * k2_);
        }

        void setWet(const FloatType wet) {
            wet_.store(std::clamp(static_cast<double>(wet), 0.0, 100.0), std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
        }

        bool getIsON() const {
            return is_on_;
        }

    private:
        static constexpr double kLowThres = 0.989, kHighThres = 0.991;
        static constexpr double kKLeft = 0.1, kKRight = 100.0;

        std::atomic<double> wet_{0.0};
        std::atomic<bool> to_update_wet_{true};
        double c_wet_{0.0};
        bool is_on_{false};
        double k_{1.0}, k1_{1.0}, k2_{1.0};

        void updateActualK() {
            const auto medium = std::min(k_, 1.0);
            const auto right = std::max(k_, 1.0);
            if (c_wet_ < 0.5) {
                const auto wet = c_wet_ * 2.0;
                k1_ = wet * (medium - kKLeft) + kKLeft;
            } else {
                const auto wet = (c_wet_ - 0.5) * 2.0;
                k1_ = wet * (right - medium) + medium;
            }
            k2_ = 1.0 / k1_;
        }

        static double evalP(const double x, const double y, const double k) {
            return std::tanh(k * x) / k / y;
        }

        static double getK(const double x, const double y) {
            auto k_left = kKLeft;
            auto k_right = kKRight;
            if (evalP(x, y, k_right) >= kLowThres) {
                return k_right;
            } else if (evalP(x, y, k_left) <= kHighThres) {
                return k_left;
            }
            size_t i = 0;
            while (i < 100) {
                i += 1;
                const auto k = std::sqrt(k_left * k_right);
                const auto p = evalP(x, y, k);
                if (p > kHighThres) {
                    k_left = k;
                } else if (p < kLowThres) {
                    k_right = k;
                } else {
                    return k;
                }
            }
            return std::sqrt(k_left * k_right);
        }
    };
}
