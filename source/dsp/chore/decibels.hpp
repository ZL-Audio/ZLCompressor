#pragma once

#include <cmath>
#include <algorithm>

namespace zldsp::chore {
    template<typename FloatType>
    FloatType decibelsToGain(const FloatType value) {
        return std::pow(FloatType(10), value * FloatType(0.05));
    }

    template<typename FloatType>
    FloatType gainToDecibels(const FloatType value) {
        return FloatType(20) * std::log10(std::max(value, FloatType(1e-12)));
    }
}
