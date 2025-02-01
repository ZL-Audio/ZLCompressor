// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "computer_panel.hpp"

namespace zlPanel {
    ComputerPanel::ComputerPanel() {
        compPath.preallocateSpace(static_cast<int>(numPoint) * 3);
        nextCompPath.preallocateSpace(static_cast<int>(numPoint) * 3);
        computer.setKneeW(1.f);
        computer.setCurve(0.f);
    }

    void ComputerPanel::paint(juce::Graphics &g) {
        const juce::GenericScopedTryLock guard{lock};
        if (!guard.isLocked()) {
            return;
        }
        g.setColour(juce::Colours::orange);
        g.strokePath(compPath,
                     juce::PathStrokeType(2.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void ComputerPanel::run() {
        if (!toUpdate.exchange(false)) { return; }
        const auto currentMinDB = minDB.load();
        computer.prepareBuffer();
        const auto bound = atomicBound.load();
        auto dBIn = currentMinDB;
        const auto deltaDBIn = -currentMinDB / static_cast<float>(numPoint - 1);
        const auto deltaY = bound.getHeight() / static_cast<float>(numPoint - 1);
        for (size_t i = 0; i < numPoint; ++i) {
            const auto dBOut = computer.eval(dBIn);
            pathY[i] = dBOut / currentMinDB * bound.getHeight() + bound.getY();
            dBIn += deltaDBIn;
        }
        auto x = bound.getX();
        const auto deltaX = deltaY;
        nextCompPath.clear();
        nextCompPath.startNewSubPath(x, pathY[0]);
        size_t previousYIdx = 0;
        for (size_t i = 1; i < numPoint - 1; ++i) {
            x += deltaX;
            const auto w = 1.f / static_cast<float>(i + 1 - previousYIdx);
            const auto linPred = w * pathY[previousYIdx] + (1.f - w) * pathY[i + 1];
            if (std::abs(linPred - pathY[i]) > 0.001f) {
                nextCompPath.lineTo(x, pathY[i]);
                previousYIdx = i;
            }
        }
        x += deltaX;
        nextCompPath.lineTo(x, pathY[numPoint - 1]);
        const juce::GenericScopedLock guard{lock};
        compPath = nextCompPath;
    }

    void ComputerPanel::resized() {
        atomicBound.store(getLocalBounds().toFloat());
        toUpdate.store(true);
    }
} // zlPanel
