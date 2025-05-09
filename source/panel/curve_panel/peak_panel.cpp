// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "peak_panel.hpp"

namespace zlPanel {
    PeakPanel::PeakPanel(PluginProcessor &processor)
        : magAnalyzer(processor.getController().getMagAnalyzer()) {
        constexpr auto preallocateSpace = static_cast<int>(zldsp::Controller::kAnalyzerPointNum) * 3 + 1;
        for (auto &path: {&inPath, &outPath, &reductionPath}) {
            path->preallocateSpace(preallocateSpace);
        }
        magAnalyzer.setToReset();
        setTimeLength(6.f);
    }

    void PeakPanel::paint(juce::Graphics &g) {
        const juce::GenericScopedTryLock guard{lock};
        if (!guard.isLocked()) {
            return;
        }
        g.setColour(juce::Colours::white.withAlpha(.25f));
        g.fillPath(inPath);
        g.setColour(juce::Colours::white.withAlpha(.9f));
        g.strokePath(outPath,
                     juce::PathStrokeType(1.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
        g.setColour(juce::Colours::mediumvioletred);
        g.strokePath(reductionPath,
                     juce::PathStrokeType(2.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void PeakPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        constexpr auto padP = 1.f / static_cast<float>(zldsp::Controller::kAnalyzerPointNum - 1);
        const auto pad = std::max(bound.getWidth() * padP, 1.f);
        bound = bound.withWidth(bound.getWidth() + pad);
        atomicBound.store(bound);
    }

    void PeakPanel::run(const double nextTimeStamp) {
        if (isFirstPoint) {
            if (magAnalyzer.run(1) > 0) {
                isFirstPoint = false;
                currentCount = 0.;
                startTime = nextTimeStamp; {
                    nextInPath.clear();
                    nextOutPath.clear();
                    nextReductionPath.clear();
                }
                magAnalyzer.template createPath<true, false>(nextInPath, nextOutPath, nextReductionPath,
                                       atomicBound.load(), 0.f, 0.f);
            }
        } else {
            const auto targetCount = (nextTimeStamp - startTime) * numPerSecond.load();
            const auto targetDelta = targetCount - currentCount;
            const auto actualDelta = static_cast<double>(magAnalyzer.run(static_cast<int>(std::floor(targetDelta))));
            currentCount += static_cast<double>(actualDelta);
            const auto currentError = std::abs(targetDelta - actualDelta);
            if (currentError < smoothError) {
                smoothError = currentError;
            } else {
                smoothError = smoothError * 0.95 + currentError * 0.05;
            }
            if (smoothError > 1.0) {
                currentCount += std::floor(smoothError);
                if (consErrorCount < 5) {
                    consErrorCount += 1;
                }
            }
            if (actualDelta > 0) {
                consErrorCount = 0;
            }
            if (consErrorCount < 5) {
                const auto shift = targetCount - currentCount; {
                    nextInPath.clear();
                    nextOutPath.clear();
                    nextReductionPath.clear();
                }
                magAnalyzer.template createPath<true, false>(nextInPath, nextOutPath, nextReductionPath,
                                                             atomicBound.load(), static_cast<float>(shift), 0.f);
            }
        } {
            const juce::GenericScopedLock guard{lock};
            inPath = nextInPath;
            outPath = nextOutPath;
            reductionPath = nextReductionPath;
        }
    }
} // zlPanel
