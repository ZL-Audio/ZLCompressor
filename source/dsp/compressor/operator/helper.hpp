// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <cmath>

namespace zlCompressor {
    template<typename Type>
    static Type decibelsToGain(const Type decibels) {
        return std::pow(static_cast<Type>(10.0), decibels * static_cast<Type>(0.05));
    }

    template <typename Type>
    static Type gainToDecibels (const Type gain) {
        return std::log10(std::max(gain, static_cast<Type>(1e-10)))  * static_cast<Type>(20.0);
    }
}
