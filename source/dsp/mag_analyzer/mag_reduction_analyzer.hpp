// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_MAG_REDUCTION_ANALYZER_HPP
#define ZL_MAG_REDUCTION_ANALYZER_HPP

#include "multiple_mag_analyzer.hpp"

namespace zlMagAnalyzer {
    template<typename FloatType, size_t PointNum>
    class MagReductionAnalyzer : public MultipleMagAnalyzer<FloatType, 2, PointNum> {
    public:
        explicit MagReductionAnalyzer() : MultipleMagAnalyzer<FloatType, 2, PointNum>() {
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            this->sampleRate.store(spec.sampleRate);
            this->setTimeLength(this->timeLength.load());
            inBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        }

        void pushInBuffer(juce::AudioBuffer<FloatType> &buffer) {
            inBuffer.makeCopyOf(buffer, true);
        }

        void pushOutBuffer(juce::AudioBuffer<FloatType> &buffer) {
            this->process({inBuffer, buffer});
        }

        template<bool closeInPath = false, bool closeOutPath = false>
        void createPath(juce::Path &inPath, juce::Path &outPath, juce::Path &reductionPath,
                       const juce::Rectangle<float> bound,
                       const float shift = 0.f,
                       const float minDB = -72.f, const float maxDB = 0.f) {
            const auto deltaX = bound.getWidth() / static_cast<float>(PointNum - 1);
            const auto x0 = bound.getX(), y0 = bound.getY(), height = bound.getHeight();
            float x = x0 - shift * deltaX;
            {
                inPath.clear();
                outPath.clear();
                reductionPath.clear();
                const auto inY = this->magToY(this->circularMags[0][0], y0, height, minDB, maxDB);
                const auto outY = this->magToY(this->circularMags[1][0], y0, height, minDB, maxDB);
                if (closeInPath) {
                    inPath.startNewSubPath(x, bound.getBottom());
                    inPath.lineTo(x, inY);
                } else {
                    inPath.startNewSubPath(x, inY);
                }
                if (closeOutPath) {
                    outPath.startNewSubPath(x, bound.getBottom());
                    outPath.lineTo(x, outY);
                } else {
                    outPath.startNewSubPath(x, outY);
                }
                reductionPath.startNewSubPath(x, outY - inY);
                x += deltaX;
            }
            for (size_t idx = 1; idx < PointNum; ++idx) {
                const auto inY = this->magToY(this->circularMags[0][idx], y0, height, minDB, maxDB);
                const auto outY = this->magToY(this->circularMags[1][idx], y0, height, minDB, maxDB);
                inPath.lineTo(x, inY);
                outPath.lineTo(x, outY);
                reductionPath.lineTo(x, outY - inY);
                x += deltaX;
            }
            if (closeInPath) {
                inPath.lineTo(x - deltaX, bound.getBottom());
                inPath.closeSubPath();
            }
            if (closeOutPath) {
                outPath.lineTo(x - deltaX, bound.getBottom());
                outPath.closeSubPath();
            }
        }

    protected:
        juce::AudioBuffer<FloatType> inBuffer;
    };
}

#endif //ZL_MAG_REDUCTION_ANALYZER_HPP
