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
    template<typename FloatType, size_t MagNum, size_t PointNum>
    class MultipleMagAnalyzer {
    public:
        enum MagType {
            peak, rms
        };

        explicit MultipleMagAnalyzer() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            sampleRate.store(spec.sampleRate);
            setTimeLength(timeLength.load());
        }

        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> buffers) {
            switch (magType.load()) {
                case MagType::peak: {
                    processBuffer<MagType::peak>(buffers);
                    break;
                }
                case MagType::rms: {
                    processBuffer<MagType::rms>(buffers);
                    break;
                }
                default: {
                }
            }
        }

        int run(const int numToRead = PointNum) {
            juce::ScopedNoDenormals noDenormals;
            // calculate number of points put into circular buffers
            const int fifoNumReady = abstractFIFO.getNumReady();
            if (toReset.exchange(false)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    std::fill(circularMags[i].begin(), circularMags[i].end(), -240.f);
                }
                const auto scope = abstractFIFO.read(fifoNumReady);
                return 0;
            }
            const int numReady = fifoNumReady >= static_cast<int>(PointNum / 2)
                                     ? fifoNumReady
                                     : std::min(fifoNumReady, numToRead);
            if (numReady <= 0) return 0;
            const auto numReadyShift = static_cast<size_t>(numReady);
            // shift circular buffers
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circularPeak{circularMags[i]};
                // for (size_t j = 0; j < circularPeak.size() - numReadyShift; ++j) {
                //     circularPeak[j] = circularPeak[j + numReadyShift];
                // }
                std::rotate(circularPeak.begin(),
                            circularPeak.begin() + numReadyShift,
                            circularPeak.end());
            }
            // read from FIFOs
            const auto scope = abstractFIFO.read(numReady);
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circularPeak{circularMags[i]};
                auto &peakFIFO{magFIFOs[i]};
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

        void createPath(std::array<std::reference_wrapper<juce::Path>, MagNum> paths,
                        const juce::Rectangle<float> bound,
                        const float shift = 0.f,
                        const float minDB = -72.f, const float maxDB = 0.f) {
            const auto deltaX = bound.getWidth() / static_cast<float>(PointNum - 1);
            const auto x0 = bound.getX(), y0 = bound.getY(), height = bound.getHeight();
            float x = x0 - shift * deltaX;
            for (size_t i = 0; i < MagNum; ++i) {
                paths[i].get().clear();
                paths[i].get().startNewSubPath(x, magToY(circularMags[i][0], y0, height, minDB, maxDB));
                x += deltaX;
            }
            for (size_t idx = 1; idx < PointNum; ++idx) {
                for (size_t i = 0; i < MagNum; ++i) {
                    const auto y = magToY(circularMags[i][idx], y0, height, minDB, maxDB);
                    paths[i].get().lineTo(x, y);
                }
                x += deltaX;
            }
        }

        void setTimeLength(const float x) {
            timeLength.store(x);
            toUpdateTimeLength.store(true);
        }

        void setToReset() { toReset.store(true); }

        void setMagType(const MagType x) { magType.store(x); }

    protected:
        std::atomic<double> sampleRate{48000.0};
        std::array<std::array<float, PointNum>, MagNum> magFIFOs{};
        juce::AbstractFifo abstractFIFO{PointNum};
        std::array<std::array<float, PointNum>, MagNum> circularMags{};
        size_t circularIdx{0};

        std::atomic<float> timeLength{7.f};
        double currentPos{0.}, maxPos{1.};
        std::atomic<bool> toUpdateTimeLength{true};
        std::array<FloatType, MagNum> currentMags{};

        std::atomic<bool> toReset{false};
        std::atomic<MagType> magType{MagType::peak};

        template<MagType currentMagType>
        void processBuffer(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> &buffers) {
            int startIdx{0}, endIdx{0};
            int numSamples = buffers[0].get().getNumSamples();
            if (numSamples == 0) { return; }
            if (toUpdateTimeLength.exchange(false)) {
                maxPos = sampleRate.load() * static_cast<double>(timeLength.load()) / static_cast<double>(PointNum - 1);
                currentPos = 0;
            }
            while (true) {
                const auto remainNum = static_cast<int>(std::round(maxPos - currentPos));
                if (numSamples >= remainNum) {
                    startIdx = endIdx;
                    endIdx = endIdx + remainNum;
                    numSamples -= remainNum;
                    currentPos = currentPos + static_cast<double>(remainNum) - maxPos;
                    updateMags<currentMagType>(buffers, startIdx, remainNum);
                    if (abstractFIFO.getFreeSpace() == 0) return;
                    const auto scope = abstractFIFO.write(1);
                    const auto writeIdx = scope.blockSize1 > 0 ? scope.startIndex1 : scope.startIndex2;
                    for (size_t i = 0; i < MagNum; ++i) {
                        magFIFOs[i][static_cast<size_t>(writeIdx)] = static_cast<float>(currentMags[i]);
                    }
                } else {
                    currentPos += static_cast<double>(numSamples);
                    startIdx = endIdx;
                    updateMags<currentMagType>(buffers, startIdx, numSamples);
                    break;
                }
            }
        }

        template<MagType currentMagType>
        void updateMags(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> buffers,
                        const int startIdx, const int numSamples) {
            for (size_t i = 0; i < MagNum; ++i) {
                auto &buffer = buffers[i];
                switch (currentMagType) {
                    case MagType::peak: {
                        const auto currentMagnitude = buffer.get().getMagnitude(startIdx, numSamples);
                        currentMags[i] = juce::Decibels::gainToDecibels(currentMagnitude, FloatType(-240));
                    }
                    case MagType::rms: {
                        FloatType currentRMS{FloatType(0)};
                        for (int j = 0; j < buffer.get().getNumChannels(); ++j) {
                            const auto channelRMS = buffer.get().getRMSLevel(j, startIdx, numSamples);
                            currentRMS += channelRMS * channelRMS;
                        }
                        currentRMS = std::sqrt(currentRMS / static_cast<FloatType>(buffer.get().getNumChannels()));
                        currentMags[i] = juce::Decibels::gainToDecibels(currentRMS, FloatType(-240));
                    }
                }
            }
        }

        static float magToY(const float mag, const float y0, const float height,
                            const float minDB, const float maxDB) {
            return y0 + height * (maxDB - mag) / (maxDB - minDB);
        }
    };
}

#endif //ZL_PEAK_ANALYZER_HPP
