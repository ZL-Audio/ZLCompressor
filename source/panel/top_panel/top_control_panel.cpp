// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "top_control_panel.hpp"

namespace zlpanel {
    TopControlPanel::TopControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          on_drawable_(juce::Drawable::createFromImageData(BinaryData::mode_off_on_svg,
                                                           BinaryData::mode_off_on_svgSize)),
          on_button_("", base_, ""),
          on_attachment_(on_button_.getButton(), p.parameters_, zlp::PCompON::kID, updater_),
          delta_drawable_(juce::Drawable::createFromImageData(BinaryData::change_svg,
                                                              BinaryData::change_svgSize)),
          delta_button_("", base_, ""),
          delta_attachment_(delta_button_.getButton(), p.parameters_, zlp::PCompDelta::kID, updater_) {

        on_button_.setDrawable(on_drawable_.get());
        on_button_.getLAF().setScale(1.15f);
        delta_button_.setDrawable(delta_drawable_.get());
        delta_button_.getLAF().setScale(1.25f);

        for (auto &b: {&on_button_, &delta_button_}) {
            b->getLAF().enableShadow(false);
            b->getLAF().setShrinkScale(.0f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }
    }

    void TopControlPanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    int TopControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale) / 2;
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        return 2 * (button_height + padding);
    }

    void TopControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale) / 2;
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);

        auto bound = getLocalBounds();
        bound.removeFromRight(padding);
        on_button_.setBounds(bound.removeFromRight(button_height));
        bound.removeFromRight(padding);
        delta_button_.setBounds(bound.removeFromRight(button_height));
    }

    void TopControlPanel::repaintCallBack(const double time_stamp) {
        if (time_stamp - previous_time_stamp > 0.1) {
            updater_.updateComponents();
            previous_time_stamp = time_stamp;
        }
    }
}
