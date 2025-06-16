// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "mag_analyzer_panel.hpp"

namespace zlpanel {
    MagAnalyzerPanel::MagAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base),
          peak_panel_(p, base), rms_panel_(p, base),
          computer_panel_(p, base), separate_panel_(base) {
        addAndMakeVisible(peak_panel_);
        addAndMakeVisible(separate_panel_);
        addAndMakeVisible(rms_panel_);
        addAndMakeVisible(computer_panel_);
        computer_panel_.setInterceptsMouseClicks(false, false);
    }

    MagAnalyzerPanel::~MagAnalyzerPanel() {
    }

    void MagAnalyzerPanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    void MagAnalyzerPanel::resized() {
        const auto bound = getLocalBounds();
        rms_panel_.setBounds(bound.withWidth(juce::roundToInt(base_.getFontSize() * kSliderScale * .75f)));
        peak_panel_.setBounds(bound);
        const auto r = std::min(bound.getWidth(), bound.getHeight());
        separate_panel_.setBounds(bound.withSize(r, r));
        computer_panel_.setBounds(bound.withSize(r, r));
    }

    void MagAnalyzerPanel::run(const juce::Thread &thread) {
        juce::ScopedNoDenormals no_denormals;
        const auto time_stamp = next_stamp_.load(std::memory_order::relaxed);
        peak_panel_.run(time_stamp);
        if (thread.threadShouldExit()) {
            return;
        }
        computer_panel_.run();
        if (thread.threadShouldExit()) {
            return;
        }
        if (to_run_rms_.exchange(false, std::memory_order::relaxed)) {
            rms_panel_.run(true);
        } else {
            rms_panel_.run(false);
        }
        if (thread.threadShouldExit()) {
            return;
        }
    }

    void MagAnalyzerPanel::repaintCallBack(const double time_stamp) {
        next_stamp_.store(time_stamp, std::memory_order::relaxed);
        peak_panel_.repaint();
        if (time_stamp - rms_previous_stamp_ > .1 && rms_panel_.isVisible()) {
            rms_panel_.repaint();
            rms_previous_stamp_ = time_stamp;
            to_run_rms_.store(true, std::memory_order::relaxed);
        }
    }
} // zlpanel
