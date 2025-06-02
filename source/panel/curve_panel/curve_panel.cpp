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
          peak_panel_(p), rms_panel_(p),
          computer_panel_(p, base_),
          bottom_control_panel_(p, base_) {
        addAndMakeVisible(peak_panel_);
        addAndMakeVisible(separate_panel_);
        addAndMakeVisible(rms_panel_);
        addAndMakeVisible(computer_panel_);
        addAndMakeVisible(bottom_control_panel_);
        computer_panel_.setInterceptsMouseClicks(false, false);
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
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        {
            auto bound = getLocalBounds();
            bound.removeFromLeft(slider_width / 3);
            rms_panel_.setBounds(bound.withWidth(75));
            peak_panel_.setBounds(bound);
            const auto r = std::min(bound.getWidth(), bound.getHeight());
            separate_panel_.setBounds(bound.withSize(r, r));
            computer_panel_.setBounds(bound.withSize(r, r));
        } {
            auto bound = getLocalBounds();
            bound = bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            bottom_control_panel_.setBounds(bound);
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            const auto time_stamp = next_stamp_.load();
            peak_panel_.run(time_stamp);
            computer_panel_.run();
            if (to_run_rms_.exchange(false)) {
                rms_panel_.run(time_stamp);
            }
        }
    }

    void CurvePanel::repaintCallBack(const double time_stamp) {
        bottom_control_panel_.repaintCallBack(time_stamp);
        if (repaint_count_ >= 0) {
            repaint_count_ = 0;
            next_stamp_.store(time_stamp);
            peak_panel_.repaint();
            if (time_stamp - rms_previous_stamp_ > .1) {
                rms_panel_.repaint();
                rms_previous_stamp_ = time_stamp;
                to_run_rms_.store(true);
            }
        } else {
            repaint_count_ += 1;
        }
    }
} // zlpanel
