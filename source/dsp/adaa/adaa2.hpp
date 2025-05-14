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

namespace zldsp::adaa {
    template<typename FloatType>
    class ADAA2 {
    public:
        ADAA2() = default;

        virtual ~ADAA2() = default;

        template <bool UseADAA>
        FloatType processSample(FloatType x2) {
            if (!UseADAA) {
                return g0(x2);
            }
            FloatType y{};
            const auto g2_x2 = g2(x2);
            const auto x2_x1_delta = x2 - x1_;
            FloatType d1 = std::abs(x2_x1_delta) > kEps
                               ? (g2_x2 - g2_x1_) / (x2_x1_delta) // x2 - x1 normal condition
                               : g1(FloatType(0.5) * (x2 + x1_)); // x2 - x1 ill condition
            const auto x2_x0_delta = x2 - x0_;
            if (std::abs(x2_x0_delta) > kEps) { // x2 - x0 normal condition
                y = FloatType(2) / (x2_x0_delta) * (d1 - d0_);
            } else { // x2 - x0 ill condition
                const auto x_bar = FloatType(0.5) * (x2 + x0_);
                const auto delta = x_bar - x1_;
                if (std::abs(delta) > kEps) { // delta normal condition
                    const auto delta_r = FloatType(1) / delta;
                    y = FloatType(2) * delta_r * (g1(x_bar) + (g2_x1_ - g2(x_bar)) * delta_r);
                } else { // delta ill condition
                    y = g0(FloatType(0.5) * (x_bar + x1_));
                }
            }
            // save states
            x0_ = x1_;
            x1_ = x2;
            d0_ = d1;
            g2_x1_ = g2_x2;
            return y;
        }

    protected:
        /**
         *
         * @param x
         * @return the function value at x
         */
        virtual FloatType g0(FloatType x) = 0;

        /**
         *
         * @param x
         * @return the first-order antiderivative function value at x
         */
        virtual FloatType g1(FloatType x) = 0;

        /**
         *
         * @param x
         * @return the second-order antiderivative function value at x
         */
        virtual FloatType g2(FloatType x) = 0;

    private:
        FloatType x0_{}, x1_{}, d0_{}, g2_x1_{};
        static constexpr FloatType kEps = FloatType(1e-10);
    };
}
