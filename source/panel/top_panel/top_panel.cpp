// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "top_panel.hpp"

namespace zlpanel {
    TopPanel::TopPanel(PluginProcessor& p, zlgui::UIBase& base,
                       multilingual::TooltipHelper& tooltip_helper)
        : base_(base),
          logo_panel_(p, base_, tooltip_helper),
          top_control_panel_(p, base_, tooltip_helper) {
        addAndMakeVisible(logo_panel_);
        addAndMakeVisible(top_control_panel_);

        setBufferedToImage(true);
    }

    void TopPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    void TopPanel::resized() {
        auto bound = getLocalBounds();
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        top_control_panel_.setBounds(bound.removeFromRight(top_control_panel_.getIdealWidth()));

        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 3).reduced(padding / 2, padding / 2));
    }

    void TopPanel::repaintCallBackSlow() {
        top_control_panel_.repaintCallBackSlow();
    }
}
