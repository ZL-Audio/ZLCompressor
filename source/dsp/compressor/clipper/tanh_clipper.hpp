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

            k_ = std::min(k1, k2);
            k_reciprocal_ = 1.0 / k_;
            c_wet_mul_ = k_reciprocal_ * c_wet_;
        }

        void prepareBuffer() {
            c_wet_ = wet_.load(std::memory_order::relaxed) / 100.0;
            cc_wet_ = 1.0 - c_wet_;
            is_on_ = c_wet_ > 1e-5;
            c_wet_mul_ = k_reciprocal_ * c_wet_;
        }

        void process(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto v = static_cast<double>(buffer[i]);
                buffer[i] = static_cast<FloatType>(std::tanh(v * k_) * c_wet_mul_ + v * cc_wet_);
            }
        }

        void setWet(const FloatType wet) {
            wet_.store(static_cast<double>(wet), std::memory_order::relaxed);
        }

        bool getIsON() const {
            return is_on_;
        }

    private:
        static constexpr double kLowThres = 0.989;
        static constexpr double kHighThres = 0.991;

        std::atomic<double> wet_{0.0};
        double c_wet_{0.0}, cc_wet_{1.0};
        double c_wet_mul_{0.0};
        bool is_on_{false};
        double k_{1.0}, k_reciprocal_{1.0};

        static double evalP(const double x, const double y, const double k) {
            return std::tanh(k * x) / k / y;
        }

        static double getK(const double x, const double y) {
            auto k_left = 0.01;
            auto k_right = 100.0;
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
