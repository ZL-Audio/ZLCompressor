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
    RightControlPanel::RightControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          range_slider_("Range", base_),
          range_attachment_(range_slider_.getSlider(), p.parameters_, zlp::PRange::kID, updater_.getFlag()),
          hold_slider_("Hold", base_),
          hold_attachment_(hold_slider_.getSlider(), p.parameters_, zlp::PHold::kID, updater_.getFlag()),
          gain_slider_("Output Gain", base_, "", 1.f),
          gain_attachment_(gain_slider_.getSlider1(), p.parameters_, zlp::POutGain::kID, updater_.getFlag()),
          wet_slider_("Wet", base_),
          wet_attachment_(wet_slider_.getSlider1(), p.parameters_, zlp::PWet::kID, updater_.getFlag()),
          label_laf_(base_) {
        juce::ignoreUnused(p_ref_, base_);

        range_slider_.setBufferedToImage(true);
        updater_.addAttachment(range_attachment_);
        addAndMakeVisible(range_slider_);

        hold_slider_.setBufferedToImage(true);
        updater_.addAttachment(hold_attachment_);
        addAndMakeVisible(hold_slider_);

        label_laf_.setFontScale(1.5f);

        gain_label_.setText("Gain", juce::dontSendNotification);
        gain_label_.setJustificationType(juce::Justification::centred);
        gain_label_.setLookAndFeel(&label_laf_);
        gain_label_.setBufferedToImage(true);
        addAndMakeVisible(gain_label_);
        gain_slider_.setBufferedToImage(true);
        updater_.addAttachment(gain_attachment_);
        addAndMakeVisible(gain_slider_);

        wet_label_.setText("Wet", juce::dontSendNotification);
        wet_label_.setJustificationType(juce::Justification::centred);
        wet_label_.setLookAndFeel(&label_laf_);
        wet_label_.setBufferedToImage(true);
        addAndMakeVisible(wet_label_);
        wet_slider_.setBufferedToImage(true);
        updater_.addAttachment(wet_attachment_);
        addAndMakeVisible(wet_slider_);
    }

    void RightControlPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillRect(getLocalBounds());
    }

    void RightControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);

        bound.removeFromBottom(padding); {
            const auto height = bound.getHeight();
            const auto half_height = (height - 2 * padding) / 2;
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(height);
            t_bound.removeFromTop(padding);
            const auto extra_padding = (t_bound.getHeight() - 2 * half_height) / 4;
            t_bound.removeFromTop(extra_padding);
            t_bound.removeFromBottom(extra_padding);
            range_slider_.setBounds(t_bound.removeFromTop(half_height));
            hold_slider_.setBounds(t_bound.removeFromBottom(half_height));
        }
        const auto label_height = juce::roundToInt(base_.getFontSize() * 1.5f); {
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(bound.getHeight() - label_height);
            gain_label_.setBounds(t_bound.removeFromTop(label_height));
            gain_slider_.setBounds(t_bound);
        } {
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(bound.getHeight() - label_height);
            wet_label_.setBounds(t_bound.removeFromTop(label_height));
            wet_slider_.setBounds(t_bound);
        }
    }

    void RightControlPanel::repaintCallBack() {
        updater_.updateComponents();
    }
} // zlpanel
