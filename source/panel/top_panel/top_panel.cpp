// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "top_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    TopPanel::TopPanel(PluginProcessor& p, zlgui::UIBase& base,
                       multilingual::TooltipHelper& tooltip_helper)
        : base_(base),
          logo_panel_(p, base_, tooltip_helper),
          preset_drawable_(juce::Drawable::createFromImageData(BinaryData::collections_bookmark_svg,
                                                               BinaryData::collections_bookmark_svgSize)),
          preset_button_(base_, preset_drawable_.get(), nullptr, ""),
          top_control_panel_(p, base_, tooltip_helper) {
        addAndMakeVisible(logo_panel_);
        preset_button_.getButton().onClick = [this]() {
            const auto panel_open = static_cast<float>(
                base_.getPanelProperty(zlgui::PanelSettingIdx::kPresetBrowser));
            base_.setPanelProperty(zlgui::PanelSettingIdx::kPresetBrowser,
                                   panel_open < .5f ? 1.f : 0.f);
        };
        preset_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        preset_button_.setBufferedToImage(true);
        addAndMakeVisible(preset_button_);
        addAndMakeVisible(top_control_panel_);

        setBufferedToImage(true);
    }

    void TopPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    int TopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        return 2 * (getPaddingSize(font_size) / 2) + getButtonSize(font_size);
    }

    void TopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        auto bound = getLocalBounds();
        bound.reduce(padding / 2, padding / 2);

        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 2 + padding));

        bound.removeFromLeft(padding);
        preset_button_.setBounds(bound.removeFromLeft(bound.getHeight()));
        preset_button_.getButton().setEdgeIndent(static_cast<int>(std::round(font_size * .15f)));
        bound.removeFromLeft(padding);

        top_control_panel_.setBounds(bound);
    }

    void TopPanel::repaintCallBackSlow() {
        top_control_panel_.repaintCallBackSlow();
    }
}
