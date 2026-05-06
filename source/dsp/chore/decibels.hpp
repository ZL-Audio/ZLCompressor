// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <algorithm>

namespace zldsp::chore {
    inline constexpr double kLogMin = 1e-12;
    inline constexpr double kLogMul = 8.6858896380650365530;

    inline constexpr double kLogSqrMin = 1e-24;
    inline constexpr double kLogSqrMul = 4.3429448190325182765;

    template <typename FloatType>
    FloatType decibelsToGain(const FloatType value) {
        return std::pow(FloatType(10), value * FloatType(0.05));
    }

    template <typename FloatType>
    FloatType gainToDecibels(const FloatType value) {
        return FloatType(20) * std::log10(std::max(value, FloatType(1e-12)));
    }

    template <typename FloatType>
    FloatType squareGainToDecibels(const FloatType value) {
        return FloatType(10) * std::log10(std::max(value, FloatType(1e-24)));
    }
}
