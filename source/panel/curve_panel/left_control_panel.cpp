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
        side_control_show_button_(base, side_control_show_drawable_.get(), side_control_show_drawable_.get()),
        side_control_show_attachment_(side_control_show_button_.getButton(), p.na_parameters_,
                                      zlstate::PSideControlDisplay::kID, updater_),
        equalize_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_eq_svg,
                                                                    BinaryData::dline_eq_svgSize)),
        equalize_show_button_(base, equalize_show_drawable_.get(), equalize_show_drawable_.get()),
        equalize_show_attachment_(equalize_show_button_.getButton(), p.na_parameters_,
                                  zlstate::PSideEQDisplay::kID, updater_),
        computer_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_computer_svg,
                                                                    BinaryData::dline_computer_svgSize)),
        computer_show_button_(base, computer_show_drawable_.get(), computer_show_drawable_.get()),
        computer_show_attachment_(computer_show_button_.getButton(), p.na_parameters_,
                                  zlstate::PComputerCurveDisplay::kID, updater_),
        rms_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_rms_svg,
                                                               BinaryData::dline_rms_svgSize)),
        rms_show_button_(base, rms_show_drawable_.get(), rms_show_drawable_.get()),
        rms_show_attachment_(rms_show_button_.getButton(), p.na_parameters_,
                             zlstate::PRMSAnalyzerDisplay::kID, updater_),
        meter_show_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_meter_svg,
                                                                 BinaryData::dline_meter_svgSize)),
        meter_show_button_(base, meter_show_drawable_.get(), meter_show_drawable_.get()),
        meter_show_attachment_(meter_show_button_.getButton(), p.na_parameters_,
                               zlstate::PMeterDisplay::kID, updater_) {
        juce::ignoreUnused(tooltip_helper);

        for (auto& b : {
                 &side_control_show_button_, &equalize_show_button_,
                 &computer_show_button_, &rms_show_button_, &meter_show_button_
             }) {
            b->setImageAlpha(.5f, .75f, 1.f, 1.f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }
        setBufferedToImage(true);
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
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
    }

    void LeftControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
