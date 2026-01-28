// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

namespace zlpanel {
    ControlPanel::ControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                               const multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        control_background_(base_, .5f, {true, false, false, true}),
        mid_control_panel_(p, base_, tooltip_helper),
        right_control_panel_(p, base_, tooltip_helper),
        bottom_control_panel_(p, base_, tooltip_helper) {
        addAndMakeVisible(control_background_);
        addAndMakeVisible(mid_control_panel_);
        addAndMakeVisible(right_control_panel_);

        addAndMakeVisible(bottom_control_panel_);
        bottom_control_panel_.setBufferedToImage(true);

        control_background_.setBufferedToImage(true);

        setBufferedToImage(true);

        setInterceptsMouseClicks(false, true);
    }

    void ControlPanel::paint(juce::Graphics&) {
    }

    void ControlPanel::resized() {
        auto bound = getLocalBounds();
        control_background_.setBounds(bound);

        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        bound.removeFromTop(padding);
        bound.removeFromRight(padding);

        bottom_control_panel_.setBounds(bound.removeFromTop(bottom_control_panel_.getIdealHeight()));

        const auto right_width = right_control_panel_.getIdealWidth();
        const auto mid_width = mid_control_panel_.getIdealWidth();
        right_control_panel_.setBounds(bound.removeFromRight(right_width));
        bound.removeFromLeft((bound.getWidth() - mid_width) / 2);
        mid_control_panel_.setBounds(bound.removeFromLeft(mid_width));
    }

    void ControlPanel::repaintCallBackSlow() {
        mid_control_panel_.repaintCallBackSlow();
        right_control_panel_.repaintCallBackSlow();
        bottom_control_panel_.repaintCallBackSlow();
    }

    int ControlPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        return padding + right_control_panel_.getIdealWidth() + mid_control_panel_.getIdealWidth();
    }

    int ControlPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        return getControlPanelHeight(font_size);
    }
}
