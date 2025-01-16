// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_PEAK_ANALYZER_HPP
#define ZL_PEAK_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlMagAnalyzer {
    template<typename FloatType, size_t PeakNum, size_t PointNum>
    class MultiplePeakAnalyzer {
        static constexpr double peakPeriod = 0.008333333333333333;
        static constexpr float maxDB = 0.f, minDB = -72.f;

    public:
        explicit MultiplePeakAnalyzer() = default;

        void prepare(const double sampleRate) {
            maxPos = static_cast<int>(sampleRate * peakPeriod);
            for (auto &fifo: peakFIFOs) {
                fifo.resize(maxPos);
            }
        }

        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, PeakNum> buffers) {
            int startIdx{0}, endIdx{0};
            int numSamples = buffers[0].get().getNumSamples();
            while (true) {
                if (const auto remainNum = maxPos - currentPos; numSamples >= remainNum) {
                    startIdx = endIdx;
                    endIdx = endIdx + remainNum;
                    numSamples -= remainNum;
                    currentPos = 0;
                    updatePeaks(buffers, startIdx, remainNum);
                    if (abstractFIFO.getFreeSpace() == 0) return;
                    const auto scope = abstractFIFO.write(1);
                    auto writeIdx = scope.blockSize1 > 0 ? scope.startIndex1 : scope.startIndex2;
                    for (size_t i = 0; i < PeakNum; ++i) {
                        peakFIFOs[i][writeIdx] = currentPeaks[i];
                    }
                } else {
                    currentPos += numSamples;
                    startIdx = endIdx;
                    endIdx = endIdx + numSamples;
                    updatePeaks(buffers, startIdx, numSamples);
                    break;
                }
            }
        }

        void run() {
            juce::ScopedNoDenormals noDenormals;
            // calculate number of points put into circular buffers
            const int numReady = abstractFIFO.getNumReady();
            // shift circular buffers
            for (size_t i = 0; i < PeakNum; ++i) {
                const auto &circularPeak{circularPeaks[i]};
                std::rotate(circularPeak.begin(), &circularPeak[static_cast<size_t>(numReady)], circularPeak.end());
            }
            // read from FIFOs
            const auto scope = abstractFIFO.read(numReady);
            for (size_t i = 0; i < PeakNum; ++i) {
                const auto &circularPeak{circularPeaks[i]};
                const auto &peakFIFO{peakFIFOs[i]};
                size_t j = circularPeak.size() - static_cast<size_t>(numReady);
                if (scope.blockSize1 > 0) {
                    std::copy(&peakFIFO[static_cast<size_t>(scope.startIndex1)],
                              &peakFIFO[static_cast<size_t>(scope.startIndex1 + scope.blockSize1)],
                              &circularPeak[j]);
                    j += static_cast<size_t>(scope.blockSize1);
                }
                if (scope.blockSize2 > 0) {
                    std::copy(&peakFIFO[static_cast<size_t>(scope.startIndex2)],
                              &peakFIFO[static_cast<size_t>(scope.startIndex2 + scope.blockSize2)],
                              &circularPeak[j]);
                }
            }
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, PeakNum> paths,
                        const juce::Rectangle<float> bound) {
            const auto deltaX = bound.getWidth() / static_cast<float>(PointNum - 1);
            const auto x0 = bound.getX(), y0 = bound.getY(), height = bound.getHeight();
            for (size_t i = 0; i < PeakNum; ++i) {
                paths[i].get().clear();
                paths[i].get().startNewSubPath(x0, peakToY(circularPeaks[i][0], y0, height));
            }
            float x = x0 + deltaX;
            for (size_t idx = 1; idx < PointNum; ++idx) {
                for (size_t i = 0; i < PeakNum; ++i) {
                    paths[i].get().lineTo(x, peakToY(circularPeaks[i][idx], y0, height));
                }
                x += deltaX;
            }
        }

    private:
        std::array<std::array<float, PointNum>, PeakNum> peakFIFOs;
        juce::AbstractFifo abstractFIFO{PointNum};
        std::array<std::array<float, PointNum>, PeakNum> circularPeaks;
        size_t circularIdx{0};

        int currentPos{0}, maxPos{1};
        std::array<FloatType, PeakNum> currentPeaks{};

        void updatePeaks(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, PeakNum> buffers,
                         const int startIdx, const int numSamples) {
            for (size_t i = 0; i < PeakNum; ++i) {
                auto &buffer = buffers[i];
                currentPeaks[i] = buffer.get().getMagnitude(startIdx, numSamples);
            }
        }

        static float peakToY(const float peak, const float y, const float height) {
            const auto dB = juce::Decibels::gainToDecibels(peak, -minDB * 2.f);
            return y + height * (dB - minDB) / (maxDB - minDB);
        }
    };
}

#endif //ZL_PEAK_ANALYZER_HPP
