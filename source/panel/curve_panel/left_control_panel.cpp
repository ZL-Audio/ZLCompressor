// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base),
          side_control_show_drawable_(
              juce::Drawable::createFromImageData(BinaryData::link_svg, BinaryData::link_svgSize)),
          side_control_show_button_("", base, ""),
          side_control_show_attachment_(side_control_show_button_.getButton(), p.na_parameters_,
                                        zlstate::PSideControlDisplay::kID, updater_),
          equalize_show_drawable_(
              juce::Drawable::createFromImageData(BinaryData::equalizer_svg, BinaryData::equalizer_svgSize)),
          equalize_show_button_("", base, ""),
          equalize_show_attachment_(equalize_show_button_.getButton(), p.na_parameters_,
                                    zlstate::PSideEQDisplay::kID, updater_) {
        side_control_show_button_.setDrawable(side_control_show_drawable_.get());
        equalize_show_button_.setDrawable(equalize_show_drawable_.get());

        for (auto &b: {&side_control_show_button_, &equalize_show_button_}) {
            b->getLAF().enableShadow(false);
            b->getLAF().setShrinkScale(.0f);
            b->getLAF().setScale(1.25f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto height = (bound.getHeight() - kButtonNum * button_height) / kButtonNum;
        bound.removeFromBottom(height / 2);
        side_control_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        equalize_show_button_.setBounds(bound.removeFromBottom(button_height));
    }

    void LeftControlPanel::repaintCallBack(double time_stamp) {
        juce::ignoreUnused(time_stamp);
    }
} // zlpanel
