// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                       multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        side_control_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_link_svg,
                                                                        BinaryData::dline_link_svgSize)),
        side_control_show_button_(base, side_control_show_drawable_.get(), side_control_show_drawable_.get(),
                                  tooltip_helper.getToolTipText(multilingual::kSideControlPanel)),
        side_control_show_attachment_(side_control_show_button_.getButton(), p.na_parameters_,
                                      zlstate::PSideControlDisplay::kID, updater_),
        equalize_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_eq_svg,
                                                                    BinaryData::dline_eq_svgSize)),
        equalize_show_button_(base, equalize_show_drawable_.get(), equalize_show_drawable_.get(),
                              tooltip_helper.getToolTipText(multilingual::kSideEQPanel)),
        equalize_show_attachment_(equalize_show_button_.getButton(), p.na_parameters_,
                                  zlstate::PSideEQDisplay::kID, updater_),
        computer_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_computer_svg,
                                                                    BinaryData::dline_computer_svgSize)),
        computer_show_button_(base, computer_show_drawable_.get(), computer_show_drawable_.get(),
                              tooltip_helper.getToolTipText(multilingual::kGainTransferPanel)),
        computer_show_attachment_(computer_show_button_.getButton(), p.na_parameters_,
                                  zlstate::PComputerCurveDisplay::kID, updater_),
        rms_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_rms_svg,
                                                               BinaryData::dline_rms_svgSize)),
        rms_show_button_(base, rms_show_drawable_.get(), rms_show_drawable_.get(),
                         tooltip_helper.getToolTipText(multilingual::kCumuRMSPanel)),
        rms_show_attachment_(rms_show_button_.getButton(), p.na_parameters_,
                             zlstate::PRMSAnalyzerDisplay::kID, updater_),
        meter_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_meter_svg,
                                                                 BinaryData::dline_meter_svgSize)),
        meter_show_button_(base, meter_show_drawable_.get(), meter_show_drawable_.get(),
                           tooltip_helper.getToolTipText(multilingual::kMeterPanel)),
        meter_show_attachment_(meter_show_button_.getButton(), p.na_parameters_,
                               zlstate::PMeterDisplay::kID, updater_),
        time_length_box_(zlstate::PAnalyzerTimeLength::kChoices, base,
                         tooltip_helper.getToolTipText(multilingual::kMagAnalyzerTimeLength)),
        time_length_attachment_(time_length_box_.getBox(), p.na_parameters_,
                                zlstate::PAnalyzerTimeLength::kID, updater_),
        mag_stereo_box_([]() -> std::vector<std::unique_ptr<juce::Drawable>> {
            std::vector<std::unique_ptr<juce::Drawable>> icons;
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::stereo_svg, BinaryData::stereo_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::left_svg, BinaryData::left_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::right_svg, BinaryData::right_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::mid_svg, BinaryData::mid_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::side_svg, BinaryData::side_svgSize));
            return icons;
        }(), base, tooltip_helper.getToolTipText(multilingual::kMagMeasureStereo)),
        mag_stereo_attachment_(mag_stereo_box_.getBox(), p.na_parameters_,
                               zlstate::PAnalyzerStereo::kID, updater_),
        mag_type_box_(zlstate::PAnalyzerMagType::kChoices, base,
                      tooltip_helper.getToolTipText(multilingual::kMagMeasureMethod)),
        mag_type_attachment_(mag_type_box_.getBox(), p.na_parameters_,
                             zlstate::PAnalyzerMagType::kID, updater_),
        min_db_box_(zlstate::PAnalyzerMinDB::kChoices, base,
                    tooltip_helper.getToolTipText(multilingual::kMagAnalyzerMinDB)),
        min_db_attachment_(min_db_box_.getBox(), p.na_parameters_,
                           zlstate::PAnalyzerMinDB::kID, updater_) {
        juce::ignoreUnused(tooltip_helper);

        for (auto& b : {&side_control_show_button_, &equalize_show_button_,
                        &computer_show_button_, &rms_show_button_, &meter_show_button_
             }) {
            b->setImageAlpha(.5f, .75f, 1.f, 1.f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);
        for (auto& box : {&time_length_box_, &mag_stereo_box_, &mag_type_box_, &min_db_box_}) {
            box->getLAF().setFontScale(1.f);
            box->getLAF().setOption(popup_option);
            box->setAlpha(.5f);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        setBufferedToImage(true);
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
        auto setting_bound = bound.removeFromBottom(getControlPanelHeight(base_.getFontSize()));
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto height = (bound.getHeight() - kButtonNum * button_height) / kButtonNum;
        bound.removeFromBottom(height / 2);
        side_control_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        equalize_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        computer_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        rms_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        meter_show_button_.setBounds(bound.removeFromBottom(button_height));

        const auto box_height = setting_bound.getHeight() / 6;
        const auto box_padding = setting_bound.getHeight() / 12;
        for (auto& box : {&min_db_box_, &time_length_box_, &mag_stereo_box_, &mag_type_box_}) {
            box->setBounds(setting_bound.removeFromBottom(box_height));
            setting_bound.removeFromBottom(box_padding);
        }
    }

    void LeftControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
