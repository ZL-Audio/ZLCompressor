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
        pre_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PPreCurveDisplay::kID)),
        post_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PPostCurveDisplay::kID)),
        delta_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PDeltaCurveDisplay::kID)),
        side_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideChainCurveDisplay::kID)),
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
        analyzer_setting_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_magnitude_svg,
                                                                            BinaryData::dline_magnitude_svgSize)),
        analyzer_setting_show_button_(base, analyzer_setting_show_drawable_.get(),
                                      analyzer_setting_show_drawable_.get(), "") {
        juce::ignoreUnused(tooltip_helper);

        for (auto& b : {&side_control_show_button_, &equalize_show_button_,
                        &computer_show_button_, &rms_show_button_, &meter_show_button_
             }) {
            b->setImageAlpha(.5f, .75f, 1.f, 1.f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }
        {
            analyzer_setting_show_button_.setImageAlpha(1.f, 1.f, 1.f, 1.f);
            analyzer_setting_show_button_.setBufferedToImage(true);
            analyzer_setting_show_button_.getButton().onClick = [this] {
                base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerSettingPanel,
                                       analyzer_setting_show_button_.getToggleState() ? 1. : 0.);
            };
            addAndMakeVisible(analyzer_setting_show_button_);
        }

        setBufferedToImage(true);
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto height = (bound.getHeight() - kButtonNum * button_height) / kButtonNum;
        bound.removeFromBottom(height / 2);
        meter_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        side_control_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        equalize_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        computer_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        rms_show_button_.setBounds(bound.removeFromBottom(button_height));
        bound.removeFromBottom(height);
        analyzer_setting_show_button_.setBounds(bound.removeFromBottom(button_height));
    }

    void LeftControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        const auto pre_on = pre_curve_display_ref_.load(std::memory_order::relaxed) > .5f;
        const auto post_on = post_curve_display_ref_.load(std::memory_order::relaxed) > .5f;
        const auto delta_on = delta_curve_display_ref_.load(std::memory_order::relaxed) > .5f;
        const auto side_on = side_curve_display_ref_.load(std::memory_order::relaxed) > .5f;
        const auto curve_on = pre_on || post_on || delta_on || side_on;
        if (curve_on && analyzer_setting_show_button_.getAlpha() < .9f) {
            analyzer_setting_show_button_.setAlpha(1.f);
        } else if (!curve_on && analyzer_setting_show_button_.getAlpha() > .6f) {
            analyzer_setting_show_button_.setAlpha(.5f);
        }
    }
}
