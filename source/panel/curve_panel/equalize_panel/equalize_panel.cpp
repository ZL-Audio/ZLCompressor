// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "equalize_panel.hpp"

namespace zlpanel {
    EqualizePanel::EqualizePanel(PluginProcessor &processor, zlgui::UIBase &base)
        : base_{base},
          background_panel_(processor, base_),
          fft_analyzer_panel_(processor, base) {
        addAndMakeVisible(background_panel_);
        background_panel_.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(fft_analyzer_panel_);
        fft_analyzer_panel_.setInterceptsMouseClicks(false, false);

        setInterceptsMouseClicks(true, true);
    }

    EqualizePanel::~EqualizePanel() {
    }

    void EqualizePanel::run(juce::Thread &thread) {
        juce::ignoreUnused(thread);
        fft_analyzer_panel_.run();
    }

    void EqualizePanel::resized() {
        const auto bound = getLocalBounds();
        background_panel_.setBounds(bound);
        fft_analyzer_panel_.setBounds(bound);
    }

    void EqualizePanel::repaintCallBack(double time_stamp) {
        juce::ignoreUnused(time_stamp);
        if (time_stamp - previous_time_stamp_ > 0.1) {
            background_panel_.setMouseOver(isMouseOver(true));
            previous_time_stamp_ = time_stamp;
        }
        background_panel_.repaintCallBack();
        repaint();
    }

    void EqualizePanel::mouseEnter(const juce::MouseEvent &) {
        background_panel_.setMouseOver(true);
    }
} // zlpanel
