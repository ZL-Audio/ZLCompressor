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
    public:
        explicit MultiplePeakAnalyzer() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            sampleRate.store(spec.sampleRate);
            setTimeLength(timeLength.load());
        }

        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, PeakNum> buffers) {
            int startIdx{0}, endIdx{0};
            int numSamples = buffers[0].get().getNumSamples();
            while (true) {
                if (const auto remainNum = maxPos.load() - currentPos; numSamples >= remainNum) {
                    startIdx = endIdx;
                    endIdx = endIdx + remainNum;
                    numSamples -= remainNum;
                    currentPos = 0;
                    updatePeaks(buffers, startIdx, remainNum);
                    if (abstractFIFO.getFreeSpace() == 0) return;
                    const auto scope = abstractFIFO.write(1);
                    const auto writeIdx = scope.blockSize1 > 0 ? scope.startIndex1 : scope.startIndex2;
                    for (size_t i = 0; i < PeakNum; ++i) {
                        peakFIFOs[i][static_cast<size_t>(writeIdx)] = static_cast<float>(currentPeaks[i]);
                    }
                } else {
                    currentPos += numSamples;
                    startIdx = endIdx;
                    updatePeaks(buffers, startIdx, numSamples);
                    break;
                }
            }
        }

        int run(const int numToRead = PointNum) {
            juce::ScopedNoDenormals noDenormals;
            // calculate number of points put into circular buffers
            const int fifoNumReady = abstractFIFO.getNumReady();
            if (toReset.exchange(false)) {
                for (size_t i = 0; i < PeakNum; ++i) {
                    std::fill(circularPeaks[i].begin(), circularPeaks[i].end(), 0.f);
                }
                const auto scope = abstractFIFO.read(fifoNumReady);
                return 0;
            }
            const int numReady = fifoNumReady >= static_cast<int>(PointNum / 2)
                                     ? fifoNumReady
                                     : std::min(fifoNumReady, numToRead);
            if (numReady == 0) return 0;
            const auto numReadyShift = static_cast<typename std::array<float, PointNum>::difference_type>(numReady);
            // shift circular buffers
            for (size_t i = 0; i < PeakNum; ++i) {
                auto &circularPeak{circularPeaks[i]};
                std::rotate(circularPeak.begin(),
                            circularPeak.begin() + numReadyShift,
                            circularPeak.end());
            }
            // read from FIFOs
            const auto scope = abstractFIFO.read(numReady);
            for (size_t i = 0; i < PeakNum; ++i) {
                auto &circularPeak{circularPeaks[i]};
                auto &peakFIFO{peakFIFOs[i]};
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
            return numReady;
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, PeakNum> paths,
                        const juce::Rectangle<float> bound,
                        const float shift = 0.f,
                        const float minDB = -72.f, const float maxDB = 0.f) {
            const auto deltaX = bound.getWidth() / static_cast<float>(PointNum - 1);
            const auto x0 = bound.getX(), y0 = bound.getY(), height = bound.getHeight();
            float x = x0 - shift * deltaX;
            for (size_t i = 0; i < PeakNum; ++i) {
                paths[i].get().clear();
                paths[i].get().startNewSubPath(x0, peakToY(circularPeaks[i][0], y0, height, minDB, maxDB));
                x += deltaX;
            }
            for (size_t idx = 1; idx < PointNum; ++idx) {
                for (size_t i = 0; i < PeakNum; ++i) {
                    const auto y = peakToY(circularPeaks[i][idx], y0, height, minDB, maxDB);
                    paths[i].get().lineTo(x, y);
                }
                x += deltaX;
            }
        }

        void setTimeLength(const float x) {
            timeLength.store(x);
            maxPos.store(static_cast<int>(
                sampleRate.load() * static_cast<double>(x) / static_cast<double>(PointNum - 1)));
        }

        void setToReset() { toReset.store(true); }

    private:
        std::atomic<double> sampleRate{48000.0};
        std::array<std::array<float, PointNum>, PeakNum> peakFIFOs{};
        juce::AbstractFifo abstractFIFO{PointNum};
        std::array<std::array<float, PointNum>, PeakNum> circularPeaks{};
        size_t circularIdx{0};

        std::atomic<float> timeLength{7.f};
        int currentPos{0};
        std::atomic<int> maxPos{1};
        std::array<FloatType, PeakNum> currentPeaks{};

        std::atomic<bool> toReset{false};

        void updatePeaks(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, PeakNum> buffers,
                         const int startIdx, const int numSamples) {
            for (size_t i = 0; i < PeakNum; ++i) {
                auto &buffer = buffers[i];
                currentPeaks[i] = buffer.get().getMagnitude(startIdx, numSamples);
            }
        }

        static float peakToY(const float peak, const float y0, const float height,
                             const float minDB, const float maxDB) {
            const auto dB = juce::Decibels::gainToDecibels(peak, minDB * 2.f);
            return y0 + height * (maxDB - dB) / (maxDB - minDB);
        }
    };
}

#endif //ZL_PEAK_ANALYZER_HPP
