// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "right_control_panel.hpp"

namespace zlpanel {
    RightControlPanel::RightControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                         multilingual::TooltipHelper& tooltip_helper)
        : p_ref_(p), base_(base), label_laf_(base_),
          gain_slider_("Output Gain", base_,
                       tooltip_helper.getToolTipText(multilingual::kMakeup), 1.f),
          gain_attachment_(gain_slider_.getSlider1(), p.parameters_, zlp::POutGain::kID, updater_),
          wet_slider_("Wet", base_,
                      tooltip_helper.getToolTipText(multilingual::kWet), 1.f),
          wet_attachment_(wet_slider_.getSlider1(), p.parameters_, zlp::PWet::kID, updater_) {
        juce::ignoreUnused(p_ref_, base_);
        label_laf_.setFontScale(1.5f);

        gain_label_.setText("Makeup", juce::dontSendNotification);
        gain_label_.setJustificationType(juce::Justification::centred);
        gain_label_.setInterceptsMouseClicks(false, false);
        gain_label_.setLookAndFeel(&label_laf_);
        gain_label_.setBufferedToImage(true);
        addAndMakeVisible(gain_label_);
        gain_slider_.setBufferedToImage(true);
        addAndMakeVisible(gain_slider_);

        wet_label_.setText("Wet", juce::dontSendNotification);
        wet_label_.setJustificationType(juce::Justification::centred);
        wet_label_.setInterceptsMouseClicks(false, false);
        wet_label_.setLookAndFeel(&label_laf_);
        wet_label_.setBufferedToImage(true);
        addAndMakeVisible(wet_label_);
        wet_slider_.setBufferedToImage(true);
        addAndMakeVisible(wet_slider_);

        base_.getPanelValueTree().addListener(this);

        setOpaque(true);
    }

    RightControlPanel::~RightControlPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void RightControlPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getBackgroundColour());
        g.fillRect(getLocalBounds());
    }

    int RightControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto small_slider_width = juce::roundToInt(base_.getFontSize() * kSmallSliderScale);
        return padding * 3 + small_slider_width * 2;
    }

    void RightControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto small_slider_width = juce::roundToInt(base_.getFontSize() * kSmallSliderScale);

        bound.removeFromBottom(padding);
        const auto label_height = juce::roundToInt(base_.getFontSize() * 1.5f);
        {
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(small_slider_width);
            gain_label_.setBounds(t_bound.removeFromTop(label_height));
            gain_slider_.setBounds(t_bound);
        }
        {
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(small_slider_width);
            wet_label_.setBounds(t_bound.removeFromTop(label_height));
            wet_slider_.setBounds(t_bound);
        }
    }

    void RightControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void RightControlPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& identifier) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kLUFSLearnButton, identifier)) {
            if (base_.getPanelProperty(zlgui::PanelSettingIdx::kLUFSLearnButton)) {
                gain_label_.setAlpha(.5f);
                gain_slider_.setAlpha(.5f);
                wet_label_.setAlpha(.5f);
                wet_slider_.setAlpha(.5f);

                gain_slider_.setInterceptsMouseClicks(false, false);
                wet_slider_.setInterceptsMouseClicks(false, false);
            }
            else {
                gain_label_.setAlpha(1.f);
                gain_slider_.setAlpha(1.f);
                wet_label_.setAlpha(1.f);
                wet_slider_.setAlpha(1.f);

                gain_slider_.setInterceptsMouseClicks(true, true);
                wet_slider_.setInterceptsMouseClicks(true, true);
            }
        }
    }
} // zlpanel
