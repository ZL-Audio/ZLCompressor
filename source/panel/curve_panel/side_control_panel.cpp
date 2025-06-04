// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "side_control_panel.hpp"

namespace zlpanel {
    SideControlPanel::SideControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          stereo_mode_ref_(*p.parameters_.getRawParameterValue(zlp::PSideStereoMode::kID)),
          stereo_swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSideStereoSwap::kID)),
          panel_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideControlDisplay::kID)),
          stereo_mid_side_drawable_(juce::Drawable::createFromImageData(BinaryData::midside_svg,
                                                                        BinaryData::midside_svgSize)),
          stereo_left_right_drawable_(juce::Drawable::createFromImageData(BinaryData::leftright_svg,
                                                                          BinaryData::leftright_svgSize)),
          stereo_mode_button_(base_, stereo_mid_side_drawable_.get(), stereo_left_right_drawable_.get()),
          stereo_mode_attachment_(stereo_mode_button_.getButton(), p.parameters_,
                                  zlp::PSideStereoMode::kID, updater_),
          stereo_swap_drawable_(juce::Drawable::createFromImageData(BinaryData::shuffle_svg,
                                                                    BinaryData::shuffle_svgSize)),
          stereo_swap_button_("", base_, ""),
          stereo_swap_attachment_(stereo_swap_button_.getButton(), p.parameters_,
                                  zlp::PSideStereoSwap::kID, updater_),
          stereo_link_slider_("Link", base_, ""),
          stereo_link_attachment_(stereo_link_slider_.getSlider(), p.parameters_,
                                  zlp::PSideStereoLink::kID, updater_),
          stereo_wet1_slider_("", base_, ""),
          stereo_wet1_attachment_(stereo_wet1_slider_.getSlider(), p.parameters_,
                                  zlp::PSideStereoWet1::kID, updater_),
          stereo_wet2_slider_("", base_, ""),
          stereo_wet2_attachment_(stereo_wet2_slider_.getSlider(), p.parameters_,
                                  zlp::PSideStereoWet2::kID, updater_),
          label_laf_(base_),
          side_in1_label_(), side_in2_label_(), side_out1_label_(), side_out2_label_(),
          side_gain_slider_("Gain", base_, ""),
          side_gain_attachment_(side_gain_slider_.getSlider(), p.parameters_,
                                zlp::PSideGain::kID, updater_),
          ext_side_drawable_(juce::Drawable::createFromImageData(BinaryData::externalside_svg,
                                                                 BinaryData::externalside_svgSize)),
          ext_side_button_("", base_, ""),
          ext_side_attachment_(ext_side_button_.getButton(), p.parameters_,
                               zlp::PExtSide::kID, updater_),
          side_out_drawable_(juce::Drawable::createFromImageData(BinaryData::headphones_svg,
                                                                 BinaryData::headphones_svgSize)),
          side_out_button_("", base_, ""),
          side_out_attachment_(side_out_button_.getButton(), p.parameters_,
                               zlp::PSideOut::kID, updater_) {

        stereo_swap_button_.setDrawable(stereo_swap_drawable_.get());
        ext_side_button_.setDrawable(ext_side_drawable_.get());
        side_out_button_.setDrawable(side_out_drawable_.get());
        for (auto &b: {&stereo_swap_button_, &ext_side_button_, &side_out_button_}) {
            b->getLAF().enableShadow(false);
            b->getLAF().setShrinkScale(.0f);
            b->getLAF().setScale(1.15f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        stereo_mode_button_.setImageAlpha(1.f, 1.f);
        stereo_mode_button_.setBufferedToImage(true);
        addAndMakeVisible(stereo_mode_button_);

        stereo_link_slider_.setBufferedToImage(true);
        addAndMakeVisible(stereo_link_slider_);

        updateLabels();
        label_laf_.setFontScale(1.25f);
        for (auto &l : {&side_in1_label_, &side_in2_label_, &side_out1_label_, &side_out2_label_}) {
            l->setJustificationType(juce::Justification::centred);
            l->setLookAndFeel(&label_laf_);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        for (auto &s : {&stereo_wet1_slider_, &stereo_wet2_slider_}) {
            s->setFontScale(1.25f);
            s->setBufferedToImage(true);
            addAndMakeVisible(s);
        }

        side_gain_slider_.setBufferedToImage(true);
        addAndMakeVisible(side_gain_slider_);

        setBufferedToImage(true);
    }

    void SideControlPanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    int SideControlPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        return 7 * padding + 2 * slider_height + 4 * button_height;
    }

    int SideControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        return 2 * padding + slider_width;
    }

    void SideControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);

        auto bound = getLocalBounds();
        bound.removeFromLeft(padding);
        bound.removeFromRight(padding);

        bound.removeFromTop(padding); {
            auto t_bound = bound.removeFromTop(button_height);
            const auto spacing = (t_bound.getWidth() - 2 * button_height) / 3;
            t_bound.removeFromLeft(spacing);
            t_bound.removeFromRight(spacing);
            stereo_mode_button_.setBounds(t_bound.removeFromLeft(button_height));
            stereo_swap_button_.setBounds(t_bound.removeFromRight(button_height));
        }

        bound.removeFromTop(padding);
        stereo_link_slider_.setBounds(bound.removeFromTop(slider_height));

        bound.removeFromTop(padding); {
            auto t_bound = bound.removeFromTop(button_height);
            side_in1_label_.setBounds(t_bound.removeFromLeft(button_height));
            side_out1_label_.setBounds(t_bound.removeFromRight(button_height));
            stereo_wet1_slider_.setBounds(t_bound);
        }

        bound.removeFromTop(padding); {
            auto t_bound = bound.removeFromTop(button_height);
            side_in2_label_.setBounds(t_bound.removeFromLeft(button_height));
            side_out2_label_.setBounds(t_bound.removeFromRight(button_height));
            stereo_wet2_slider_.setBounds(t_bound);
        }

        bound.removeFromTop(padding);
        side_gain_slider_.setBounds(bound.removeFromTop(slider_height));

        bound.removeFromTop(padding); {
            auto t_bound = bound.removeFromTop(button_height);
            const auto spacing = (t_bound.getWidth() - 2 * button_height) / 3;
            t_bound.removeFromLeft(spacing);
            t_bound.removeFromRight(spacing);
            ext_side_button_.setBounds(t_bound.removeFromLeft(button_height));
            side_out_button_.setBounds(t_bound.removeFromRight(button_height));
        }
    }

    void SideControlPanel::repaintCallBack(const double time_stamp) {
        if (time_stamp - previous_time_stamp > 0.1) {
            const auto stereo_mode_flag = stereo_mode_ref_.load(std::memory_order::relaxed) > .5f;
            const auto stereo_swap_flag = stereo_swap_ref_.load(std::memory_order::relaxed) > .5f;
            if (stereo_mode_flag != stereo_mode_flag_ || stereo_swap_flag != stereo_swap_flag_) {
                stereo_mode_flag_ = stereo_mode_flag;
                stereo_swap_flag_ = stereo_swap_flag;
                updateLabels();
            }
            const auto should_be_visible = panel_show_ref_.load(std::memory_order::relaxed) > .5f;
            if (should_be_visible != isVisible()) {
                setVisible(should_be_visible);
            }
            updater_.updateComponents();
            previous_time_stamp = time_stamp;
        }
    }

    void SideControlPanel::updateLabels() {
        if (stereo_mode_flag_) {
            side_in1_label_.setText("L", juce::dontSendNotification);
            side_in2_label_.setText("R", juce::dontSendNotification);
            side_out1_label_.setText(stereo_swap_flag_ ? "R" : "L", juce::dontSendNotification);
            side_out2_label_.setText(stereo_swap_flag_ ? "L" : "R", juce::dontSendNotification);
        } else {
            side_in1_label_.setText("M", juce::dontSendNotification);
            side_in2_label_.setText("S", juce::dontSendNotification);
            side_out1_label_.setText(stereo_swap_flag_ ? "S" : "M", juce::dontSendNotification);
            side_out2_label_.setText(stereo_swap_flag_ ? "M" : "S", juce::dontSendNotification);
        }
    }
} // zlpanel
