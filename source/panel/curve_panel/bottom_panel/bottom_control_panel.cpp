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
    BottomControlPanel::BottomControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                           multilingual::TooltipHelper& tooltip_helper)
        : p_ref_(p), base_(base),
          side_control_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideControlDisplay::kID)),
          side_eq_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideEQDisplay::kID)),
          time_length_box_(zlstate::PAnalyzerTimeLength::kChoices, base),
          time_length_attachment_(time_length_box_.getBox(), p.na_parameters_,
                                  zlstate::PAnalyzerTimeLength::kID, updater_),
          mag_type_box_(zlstate::PAnalyzerMagType::kChoices, base),
          mag_type_attachment_(mag_type_box_.getBox(), p.na_parameters_,
                               zlstate::PAnalyzerMagType::kID, updater_),
          min_db_box_(zlstate::PAnalyzerMinDB::kChoices, base),
          min_db_attachment_(min_db_box_.getBox(), p.na_parameters_,
                             zlstate::PAnalyzerMinDB::kID, updater_),
          label_laf_(base),
          style_box_(zlp::PCompStyle::kChoices, base,
                     tooltip_helper.getToolTipText(multilingual::kCompressionStyle)),
          style_attachment_(style_box_.getBox(), p.parameters_, zlp::PCompStyle::kID, updater_),
          rms_button_(p, base, tooltip_helper),
          lufs_button_(p, base, tooltip_helper) {
        juce::ignoreUnused(p_ref_, base_);

        time_length_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        time_length_box_.getLAF().setItemJustification(juce::Justification::centredRight);

        mag_type_box_.getLAF().setLabelJustification(juce::Justification::centredBottom);
        mag_type_box_.getLAF().setItemJustification(juce::Justification::centred);

        min_db_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);
        min_db_box_.getLAF().setItemJustification(juce::Justification::centredRight);

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);

        for (auto& box : {&time_length_box_, &mag_type_box_, &min_db_box_}) {
            box->getLAF().setFontScale(1.f);
            box->getLAF().setOption(popup_option);
            box->setAlpha(.5f);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

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

        addAndMakeVisible(rms_button_);

        addAndMakeVisible(lufs_button_);

        setBufferedToImage(true);
    }

    void BottomControlPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getBackgroundColour());
        g.fillPath(show_path1_ ? background_path1_ : background_path0_);
    }

    void BottomControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        rms_button_.repaintCallBackSlow();

        const auto f1 = side_control_show_ref_.load(std::memory_order::relaxed) > .5f;
        const auto f2 = side_eq_show_ref_.load(std::memory_order::relaxed) > .5f;
        if (const auto f = f1 || f2; f != show_path1_) {
            show_path1_ = f;
            repaint();
        }
    }

    void BottomControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto small_slider_width = juce::roundToInt(base_.getFontSize() * kSmallSliderScale);
        const auto left_padding = (getWidth() - (padding * 11 + slider_width * 7 + small_slider_width * 2)) / 2;
        {
            auto bound = getLocalBounds().toFloat();
            bound.removeFromLeft(static_cast<float>(left_padding));
            const auto sum_padding = static_cast<float>(padding + slider_width);

            background_path0_.clear();
            background_path0_.startNewSubPath(bound.getX() + sum_padding, bound.getY());
            background_path0_.lineTo(bound.getX() + sum_padding - bound.getHeight(), bound.getBottom());
            background_path0_.lineTo(bound.getX() + sum_padding * 6.f + bound.getHeight(), bound.getBottom());
            background_path0_.lineTo(bound.getX() + sum_padding * 6.f, bound.getY());
            background_path0_.closeSubPath();

            background_path1_.clear();
            background_path1_.startNewSubPath(bound.getTopLeft());
            background_path1_.lineTo(bound.getBottomLeft());
            background_path1_.lineTo(bound.getX() + sum_padding * 6.f + bound.getHeight(), bound.getBottom());
            background_path1_.lineTo(bound.getX() + sum_padding * 6.f, bound.getY());
            background_path1_.closeSubPath();
        }
        {
            auto bound = getLocalBounds();
            auto box_bound = bound.removeFromLeft(button_height);
            box_bound.removeFromTop(box_bound.getHeight() / 3);
            time_length_box_.setBounds(box_bound);
        }
        {
            auto bound = getLocalBounds();
            bound.removeFromLeft(slider_width + 2 * padding + left_padding);
            threshold_label_.setBounds(bound.removeFromLeft(slider_width));

            bound.removeFromLeft(padding);
            ratio_label_.setBounds(bound.removeFromLeft(slider_width));

            bound.removeFromLeft(padding);
            style_box_.setBounds(bound.removeFromLeft(slider_width));

            bound.removeFromLeft(padding);
            attack_label_.setBounds(bound.removeFromLeft(slider_width));

            bound.removeFromLeft(padding);
            release_label_.setBounds(bound.removeFromLeft(slider_width));

            rms_button_.setBounds(bound.removeFromLeft(rms_button_.getIdealWidth()));

            bound.removeFromRight(padding / 2);
            {
                auto box_bound = bound.removeFromRight(
                    juce::roundToInt(base_.getFontSize() * kSliderScale * 0.3f));
                box_bound.removeFromTop(box_bound.getHeight() / 3);
                min_db_box_.setBounds(box_bound);
            }

            bound.removeFromRight(padding / 2);
            {
                auto box_bound = bound.removeFromRight(
                    juce::roundToInt(base_.getFontSize() * kSliderScale * 0.4f));
                box_bound.removeFromTop(box_bound.getHeight() / 3);
                mag_type_box_.setBounds(box_bound);
                bound.removeFromLeft(slider_width - slider_width / 2);
            }
        }
        {
            auto bound = getLocalBounds();
            bound.removeFromRight(2 * padding + small_slider_width);
            lufs_button_.setBounds(bound.removeFromRight(small_slider_width));
        }
    }
}
