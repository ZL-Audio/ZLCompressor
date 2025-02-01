// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_MULTIPLE_MAG_AVG_ANALYZER_HPP
#define ZL_MULTIPLE_MAG_AVG_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlMagAnalyzer {
    template<typename FloatType, size_t MagNum, size_t BinNum>
    class MultipleMagAvgAnalyzer {
    public:
        static constexpr double rmsLength = 0.01;

        enum MagType {
            peak, rms
        };

        explicit MultipleMagAvgAnalyzer() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            sampleRate.store(spec.sampleRate);
            maxPos = sampleRate.load() * rmsLength;
            currentPos = 0.;
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

        void setToReset() { toReset.store(true); }

        void setMagType(const MagType x) { magType.store(x); }

        void run() {
            juce::ScopedNoDenormals noDenormals;
            if (toReset.exchange(false)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    auto &cumulativeCount{cumulativeCounts[i]};
                    std::fill(cumulativeCount.begin(), cumulativeCount.end(), 0.);
                }
            }
            const int numReady = abstractFIFO.getNumReady();
            const auto scope = abstractFIFO.read(numReady);
            for (size_t i = 0; i < MagNum; ++i) {
                auto &magFIFO{magFIFOs[i]};
                auto &cumulativeCount{cumulativeCounts[i]};
                for (auto idx = scope.startIndex1; idx < scope.startIndex1 + scope.blockSize1; ++idx) {
                    updateHist(cumulativeCount, magFIFO[static_cast<size_t>(idx)]);
                }
                for (auto idx = scope.startIndex2; idx < scope.startIndex2 + scope.blockSize2; ++idx) {
                    updateHist(cumulativeCount, magFIFO[static_cast<size_t>(idx)]);
                }
            }
            std::array<double, MagNum> maximumCounts{};
            for (size_t i = 0; i < MagNum; ++i) {
                maximumCounts[i] = *std::max_element(cumulativeCounts[i].begin(), cumulativeCounts[i].end());
            }
            const auto maximumCount = std::max(10. / rmsLength,
                                               *std::max_element(maximumCounts.begin(), maximumCounts.end()));
            for (size_t i = 0; i < MagNum; ++i) {
                const auto &cumulativeCount{cumulativeCounts[i]};
                auto &avgCount{avgCounts[i]};
                juce::FloatVectorOperations::multiply(&avgCount[0], &cumulativeCount[0],
                                                      1. / maximumCount, avgCount.size());
            }
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, MagNum> paths,
                        const std::array<bool, MagNum> isClosePath,
                        const juce::Rectangle<float> bound, size_t endIdx) {
            endIdx = std::min(endIdx, BinNum);
            const auto deltaY = bound.getHeight() / static_cast<float>(endIdx - 1);
            for (size_t i = 0; i < MagNum; ++i) {
                const auto y = bound.getY();
                const auto &avgCount{avgCounts[i]};
                constexpr size_t idx = 0;
                const auto x = bound.getX() + static_cast<float>(avgCount[idx]) * bound.getWidth();
                if (isClosePath[i]) {
                    paths[i].get().startNewSubPath(bound.getTopLeft());
                    paths[i].get().lineTo(x, y);
                } else {
                    paths[i].get().startNewSubPath(x, y);
                }
            }

            for (size_t i = 0; i < MagNum; ++i) {
                auto y = deltaY;
                const auto &avgCount{avgCounts[i]};
                for (size_t idx = 1; idx < endIdx; ++idx) {
                    const auto x = bound.getX() + static_cast<float>(avgCount[idx]) * bound.getWidth();
                    paths[i].get().lineTo(x, y);
                    y += deltaY;
                }
            }
            for (size_t i = 0; i < MagNum; ++i) {
                if (isClosePath[i]) {
                    paths[i].get().lineTo(bound.getBottomLeft());
                    paths[i].get().closeSubPath();
                }
            }
        }

    protected:
        std::atomic<double> sampleRate{48000.0};
        std::array<std::array<float, 1000>, MagNum> magFIFOs{};
        juce::AbstractFifo abstractFIFO{1000};

        std::atomic<bool> toReset{false};
        std::atomic<MagType> magType{MagType::rms};

        double currentPos{0.}, maxPos{1.};
        std::array<FloatType, MagNum> currentMags{};

        std::array<std::array<double, BinNum>, MagNum> cumulativeCounts{};
        std::array<std::array<double, BinNum>, MagNum> avgCounts{};

        template<MagType currentMagType>
        void processBuffer(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> &buffers) {
            int startIdx{0}, endIdx{0};
            int numSamples = buffers[0].get().getNumSamples();
            if (numSamples == 0) { return; }
            while (true) {
                const auto remainNum = static_cast<int>(std::round(maxPos - currentPos));
                if (numSamples >= remainNum) {
                    startIdx = endIdx;
                    endIdx = endIdx + remainNum;
                    numSamples -= remainNum;
                    updateMags<currentMagType, false>(buffers, startIdx, remainNum);
                    currentPos = currentPos + static_cast<double>(remainNum) - maxPos;
                    if (abstractFIFO.getFreeSpace() > 0) {
                        const auto scope = abstractFIFO.write(1);
                        const auto writeIdx = scope.blockSize1 > 0 ? scope.startIndex1 : scope.startIndex2;
                        for (size_t i = 0; i < MagNum; ++i) {
                            magFIFOs[i][static_cast<size_t>(writeIdx)] = juce::Decibels::gainToDecibels(
                                static_cast<float>(currentMags[i]), -240.f);
                        }
                    }
                    std::fill(currentMags.begin(), currentMags.end(), FloatType(0));
                } else {
                    updateMags<currentMagType, false>(buffers, startIdx, numSamples);
                    currentPos += static_cast<double>(numSamples);
                    break;
                }
            }
        }

        template<MagType currentMagType, bool replaceMag = true>
        void updateMags(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> buffers,
                        const int startIdx, const int numSamples) {
            for (size_t i = 0; i < MagNum; ++i) {
                auto &buffer = buffers[i];
                switch (currentMagType) {
                    case MagType::peak: {
                        const auto currentMagnitude = buffer.get().getMagnitude(startIdx, numSamples);
                        currentMags[i] = replaceMag ? currentMagnitude : std::max(currentMags[i], currentMagnitude);
                    }
                    case MagType::rms: {
                        FloatType currentMS{FloatType(0)};
                        for (int j = 0; j < buffer.get().getNumChannels(); ++j) {
                            const auto channelRMS = buffer.get().getRMSLevel(j, startIdx, numSamples);
                            currentMS += channelRMS * channelRMS;
                        }
                        currentMS = currentMS / static_cast<FloatType>(buffer.get().getNumChannels());
                        currentMags[i] = replaceMag
                                             ? std::sqrt(currentMS)
                                             : std::sqrt(currentMags[i] * currentMags[i] + currentMS);
                    }
                }
            }
        }

        static inline void updateHist(std::array<double, BinNum> &hist, const double x) {
            const auto idx = static_cast<size_t>(std::max(0., std::round(-x)));
            if (idx < BinNum) {
                juce::FloatVectorOperations::multiply(&hist[0], 0.99999, hist.size());
                hist[idx] += 1.;
            }
        }
    };
}

#endif //ZL_MULTIPLE_MAG_AVG_ANALYZER_HPP
