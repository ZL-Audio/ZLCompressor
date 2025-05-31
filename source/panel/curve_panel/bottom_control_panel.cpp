// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "bottom_control_panel.hpp"

namespace zlpanel {
    BottomControlPanel::Background::Background(zlgui::UIBase &base) : base_{base} {
        setBufferedToImage(true);
    }

    void BottomControlPanel::Background::paint(juce::Graphics &g) {
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        if (show_buttons_) {
            path.startNewSubPath(bound.getTopLeft());
            path.lineTo(bound.getBottomLeft());
            path.lineTo(bound.getBottomRight());
            path.lineTo(juce::Point(bound.getRight() - bound.getHeight(), bound.getY()).toFloat());
            path.closeSubPath();
        } else {
            const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
            const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
            const auto left_padding = static_cast<float>(padding + slider_width);
            path.startNewSubPath(bound.getX() + left_padding, bound.getY());
            path.lineTo(bound.getX() + left_padding - bound.getHeight(), bound.getBottom());
            path.lineTo(bound.getBottomRight());
            path.lineTo(juce::Point(bound.getRight() - bound.getHeight(), bound.getY()).toFloat());
            path.closeSubPath();
        }
        g.setColour(base_.getBackgroundColor());
        g.fillPath(path);
    }

    BottomControlPanel::Buttons::Buttons(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base),
          ext_side_button_("", base_, ""),
          ext_side_attachment_(ext_side_button_.getButton(), p.parameters_, zlp::PExtSide::kID, updater_),
          ext_side_drawable_(juce::Drawable::createFromImageData(
              BinaryData::externalside_svg, BinaryData::externalside_svgSize)),
          side_out_button_("", base_, ""),
          side_out_attachment_(side_out_button_.getButton(), p.parameters_, zlp::PSideOut::kID, updater_),
          side_out_drawable_(juce::Drawable::createFromImageData(
              BinaryData::fadsolo_svg, BinaryData::fadsolo_svgSize)) {
        ext_side_button_.getLAF().setDrawable(ext_side_drawable_.get());
        side_out_button_.getLAF().setDrawable(side_out_drawable_.get());

        for (auto &b: {&ext_side_button_, &side_out_button_}) {
            b->getLAF().enableShadow(false);
            b->getLAF().setShrinkScale(.0f);
            b->getLAF().setScale(1.5f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        setBufferedToImage(true);
    }

    void BottomControlPanel::Buttons::repaintCallBack() {
        updater_.updateComponents();
    }

    void BottomControlPanel::Buttons::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);

        auto bound = getLocalBounds();
        bound.removeFromLeft(padding); {
            auto t_bound = bound.removeFromLeft(slider_width);
            ext_side_button_.setBounds(t_bound.removeFromLeft(t_bound.getHeight()));
            t_bound.removeFromRight(padding);
            side_out_button_.setBounds(t_bound.removeFromLeft(t_bound.getHeight()));
        }
    }

    BottomControlPanel::BottomControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          background_(base_),
          buttons_(p, base_),
          label_laf_(base_),
          style_box_("", zlp::PCompStyle::kChoices, base_),
          style_attachment_(style_box_.getBox(), p.parameters_, zlp::PCompStyle::kID, updater_) {
        juce::ignoreUnused(p_ref_, base_);
        addChildComponent(buttons_);
        addAndMakeVisible(background_);

        threshold_label_.setText("Threshold", juce::dontSendNotification);
        ratio_label_.setText("Ratio", juce::dontSendNotification);
        attack_label_.setText("Attack", juce::dontSendNotification);
        release_label_.setText("Release", juce::dontSendNotification);

        label_laf_.setFontScale(1.5f);
        for (auto &l : {&threshold_label_, &ratio_label_, &attack_label_, &release_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setInterceptsMouseClicks(false, false);
            addAndMakeVisible(l);
        }

        addAndMakeVisible(style_box_);
    }

    int BottomControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto right_padding = juce::roundToInt(base_.getFontSize() * 1.75f);
        return (padding + slider_width) * 6 + right_padding;
    }

    void BottomControlPanel::repaintCallBack(const double time_stamp) {
        if (time_stamp - previous_time_stamp > 0.1) {
            buttons_.repaintCallBack();
            updater_.updateComponents();
            previous_time_stamp = time_stamp;
        }
    }

    void BottomControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);

        auto bound = getLocalBounds();
        background_.setBounds(bound);

        bound.removeFromLeft(padding);
        buttons_.setBounds(bound.removeFromLeft(slider_width));

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
    }
}
