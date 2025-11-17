// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "BinaryData.h"
#include "top_control_panel.hpp"

namespace zlpanel {
    TopControlPanel::TopControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                     multilingual::TooltipHelper& tooltip_helper)
        : p_ref_(p), base_(base), label_laf_(base_),
          on_drawable_(juce::Drawable::createFromImageData(BinaryData::mode_off_on_svg,
                                                           BinaryData::mode_off_on_svgSize)),
          on_button_("", base_,
                     tooltip_helper.getToolTipText(multilingual::TooltipLabel::kBypass)),
          on_attachment_(on_button_.getButton(), p.parameters_, zlp::PCompON::kID, updater_),
          delta_drawable_(juce::Drawable::createFromImageData(BinaryData::change_svg,
                                                              BinaryData::change_svgSize)),
          delta_button_("", base_,
                        tooltip_helper.getToolTipText(multilingual::TooltipLabel::kDelta)),
          delta_attachment_(delta_button_.getButton(), p.parameters_, zlp::PCompDelta::kID, updater_),
          lookahead_label_("Lookahead", "Lookahead"),
          lookahead_slider_("", base_,
                            tooltip_helper.getToolTipText(multilingual::TooltipLabel::kLookahead)),
          lookahead_attachment_(lookahead_slider_.getSlider(), p.parameters_, zlp::PLookAhead::kID, updater_),
          oversample_label_("Oversample", "Oversample"),
          oversample_box_(zlp::POversample::kChoices, base_,
                          tooltip_helper.getToolTipText(multilingual::TooltipLabel::kOversample)),
          oversample_attachment_(oversample_box_.getBox(), p.parameters_, zlp::POversample::kID, updater_),
          clipper_label_("Clipper", "Clipper"),
          clipper_slider_("", base_,
                          tooltip_helper.getToolTipText(multilingual::TooltipLabel::kClipper)),
          clipper_attachment_(clipper_slider_.getSlider(), p.parameters_, zlp::PClipperDrive::kID, updater_) {
        on_button_.setDrawable(on_drawable_.get());
        on_button_.getLAF().setScale(1.15f);
        delta_button_.setDrawable(delta_drawable_.get());
        delta_button_.getLAF().setScale(1.25f);

        for (auto& b : {&on_button_, &delta_button_}) {
            b->getLAF().enableShadow(false);
            b->getLAF().setShrinkScale(.0f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        label_laf_.setFontScale(1.25f);

        lookahead_slider_.setFontScale(1.25f);
        lookahead_slider_.setJustification(juce::Justification::centred);
        lookahead_slider_.setBufferedToImage(true);
        lookahead_slider_.getSlider().setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        lookahead_slider_.getSlider().setSliderSnapsToMousePosition(false);
        addAndMakeVisible(lookahead_slider_);
        lookahead_label_.setLookAndFeel(&label_laf_);
        lookahead_label_.setJustificationType(juce::Justification::centredRight);
        lookahead_label_.setBufferedToImage(true);
        lookahead_label_.setTooltip(tooltip_helper.getToolTipText(multilingual::TooltipLabel::kLookahead));
        addAndMakeVisible(lookahead_label_);

        oversample_box_.getLAF().setFontScale(1.25f);
        oversample_box_.setBufferedToImage(true);
        addAndMakeVisible(oversample_box_);
        oversample_label_.setLookAndFeel(&label_laf_);
        oversample_label_.setJustificationType(juce::Justification::centredRight);
        oversample_label_.setBufferedToImage(true);
        oversample_label_.setTooltip(tooltip_helper.getToolTipText(multilingual::TooltipLabel::kOversample));
        addAndMakeVisible(oversample_label_);

        clipper_slider_.setFontScale(1.25f);
        clipper_slider_.setJustification(juce::Justification::centred);
        clipper_slider_.setBufferedToImage(true);
        clipper_slider_.getSlider().setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        clipper_slider_.getSlider().setSliderSnapsToMousePosition(false);
        addAndMakeVisible(clipper_slider_);
        clipper_label_.setLookAndFeel(&label_laf_);
        clipper_label_.setJustificationType(juce::Justification::centredRight);
        clipper_label_.setBufferedToImage(true);
        clipper_label_.setTooltip(tooltip_helper.getToolTipText(multilingual::TooltipLabel::kClipper));
        addAndMakeVisible(clipper_label_);

        setLookaheadAlpha(.5f);
        setOversampleAlpha(.5f);
        setClipperAlpha(.5f);
    }

    int TopControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale) / 2;
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto small_slider_width = juce::roundToInt(base_.getFontSize() * kSmallSliderScale) / 2;
        return 2 * button_height + 5 * padding + 3 * slider_width + 3 * small_slider_width;
    }

    void TopControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale) / 2;
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto small_slider_width = juce::roundToInt(base_.getFontSize() * kSmallSliderScale) / 2;

        auto bound = getLocalBounds();
        bound.removeFromRight(padding);
        on_button_.setBounds(bound.removeFromRight(button_height));
        bound.removeFromRight(padding);
        delta_button_.setBounds(bound.removeFromRight(button_height));
        bound.removeFromRight(padding);
        lookahead_slider_.setBounds(bound.removeFromRight(small_slider_width));
        bound.removeFromRight(padding);
        lookahead_label_.setBounds(bound.removeFromRight(slider_width));
        bound.removeFromRight(padding);
        oversample_box_.setBounds(bound.removeFromRight(small_slider_width));
        bound.removeFromRight(padding);
        oversample_label_.setBounds(bound.removeFromRight(slider_width));
        bound.removeFromRight(padding);
        clipper_slider_.setBounds(bound.removeFromRight(small_slider_width));
        bound.removeFromRight(padding);
        clipper_label_.setBounds(bound.removeFromRight(slider_width));
    }

    void TopControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        const auto new_lookahead_value = lookahead_slider_.getSlider().getValue();
        if (std::abs(new_lookahead_value - old_lookahead_value_) > 1e-4) {
            if (new_lookahead_value < 1e-4 && old_lookahead_value_ > 1e-4) {
                setLookaheadAlpha(.5f);
            }
            if (new_lookahead_value > 1e-4 && old_lookahead_value_ < 1e-4) {
                setLookaheadAlpha(1.f);
            }
            old_lookahead_value_ = new_lookahead_value;
        }

        const auto new_oversample_id = oversample_box_.getBox().getSelectedItemIndex();
        if (new_oversample_id != old_oversample_id_) {
            setOversampleAlpha(new_oversample_id > 0 ? 1.f : .5f);
            old_oversample_id_ = new_oversample_id;
        }

        const auto new_clipper_value = clipper_slider_.getSlider().getValue();
        if (std::abs(new_clipper_value - old_clipper_value_) > 1e-4) {
            if (new_clipper_value < 1e-4 && old_clipper_value_ > 1e-4) {
                setClipperAlpha(.5f);
            }
            if (new_clipper_value > 1e-4 && old_clipper_value_ < 1e-4) {
                setClipperAlpha(1.f);
            }
            old_clipper_value_ = new_clipper_value;
        }
    }

    void TopControlPanel::setOversampleAlpha(const float alpha) {
        oversample_box_.setAlpha(alpha);
        oversample_label_.setAlpha(alpha);
    }

    void TopControlPanel::setLookaheadAlpha(const float alpha) {
        lookahead_slider_.setAlpha(alpha);
        lookahead_label_.setAlpha(alpha);
    }

    void TopControlPanel::setClipperAlpha(const float alpha) {
        clipper_slider_.setAlpha(alpha);
        clipper_label_.setAlpha(alpha);
    }
}
