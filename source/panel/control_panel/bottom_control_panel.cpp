// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "bottom_control_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    BottomControlPanel::BottomControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                           const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base),
        rms_control_panel_(p, base, tooltip_helper),
        rms_on_ref_(*p.parameters_.getRawParameterValue(zlp::PRMSON::kID)),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        label_laf_(base),
        style_box_(zlp::PCompStyle::kChoices, base,
                   tooltip_helper.getToolTipText(multilingual::kCompressionStyle)),
        style_attachment_(style_box_.getBox(), p.parameters_, zlp::PCompStyle::kID, updater_),
    rms_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_r_svg,
                                                          BinaryData::dline_r_svgSize)),
        rms_button_(base, rms_drawable_.get(), rms_drawable_.get(),
                    tooltip_helper.getToolTipText(multilingual::kRMSCompress)),
        rms_attachment_(rms_button_.getButton(), p.parameters_,
                        zlp::PRMSON::kID, updater_) {
        juce::ignoreUnused(p_ref_, base_);

        label_laf_.setFontScale(1.5f);
        threshold_label_.setText("Threshold", juce::dontSendNotification);
        ratio_label_.setText("Ratio", juce::dontSendNotification);
        attack_label_.setText("Attack", juce::dontSendNotification);
        release_label_.setText("Release", juce::dontSendNotification);
        for (auto& l : {&threshold_label_, &ratio_label_, &attack_label_, &release_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setInterceptsMouseClicks(false, false);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        style_box_.setBufferedToImage(true);
        addAndMakeVisible(style_box_);

        rms_button_.setImageAlpha(.5f, .5f, 1.f, 1.f);
        rms_button_.setBufferedToImage(true);
        addAndMakeVisible(rms_button_);

        addChildComponent(rms_control_panel_);

        setInterceptsMouseClicks(false, true);
    }

    void BottomControlPanel::paint(juce::Graphics&) {
    }

    void BottomControlPanel::repaintCallBackSlow() {
        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        if (direction != c_comp_direction_) {
            c_comp_direction_ = direction;
            const auto f = (c_comp_direction_ == zlp::PCompDirection::kCompress
                || c_comp_direction_ == zlp::PCompDirection::kShape);
            if (!f && style_box_.getBox().getSelectedId() >= 3) {
                style_box_.getBox().setSelectedId(1, juce::sendNotificationSync);
            }
            style_box_.getBox().setItemEnabled(3, f);
            style_box_.getBox().setItemEnabled(4, f);
        }
        const auto rms_on = rms_on_ref_.load(std::memory_order::relaxed) > .5f;
        if (rms_on != rms_control_panel_.isVisible()) {
            rms_control_panel_.setVisible(rms_on);
        }
        updater_.updateComponents();
    }

    void BottomControlPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        auto bound = getLocalBounds();
        bound.removeFromLeft(padding);
        bound.removeFromLeft(slider_width);
        bound.removeFromLeft(padding);
        threshold_label_.setBounds(bound.removeFromLeft(slider_width));

        bound.removeFromLeft(padding);
        ratio_label_.setBounds(bound.removeFromLeft(slider_width));

        bound.removeFromLeft(padding);
        style_box_.setBounds(bound.removeFromLeft(slider_width));

        bound.removeFromLeft(padding);
        attack_label_.setBounds(bound.removeFromLeft(slider_width));

        bound.removeFromLeft(padding);
        release_label_.setBounds(bound.removeFromLeft(slider_width));

        rms_button_.setBounds(bound.removeFromLeft(button_size));
        bound.removeFromLeft(padding);

        rms_control_panel_.setBounds(bound);
    }

    int BottomControlPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        return getButtonSize(font_size);
    }
}
