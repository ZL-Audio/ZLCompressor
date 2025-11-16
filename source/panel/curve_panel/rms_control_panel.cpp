// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "rms_control_panel.hpp"

namespace zlpanel {
    RMSControlPanel::RMSControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper)
        : base_(base), updater_(),
          rms_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PRMSPanelDisplay::kID)),
          label_laf_(base),
          rms_length_label_("", "RMS Length"),
          rms_speed_label_("", "RMS Speed"),
          rms_mix_label_("", "RMS Mix"),
          rms_length_slider_("", base,
                             tooltip_helper.getToolTipText(multilingual::kRMSLength)),
          rms_length_attachment_(rms_length_slider_.getSlider(), p.parameters_,
                                 zlp::PRMSLength::kID, updater_),
          rms_speed_slider_("", base,
                            tooltip_helper.getToolTipText(multilingual::kRMSSpeed)),
          rms_speed_attachment_(rms_speed_slider_.getSlider(), p.parameters_,
                                zlp::PRMSSpeed::kID, updater_),
          rms_mix_slider_("", base,
                          tooltip_helper.getToolTipText(multilingual::kRMSMix)),
          rms_mix_attachment_(rms_mix_slider_.getSlider(), p.parameters_,
                              zlp::PRMSMix::kID, updater_) {
        label_laf_.setFontScale(1.25f);
        for (auto& l : {&rms_length_label_, &rms_speed_label_, &rms_mix_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setInterceptsMouseClicks(false, false);
            l->setJustificationType(juce::Justification::centredLeft);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        for (auto& s : {&rms_length_slider_, &rms_speed_slider_, &rms_mix_slider_}) {
            s->setFontScale(1.25f);
            s->getSlider().setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s->getSlider().setSliderSnapsToMousePosition(false);
            s->setBufferedToImage(true);
            addAndMakeVisible(s);
        }
    }

    void RMSControlPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    int RMSControlPanel::getIdealWidth() const {
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        return slider_width + slider_width / 2;
    }

    int RMSControlPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto c_height = juce::roundToInt(base_.getFontSize() * 1.75f);
        return c_height * 3 + (padding / 2) * 2;
    }

    void RMSControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto c_height = juce::roundToInt(base_.getFontSize() * 1.75f);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);

        auto bound = getLocalBounds();
        bound.removeFromLeft(padding);
        {
            auto t_bound = bound.removeFromTop(c_height);
            rms_length_label_.setBounds(t_bound.removeFromLeft(slider_width));
            rms_length_slider_.setBounds(t_bound);
        }
        bound.removeFromTop(padding / 2);
        {
            auto t_bound = bound.removeFromTop(c_height);
            rms_speed_label_.setBounds(t_bound.removeFromLeft(slider_width));
            rms_speed_slider_.setBounds(t_bound);
        }
        bound.removeFromTop(padding / 2);
        {
            auto t_bound = bound.removeFromTop(c_height);
            rms_mix_label_.setBounds(t_bound.removeFromLeft(slider_width));
            rms_mix_slider_.setBounds(t_bound);
        }
    }

    void RMSControlPanel::repaintCallBackSlow() {
        setVisible(rms_show_ref_.load(std::memory_order::relaxed) > .5f);
        if (isVisible()) {
            updater_.updateComponents();
        }
    }
} // zlpanel
