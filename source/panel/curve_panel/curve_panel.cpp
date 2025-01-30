// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlPanel {
    CurvePanel::CurvePanel(PluginProcessor &processor)
        : Thread("curve_panel"),
          peakPanel(processor),
          vblank(this, [this](const double timeStamp) { repaintCallBack(timeStamp); }) {
        addAndMakeVisible(peakPanel);
        addAndMakeVisible(computerPanel);
        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    void CurvePanel::paint(juce::Graphics &g) {
        g.fillAll(juce::Colours::lightgrey);
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        notify();
    }

    void CurvePanel::resized() {
        const auto bound = getLocalBounds();
        peakPanel.setBounds(bound);
        const auto r = std::min(bound.getWidth(), bound.getHeight());
        computerPanel.setBounds(bound.withSize(r, r));
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals noDenormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            peakPanel.run(nextStamp.load());
            computerPanel.run();
        }
    }

    void CurvePanel::repaintCallBack(const double timeStamp) {\
        nextStamp.store(timeStamp);
        repaint();
    }
} // zlPanel
