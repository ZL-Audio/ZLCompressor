// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_COMPRESSOR_KNEE_COMPUTER_HPP
#define ZL_COMPRESSOR_KNEE_COMPUTER_HPP

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlCompressor {
    template<typename FloatType>
    struct LinearCurve {
        static constexpr FloatType a{FloatType(0)};
        FloatType b, c;

        void setPara(FloatType t, FloatType r, FloatType w) {
            juce::ignoreUnused(w);
            b = FloatType(1) / r;
            c = t * (FloatType(1) - FloatType(1) / r);
        }
    };

    template<typename FloatType>
    struct DownCurve {
        FloatType a, c;
        static constexpr FloatType b{FloatType(0)};

        void setPara(FloatType t, FloatType r, FloatType w) {
            a = FloatType(0.5) / (r * std::min(t + w, FloatType(-0.0001)));
            c = FloatType(0.5) * (w - t) / r + t;
        }
    };

    template<typename FloatType>
    struct UpCurve {
        FloatType a, c;
        static constexpr FloatType b{FloatType(1)};

        void setPara(FloatType t, FloatType r, FloatType w) {
            a = FloatType(0.5) * (FloatType(1) - r) / (r * std::min(t + w, FloatType(-0.0001)));
            c = FloatType(0.5) * (FloatType(1) - r) * (w - t) / r;
        }
    };

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

        void prepareBuffer();

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
            ratio.store(std::max(FloatType(1), v));
            toInterpolate.store(true);
        }

        inline FloatType getRatio() const { return ratio.load(); }

        inline void setKneeW(FloatType v) {
            kneeW.store(std::max(v, FloatType(0.01)));
            toInterpolate.store(true);
        }

        inline FloatType getKneeW() const { return kneeW.load(); }

        inline void setCurve(FloatType v) {
            curve.store(juce::jlimit(FloatType(-1), FloatType(1), v));
            toInterpolate.store(true);
        }

        inline FloatType getCurve() const { return curve.load(); }

        inline void setBound(FloatType v) {
            bound.store(v);
        }

        inline FloatType getBound() const { return bound.load(); }

    private:
        LinearCurve<FloatType> linearCurve;
        DownCurve<FloatType> downCurve;
        UpCurve<FloatType> upCurve;
        std::atomic<FloatType> threshold{-18}, ratio{2};
        std::atomic<FloatType> kneeW{FloatType(0.25)}, curve{0};
        FloatType lowThres{0}, highThres{0};
        std::atomic<FloatType> bound{60};
        FloatType currentBound{60};
        std::array<FloatType, 5> paras;
        std::atomic<bool> toInterpolate{true};

        void interpolate();
    };
} // KneeComputer

#endif //ZL_COMPRESSOR_KNEE_COMPUTER_HPP
