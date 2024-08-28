// Copyright (C) 2024 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "knee_computer.hpp"

namespace zlCompressor {
    template<typename FloatType>
    KneeComputer<FloatType>::KneeComputer(const KneeComputer<FloatType> &c) {
        setThreshold(c.getThreshold());
        setRatio(c.getRatio());
        setKneeW(c.getKneeW());
        setCurve(c.getCurve());
        setBound(c.getBound());
    }

    template<typename FloatType>
    KneeComputer<FloatType>::~KneeComputer() = default;

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::eval(FloatType x) {
        if (toInterpolate.exchange(false)) {
            interpolate();
        }
        if (x <= lowThres) {
            return x;
        } else if (x >= highThres) {
            const auto linearV = paras[5] + x * currentSlope;
            const auto quadV = paras[3] * x * x + paras[4];
            return juce::jlimit(x - currentBound, x + currentBound,
                                currentOppositeCurve * linearV + currentCurve * quadV);
        } else {
            const auto xx = x + paras[1];
            return juce::jlimit(x - currentBound, x + currentBound,
                                x + paras[0] * xx * xx * paras[2]);
        }
    }

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::process(FloatType x) {
        return eval(x) - x;
    }

    template<typename FloatType>
    void KneeComputer<FloatType>::interpolate() {
        currentThreshold = threshold.load();
        const auto kneeW_ = kneeW.load();
        currentRatio = ratio.load();
        currentSlope = FloatType(1) / currentRatio;
        currentBound = bound.load();
        currentCurve = curve.load();
        currentOppositeCurve = FloatType(1) - currentCurve;
        lowThres = currentThreshold - kneeW_;
        highThres = currentThreshold + kneeW_;
        paras[0] = FloatType(1) / currentRatio - FloatType(1);
        paras[1] = -lowThres;
        paras[2] = FloatType(1) / (kneeW_ * FloatType(4));
        paras[3] = 1 / std::min(currentThreshold + kneeW_, FloatType(0.0001)) / currentRatio;
        paras[4] = currentThreshold + (kneeW_ - currentThreshold) / FloatType(2) / currentRatio;
        paras[5] = currentThreshold * (1 - currentSlope);
    }

    template
    class KneeComputer<float>;

    template
    class KneeComputer<double>;
} // KneeComputer
