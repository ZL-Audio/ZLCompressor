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
    BottomControlPanel::BottomControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          time_length_box_("", zlstate::PAnalyzerTimeLength::kChoices, base_),
          time_length_attachment_(time_length_box_.getBox(), p.na_parameters_,
                                  zlstate::PAnalyzerTimeLength::kID, updater_),
          mag_type_box_("", zlstate::PAnalyzerMagType::kChoices, base_),
          mag_type_attachment_(mag_type_box_.getBox(), p.na_parameters_,
                               zlstate::PAnalyzerMagType::kID, updater_),
          min_db_box_("", zlstate::PAnalyzerMinDB::kChoices, base_),
          min_db_attachment_(min_db_box_.getBox(), p.na_parameters_,
                             zlstate::PAnalyzerMinDB::kID, updater_),
          label_laf_(base_),
          style_box_("", zlp::PCompStyle::kChoices, base_),
          style_attachment_(style_box_.getBox(), p.parameters_, zlp::PCompStyle::kID, updater_) {
        juce::ignoreUnused(p_ref_, base_);

        time_length_box_.getLAF().setLabelJustification(juce::Justification::centredRight);
        time_length_box_.getLAF().setItemJustification(juce::Justification::centredRight);

        mag_type_box_.getLAF().setLabelJustification(juce::Justification::centredBottom);
        mag_type_box_.getLAF().setItemJustification(juce::Justification::centred);

        min_db_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);
        min_db_box_.getLAF().setItemJustification(juce::Justification::centredRight);

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);

        for (auto &box: {&time_length_box_, &mag_type_box_, &min_db_box_}) {
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
        for (auto &l: {&threshold_label_, &ratio_label_, &attack_label_, &release_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setInterceptsMouseClicks(false, false);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        style_box_.setBufferedToImage(true);
        addAndMakeVisible(style_box_);

        setBufferedToImage(true);
    }

    void BottomControlPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillPath(background_path_);
    }

    int BottomControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto right_padding = juce::roundToInt(base_.getFontSize() * 1.75f);
        return (padding + slider_width) * 6 + right_padding;
    }

    void BottomControlPanel::repaintCallBack(const double time_stamp) {
        if (time_stamp - previous_time_stamp > 0.1) {
            updater_.updateComponents();
            previous_time_stamp = time_stamp;
        }
    }

    void BottomControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale); {
            const auto bound = getLocalBounds().toFloat();
            const auto sum_padding = static_cast<float>(padding + slider_width);
            background_path_.startNewSubPath(bound.getX() + sum_padding, bound.getY());
            background_path_.lineTo(bound.getX() + sum_padding - bound.getHeight(), bound.getBottom());
            background_path_.lineTo(bound.getX() + sum_padding * 6.f + bound.getHeight(), bound.getBottom());
            background_path_.lineTo(bound.getX() + sum_padding * 6.f, bound.getY());
            background_path_.closeSubPath();
        } {
            auto bound = getLocalBounds(); {
                auto box_bound = bound.removeFromLeft(button_height);
                box_bound.removeFromTop(box_bound.getHeight() / 3);
                time_length_box_.setBounds(box_bound);
                bound.removeFromLeft(slider_width - button_height);
                bound.removeFromLeft(padding);
            }

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

            bound.removeFromRight(padding / 2); {
                auto box_bound = bound.removeFromRight(
                    juce::roundToInt(base_.getFontSize() * kSliderScale * 0.3));
                box_bound.removeFromTop(box_bound.getHeight() / 3);
                min_db_box_.setBounds(box_bound);
            }

            bound.removeFromRight(padding / 2); {
                auto box_bound = bound.removeFromRight(
                    juce::roundToInt(base_.getFontSize() * kSliderScale * 0.4));
                box_bound.removeFromTop(box_bound.getHeight() / 3);
                mag_type_box_.setBounds(box_bound);
                bound.removeFromLeft(slider_width - slider_width / 2);
            }
        }
    }
}
