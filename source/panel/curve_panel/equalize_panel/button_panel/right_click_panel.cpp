// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "right_click_panel.hpp"

namespace zlpanel {
    RightClickPanel::RightClickPanel(PluginProcessor &processor, zlgui::UIBase &base,
                                     size_t &selected_band_idx)
        : p_ref_(processor), base_(base), selected_band_idx_(selected_band_idx),
          invert_gain_button_(base, "Invert Gain", ""),
          copy_button_(base, "Copy", ""),
          paste_button_(base, "Paste", "") {
        for (auto &button : {&invert_gain_button_, &copy_button_, &paste_button_}) {
            button->getLAF().setFontScale(1.25f);
            addAndMakeVisible(button);
        }
        setInterceptsMouseClicks(false, true);
    }

    void RightClickPanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    void RightClickPanel::updateCopyVisibility(const bool show_copy) {
        if (!show_copy) {
            copy_button_.setAlpha(.25f);
            copy_button_.setInterceptsMouseClicks(false, false);
        } else {
            copy_button_.setAlpha(1.f);
            copy_button_.setInterceptsMouseClicks(false, true);
        }
    }

    void RightClickPanel::resized() {
        auto bound = getLocalBounds();
        bound.removeFromLeft(juce::roundToInt(base_.getFontSize()));
        bound.removeFromRight(juce::roundToInt(base_.getFontSize()));
        const auto button_height = bound.getHeight() / 3;
        invert_gain_button_.setBounds(bound.removeFromTop(button_height));
        copy_button_.setBounds(bound.removeFromTop(button_height));
        paste_button_.setBounds(bound.removeFromTop(button_height));
    }

    void RightClickPanel::invertGain() {

    }
} // zlpanel
