// Copyright (C) 2026 - zsliu98
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
                                     const multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        label_laf_(base),
        rms_length_label_("", "Length"),
        rms_speed_label_("", "Speed"),
        rms_mix_label_("", "Mix"),
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
        juce::ignoreUnused(base_);

        label_laf_.setFontScale(1.25f);
        for (auto& l : {&rms_length_label_, &rms_speed_label_, &rms_mix_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setInterceptsMouseClicks(false, false);
            l->setJustificationType(juce::Justification::centredRight);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        rms_mix_slider_.setComponentID(zlp::PRMSMix::kID);
        for (auto& s : {&rms_length_slider_, &rms_speed_slider_, &rms_mix_slider_}) {
            s->setPrecision(3);
            s->setFontScale(1.25f);
            s->getSlider().setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s->getSlider().setSliderSnapsToMousePosition(false);
            s->setBufferedToImage(true);
            addAndMakeVisible(s);
        }
    }

    void RMSControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto width = bound.getWidth() / 6;
        const auto delta = width / 6;
        rms_length_label_.setBounds(bound.removeFromLeft(width + delta));
        rms_length_slider_.setBounds(bound.removeFromLeft(width));
        rms_speed_label_.setBounds(bound.removeFromLeft(width + delta));
        rms_speed_slider_.setBounds(bound.removeFromLeft(width));
        rms_mix_label_.setBounds(bound.removeFromLeft(width - 2 * delta));
        rms_mix_slider_.setBounds(bound.removeFromLeft(width));
    }

    void RMSControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
