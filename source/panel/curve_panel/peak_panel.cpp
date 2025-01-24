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
        constexpr auto preallocateSpace = static_cast<int>(zlDSP::Controller::analyzerPointNum) * 3 + 1;
        for (auto &path: {&inPath, &outPath, &reductionPath}) {
            path->preallocateSpace(preallocateSpace);
        }
        magAnalyzer.setToReset();
        setTimeLength(12.f);
    }

    void PeakPanel::paint(juce::Graphics &g) {
        g.setColour(juce::Colours::black.withAlpha(.25f));
        g.fillPath(inPath);
        g.setColour(juce::Colours::black.withAlpha(.9f));
        g.strokePath(outPath,
                     juce::PathStrokeType(1.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
        g.setColour(juce::Colours::darkred);
        g.strokePath(reductionPath,
                     juce::PathStrokeType(1.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void PeakPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withWidth(bound.getWidth() * 1.01f);
        atomicBound.store(bound);
    }

    void PeakPanel::run(const double nextTimeStamp) {
        if (startTime <= 1e-6) {
            startTime = nextTimeStamp;
            magAnalyzer.run(static_cast<int>(zlDSP::Controller::analyzerPointNum));
            magAnalyzer.createPath(inPath, outPath, reductionPath, atomicBound.load(), 0.f);
        } else {
            const auto targetCount = (nextTimeStamp - startTime) * numPerSecond.load();
            const auto targetDelta = targetCount - currentCount;
            const auto actualDelta = static_cast<double>(magAnalyzer.run(static_cast<int>(std::floor(targetDelta))));
            if (std::abs(targetDelta - actualDelta) < 2.9) {
                currentCount += static_cast<double>(actualDelta);
            } else {
                currentCount = std::floor(targetCount);
            }
            const auto shift = targetCount - currentCount;
            magAnalyzer.template createPath<true, false>(inPath, outPath, reductionPath, atomicBound.load(), static_cast<float>(shift));
        }
    }
} // zlPanel
