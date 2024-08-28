// Copyright (C) 2024 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPRESSOR_COMPUTER_H
#define COMPRESSOR_COMPUTER_H

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlCompressor {
    /**
     * a computer that computes the current compression
     * @tparam FloatType
     */
    template<typename FloatType>
    class KneeComputer final {
    public:
        KneeComputer() = default;

        KneeComputer(const KneeComputer<FloatType> &c);

        ~KneeComputer();

        FloatType eval(FloatType x);

        /**
         * computes the current compression
         * @param x input level (in dB)
         * @return current compression (in dB)
         */
        FloatType process(FloatType x);

        inline void setThreshold(FloatType v) {
            threshold.store(v);
            toInterpolate.store(true);
        }

        inline FloatType getThreshold() const { return threshold.load(); }

        inline void setRatio(FloatType v) {
            ratio.store(v);
            toInterpolate.store(true);
        }

        inline FloatType getRatio() const { return ratio.load(); }

        inline void setKneeW(FloatType v) {
            kneeW.store(std::max(v, FloatType(0.01)));
            toInterpolate.store(true);
        }

        inline FloatType getKneeW() const { return kneeW.load(); }

        inline void setCurve(FloatType v) {
            curve.store(v);
            toInterpolate.store(true);
        }

        inline FloatType getCurve() const { return curve.load(); }

        inline void setBound(FloatType v) {
            bound.store(v);
        }

        inline FloatType getBound() const { return bound.load(); }

    private:
        std::atomic<FloatType> threshold{0}, ratio{1};
        FloatType currentThreshold{0}, currentRatio{1}, currentSlope{1};
        std::atomic<FloatType> kneeW{FloatType(0.01)}, curve{0};
        FloatType lowThres{0}, highThres{0}, currentCurve{0}, currentOppositeCurve{1};
        std::atomic<FloatType> bound{60};
        FloatType currentBound{60};
        std::array<FloatType, 6> paras;
        std::atomic<bool> toInterpolate{true};

        void interpolate();
    };

} // KneeComputer

#endif //COMPRESSOR_COMPUTER_H
