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
        : peakAnalyzer(processor.getController().getPeakAnalyzer()) {
        constexpr auto preallocateSpace = static_cast<int>(zlDSP::Controller::analyzerPointNum) * 3 + 1;
        recentInPath.preallocateSpace(preallocateSpace);
        inPath.preallocateSpace(preallocateSpace);
        peakAnalyzer.setToReset();
    }

    void PeakPanel::paint(juce::Graphics &g) {
        const juce::GenericScopedTryLock lockGuard(lock);
        if (lockGuard.isLocked()) {
            g.setColour(juce::Colours::black);
            g.strokePath(recentInPath,
                         juce::PathStrokeType(1.0f,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
    }

    void PeakPanel::resized() {
        atomicBound.store(getLocalBounds().toFloat());
    }

    void PeakPanel::run() {
        if (startTime.toMilliseconds() == 0) {
            startTime = juce::Time::getCurrentTime();
            peakAnalyzer.run(10000);
            peakAnalyzer.createPath({inPath}, atomicBound.load(), 0.f);
            const juce::GenericScopedLock lockGuard(lock);
            recentInPath = inPath;
        } else {
            const auto currentNumPerMilliSecond = numPerMilliSecond.load();
            const auto currentTime = juce::Time::getCurrentTime();
            const auto deltaTime = static_cast<double>(currentTime.toMilliseconds() - startTime.toMilliseconds());
            const auto targetCount = deltaTime * currentNumPerMilliSecond;
            const auto targetDelta = targetCount - currentCount;
            const auto actualDelta = peakAnalyzer.run(static_cast<int>(std::floor(targetDelta)));
            if (std::abs(targetDelta - actualDelta) < 2.9) {
                currentCount += static_cast<double>(actualDelta);
            } else {
                currentCount = std::floor(targetCount);
            }
            const auto shift = targetCount - currentCount;
            peakAnalyzer.createPath({inPath}, atomicBound.load(), static_cast<float>(shift));
            const juce::GenericScopedLock lockGuard(lock);
            recentInPath = inPath;
        }
    }
} // zlPanel
