// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor &p, zlgui::UIBase &base)
        : Thread("curve_panel"), base_(base),
          mag_analyzer_panel_(p, base_),
          bottom_control_panel_(p, base_),
          left_control_panel_(p, base_),
    side_control_panel_(p, base_) {
        addAndMakeVisible(mag_analyzer_panel_);
        addAndMakeVisible(bottom_control_panel_);
        addAndMakeVisible(left_control_panel_);

        addChildComponent(side_control_panel_);
        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    void CurvePanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        notify();
    }

    void CurvePanel::resized() {
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale); {
            auto bound = getLocalBounds();
            bound.removeFromLeft(button_height);
            mag_analyzer_panel_.setBounds(bound);
        } {
            auto bound = getLocalBounds();
            bound = bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            bottom_control_panel_.setBounds(bound);
        } {
            auto bound = getLocalBounds();
            bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            left_control_panel_.setBounds(bound.removeFromLeft(button_height));
        } {
            auto bound = getLocalBounds();
            bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            bound.removeFromLeft(button_height);
            const auto p = bound.getBottomLeft();
            const auto width = side_control_panel_.getIdealWidth();
            const auto height = side_control_panel_.getIdealHeight();
            side_control_panel_.setBounds(p.getX(), p.getY() - height, width, height);
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            mag_analyzer_panel_.run();
        }
    }

    void CurvePanel::repaintCallBack(const double time_stamp) {
        mag_analyzer_panel_.repaintCallBack(time_stamp);
        bottom_control_panel_.repaintCallBack(time_stamp);
        left_control_panel_.repaintCallBack(time_stamp);
        side_control_panel_.repaintCallBack(time_stamp);
    }
} // zlpanel
