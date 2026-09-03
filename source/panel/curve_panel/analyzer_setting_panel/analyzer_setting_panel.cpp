// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "analyzer_setting_panel.hpp"

namespace zlpanel {
    AnalyzerSettingPanel::AnalyzerSettingPanel(PluginProcessor& p, zlgui::UIBase& base,
                                               multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
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

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards);
        for (auto& box : {&time_length_box_, &mag_stereo_box_, &mag_type_box_, &min_db_box_}) {
            box->setScrollEnabled(true);
            box->getLAF().setOption(popup_option);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        setBufferedToImage(true);
    }

    void AnalyzerSettingPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
