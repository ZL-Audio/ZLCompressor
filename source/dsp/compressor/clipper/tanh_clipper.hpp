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
            x1 = std::clamp(x1, static_cast<FloatType>(1e-5), static_cast<FloatType>(0.99999));
            y1 = std::clamp(y1, static_cast<FloatType>(1e-5), static_cast<FloatType>(0.99999));
            y2 = std::clamp(y2, static_cast<FloatType>(1e-5), static_cast<FloatType>(1));
            const auto k1 = getK(x1, y1);
            const auto k2 = getK(static_cast<FloatType>(1), y2);

            k_ = std::min(k1, k2);
            k_reciprocal_ = static_cast<FloatType>(1) / k_;
            c_wet_mul_ = k_reciprocal_ * c_wet_;
        }

        void prepareBuffer() {
            c_wet_ = wet_.load(std::memory_order::relaxed) / 100.f;
            cc_wet_ = static_cast<FloatType>(1) - c_wet_;
            is_on_ = c_wet_ > static_cast<FloatType>(1e-5);
            c_wet_mul_ = k_reciprocal_ * c_wet_;
        }

        void process(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto v = buffer[i];
                buffer[i] = std::tanh(v * k_) * c_wet_mul_ + v * cc_wet_;
            }
        }

        void setWet(const FloatType wet) {
            wet_.store(wet, std::memory_order::relaxed);
        }

        bool getIsON() const {
            return is_on_;
        }

    private:
        static constexpr FloatType kLowThres = static_cast<FloatType>(0.9875);
        static constexpr FloatType kHighThres = static_cast<FloatType>(0.9925);

        std::atomic<FloatType> wet_{static_cast<FloatType>(0)};
        FloatType c_wet_, cc_wet_{static_cast<FloatType>(1)};
        FloatType c_wet_mul_{};
        bool is_on_{false};
        FloatType k_{static_cast<FloatType>(1)}, k_reciprocal_{static_cast<FloatType>(1)};

        static FloatType evalP(const FloatType x, const FloatType y, const FloatType k) {
            return std::tanh(k * x) / k / y;
        }

        static FloatType getK(const FloatType x, const FloatType y) {
            auto k_left = static_cast<FloatType>(1e-2);
            auto k_right = static_cast<FloatType>(100);
            if (evalP(x, y, k_right) >= kLowThres) {
                return k_right;
            } else if (evalP(x, y, k_left) <= kHighThres) {
                return k_left;
            }
            size_t i = 0;
            while (i < 100) {
                i += 1;
                const FloatType k = std::sqrt(k_left * k_right);
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
