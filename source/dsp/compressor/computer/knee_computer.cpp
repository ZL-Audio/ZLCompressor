// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

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
    void KneeComputer<FloatType>::prepareBuffer() {
        if (toInterpolate.exchange(false)) {
            interpolate();
        }
    }

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::eval(FloatType x) {
        if (x <= lowThres) {
            return x;
        } else if (x >= highThres) {
            return juce::jlimit(x - currentBound, x + currentBound,
                paras[2] + paras[3] * x + paras[4] * x * x);
        } else {
            const auto xx = x + paras[1];
            return juce::jlimit(x - currentBound, x + currentBound,
                                x + paras[0] * xx * xx);
        }
    }

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::process(FloatType x) {
        return eval(x) - x;
    }

    template<typename FloatType>
    void KneeComputer<FloatType>::interpolate() {
        const auto currentThreshold = threshold.load();
        const auto currentKneeW = kneeW.load();
        const auto currentRatio = ratio.load();
        currentBound = bound.load();
        const auto currentCurve = curve.load();
        lowThres = currentThreshold - currentKneeW;
        highThres = currentThreshold + currentKneeW;
        paras[0] = FloatType(1) / currentRatio - FloatType(1);
        paras[1] = -lowThres;
        paras[0] *= FloatType(1) / (currentKneeW * FloatType(4));
        if (currentCurve >= FloatType(0)) {
            const auto alpha = FloatType(1) - currentCurve, beta = currentCurve;
            linearCurve.setPara(currentThreshold, currentRatio, currentKneeW);
            downCurve.setPara(currentThreshold, currentRatio, currentKneeW);
            paras[2] = alpha * linearCurve.c + beta * downCurve.c;
            paras[3] = alpha * linearCurve.b + beta * downCurve.b;
            paras[4] = alpha * linearCurve.a + beta * downCurve.a;
        } else {
            const auto alpha = FloatType(1) + currentCurve, beta = -currentCurve;
            linearCurve.setPara(currentThreshold, currentRatio, currentKneeW);
            upCurve.setPara(currentThreshold, currentRatio, currentKneeW);
            paras[2] = alpha * linearCurve.c + beta * upCurve.c;
            paras[3] = alpha * linearCurve.b + beta * upCurve.b;
            paras[4] = alpha * linearCurve.a + beta * upCurve.a;
        }
    }

    template
    class KneeComputer<float>;

    template
    class KneeComputer<double>;
} // KneeComputer
