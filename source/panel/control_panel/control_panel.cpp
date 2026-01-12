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
                               multilingual::TooltipHelper& tooltip_helper)
        : base_(base),
          mid_control_panel_(p, base_, tooltip_helper),
          right_control_panel_(p, base_, tooltip_helper) {
        addAndMakeVisible(mid_control_panel_);
        addAndMakeVisible(right_control_panel_);

        setOpaque(true);
    }

    void ControlPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getBackgroundColour());
        g.fillRect(getLocalBounds());
    }

    void ControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto right_width = right_control_panel_.getIdealWidth();
        const auto mid_width = mid_control_panel_.getIdealWidth();
        right_control_panel_.setBounds(bound.removeFromRight(right_width));
        bound.removeFromLeft((bound.getWidth() - mid_width) / 2);
        mid_control_panel_.setBounds(bound.removeFromLeft(mid_width));
    }

    void ControlPanel::repaintCallBackSlow() {
        mid_control_panel_.repaintCallBackSlow();
        right_control_panel_.repaintCallBackSlow();
    }
}
