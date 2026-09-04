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
        control_background_(base),
        mag_type_box_(zlstate::PAnalyzerMagType::kChoices, base,
                      tooltip_helper.getToolTipText(multilingual::kMagMeasureMethod)),
        mag_type_attachment_(mag_type_box_.getBox(), p.na_parameters_,
                             zlstate::PAnalyzerMagType::kID, updater_),
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
        move_type_box_(zlstate::PAnalyzerMoveType::kChoices, base, ""),
        move_type_attachment_(move_type_box_.getBox(), p.na_parameters_,
                              zlstate::PAnalyzerMoveType::kID, updater_),
        pre_button_(base, "Pre", ""),
        pre_button_attachment_(pre_button_.getButton(), p.na_parameters_,
                               zlstate::PPreCurveDisplay::kID, updater_),
        post_button_(base, "Post", ""),
        post_button_attachment_(post_button_.getButton(), p.na_parameters_,
                                zlstate::PPostCurveDisplay::kID, updater_),
        delta_button_(base, "Delta", ""),
        delta_button_attachment_(delta_button_.getButton(), p.na_parameters_,
                                 zlstate::PDeltaCurveDisplay::kID, updater_),
        side_button_(base, "Side", ""),
        side_button_attachment_(side_button_.getButton(), p.na_parameters_,
                                zlstate::PSideChainCurveDisplay::kID, updater_),
        time_length_box_(zlstate::PAnalyzerTimeLength::kChoices, base,
                         tooltip_helper.getToolTipText(multilingual::kMagAnalyzerTimeLength)),
        time_length_attachment_(time_length_box_.getBox(), p.na_parameters_,
                                zlstate::PAnalyzerTimeLength::kID, updater_),
        max_db_box_(zlstate::PAnalyzerMaxDB::kChoices, base, ""),
        max_db_attachment_(max_db_box_.getBox(), p.na_parameters_,
                           zlstate::PAnalyzerMaxDB::kID, updater_),
        min_db_box_(zlstate::PAnalyzerMinDB::kChoices, base,
                    tooltip_helper.getToolTipText(multilingual::kMagAnalyzerMinDB)),
        min_db_attachment_(min_db_box_.getBox(), p.na_parameters_,
                           zlstate::PAnalyzerMinDB::kID, updater_),
        label_laf_(base),
        delimiter_label_("", "~"),
        db_label_("", "dB") {

        control_background_.setBufferedToImage(true);
        addAndMakeVisible(control_background_);

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::downwards);
        for (auto& box : {&mag_type_box_, &mag_stereo_box_, &move_type_box_}) {
            box->setScrollEnabled(true);
            box->getLAF().setOption(popup_option);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }
        for (auto& button : {&pre_button_, &post_button_, &delta_button_, &side_button_}) {
            button->getButton().setToggleable(true);
            button->getButton().setClickingTogglesState(true);
            button->getLAF().setFontScale(1.5f);
            button->getLAF().setJustification(juce::Justification::centred);
            button->setBufferedToImage(true);
            addAndMakeVisible(button);
        }
        for (auto& box : {&time_length_box_, &max_db_box_, &min_db_box_}) {
            box->setScrollEnabled(true);
            box->getLAF().setOption(popup_option);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }
        label_laf_.setFontScale(1.5f);
        db_label_.setJustificationType(juce::Justification::centredLeft);
        for (auto &label: {&delimiter_label_, &db_label_}) {
            label->setLookAndFeel(&label_laf_);
            label->setBufferedToImage(true);
            addAndMakeVisible(label);
        }

        base_.getPanelValueTree().addListener(this);

        setBufferedToImage(true);
    }

    AnalyzerSettingPanel::~AnalyzerSettingPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    int AnalyzerSettingPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);

        return 8 * padding + 4 * (slider_width / 2);
    }

    int AnalyzerSettingPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_height = getButtonSize(font_size);

        return 4 * padding + 3 * button_height;
    }

    void AnalyzerSettingPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto button_height = getButtonSize(font_size);
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        control_background_.setBounds(bound);

        bound.reduce(2 * padding, padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            const auto box_width = t_bound.getWidth() / 3;
            mag_type_box_.setBounds(t_bound.removeFromLeft(box_width));
            move_type_box_.setBounds(t_bound.removeFromRight(box_width));
            mag_stereo_box_.getLAF().setPadding(font_size * 1.75f);
            mag_stereo_box_.setBounds(t_bound);
        }
        bound.removeFromTop(padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            const auto button_width = (t_bound.getWidth() - padding) / 4;
            pre_button_.setBounds(t_bound.removeFromLeft(button_width));
            post_button_.setBounds(t_bound.removeFromLeft(button_width));
            side_button_.setBounds(t_bound.removeFromRight(button_width));
            delta_button_.setBounds(t_bound);
        }
        bound.removeFromTop(padding);
        {
            auto t_bound = bound.removeFromTop(button_height);
            const auto box_width = (t_bound.getWidth() - 8 * padding) / 3;
            time_length_box_.setBounds(t_bound.removeFromLeft(box_width));
            db_label_.setBounds(t_bound.removeFromRight(5 * padding));
            min_db_box_.setBounds(t_bound.removeFromRight(box_width));
            delimiter_label_.setBounds(t_bound.removeFromRight(3 * padding));
            max_db_box_.setBounds(t_bound.removeFromRight(box_width));
        }
    }

    void AnalyzerSettingPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        updateMinDBChoices(max_db_box_.getBox().getSelectedItemIndex());
    }

    void AnalyzerSettingPanel::updateMinDBChoices(const int max_db_idx) {
        if (max_db_idx < 0 || max_db_idx == c_max_db_idx_) {
            return;
        }

        auto& box = min_db_box_.getBox();
        const auto selected_idx = box.getSelectedItemIndex();
        const auto max_db = zlstate::PAnalyzerMaxDB::kDBs[static_cast<size_t>(max_db_idx)];
        // The parameter stores a span; present the absolute floor produced by the selected maximum.
        for (size_t i = 0; i < zlstate::PAnalyzerMinDB::kDBs.size(); ++i) {
            const auto min_db = static_cast<int>(std::round(max_db + zlstate::PAnalyzerMinDB::kDBs[i]));
            box.changeItemText(static_cast<int>(i + 1), juce::String(min_db));
        }
        box.setSelectedItemIndex(selected_idx, juce::dontSendNotification);
        min_db_box_.repaint();
        c_max_db_idx_ = max_db_idx;
    }

    void AnalyzerSettingPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kAnalyzerSettingPanel, property)) {
            setVisible(static_cast<double>(base_.getPanelProperty(zlgui::PanelSettingIdx::kAnalyzerSettingPanel)) > .5);
        }
    }
}
