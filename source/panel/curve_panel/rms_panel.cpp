// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "rms_panel.hpp"

namespace zlPanel {
    RMSPanel::RMSPanel(PluginProcessor &processor)
        : avgAnalyzer(processor.getController().getMagAvgAnalyzer()) {
        avgAnalyzer.setToReset();
    }

    void RMSPanel::paint(juce::Graphics &g) {
        const juce::GenericScopedTryLock guard{lock};
        if (!guard.isLocked()) {
            return;
        }
        // g.setGradientFill(gradient);
        // g.fillRect(getLocalBounds());
        g.setColour(juce::Colours::white.withAlpha(.25f));
        g.fillPath(inPath);
        g.setColour(juce::Colours::white.withAlpha(.9f));
        g.strokePath(outPath,
                     juce::PathStrokeType(1.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void RMSPanel::run(double nextTimeStamp) {
        juce::ignoreUnused(nextTimeStamp);
        const auto currentBound = atomicBound.load();
        avgAnalyzer.run();
        nextInPath.clear();
        nextOutPath.clear();
        avgAnalyzer.createPath({nextInPath, nextOutPath}, {true, false},
                               currentBound, 72); {
            const juce::GenericScopedLock guard{lock};
            inPath = nextInPath;
            outPath = nextOutPath;
        }
    }

    void RMSPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        atomicBound.store(bound.withWidth(bound.getWidth() - 20.f));

        gradient.point1 = juce::Point<float>(bound.getX(), 0.f);
        gradient.point2 = juce::Point<float>(bound.getRight(), 0.f);
        gradient.isRadial = false;
        gradient.clearColours();

        gradient.addColour(0.0,
                           juce::Colours::black.withAlpha(1.f));
        gradient.addColour(1.0,
                           juce::Colours::black.withAlpha(0.f));
    }

    void RMSPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        avgAnalyzer.setToReset();
    }
} // zlPanel
