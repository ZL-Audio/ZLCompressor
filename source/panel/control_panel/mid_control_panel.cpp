// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "mid_control_panel.hpp"

namespace zlpanel {
    MidControlPanel::MidControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          knee_slider_("Knee", base_),
          knee_attachment_(knee_slider_.getSlider(), p_ref_.parameters_, zlp::PKneeW::kID, updater_.getFlag()),
          curve_slider_("Curve", base_),
          curve_attachment_(curve_slider_.getSlider(), p_ref_.parameters_, zlp::PCurve::kID, updater_.getFlag()),
          th_slider_("Threshold", base_),
          th_attachment_(th_slider_.getSlider1(), p_ref_.parameters_, zlp::PThreshold::kID, updater_.getFlag()),
          ratio_slider_("Ratio", base_),
          ratio_attachment_(ratio_slider_.getSlider1(), p_ref_.parameters_, zlp::PRatio::kID, updater_.getFlag()),
          attack_slider_("Attack", base_),
          attack_attachment_(attack_slider_.getSlider1(), p_ref_.parameters_, zlp::PAttack::kID, updater_.getFlag()),
          release_slider_("Release", base_),
          release_attachment_(release_slider_.getSlider1(), p_ref_.parameters_, zlp::PRelease::kID, updater_.getFlag()),
          pump_slider_("Pump", base_),
          pump_attachment_(pump_slider_.getSlider(), p_ref_.parameters_, zlp::PPump::kID, updater_.getFlag()),
          smooth_slider_("Smooth", base_),
          smooth_attachment_(smooth_slider_.getSlider(), p_ref_.parameters_, zlp::PSmooth::kID, updater_.getFlag()) {
        juce::ignoreUnused(p_ref_, base_);

        knee_slider_.setBufferedToImage(true);
        updater_.addAttachment(knee_attachment_);
        addAndMakeVisible(knee_slider_);

        curve_slider_.setBufferedToImage(true);
        updater_.addAttachment(curve_attachment_);
        addAndMakeVisible(curve_slider_);

        th_slider_.setBufferedToImage(true);
        updater_.addAttachment(th_attachment_);
        addAndMakeVisible(th_slider_);

        ratio_slider_.setBufferedToImage(true);
        updater_.addAttachment(ratio_attachment_);
        addAndMakeVisible(ratio_slider_);

        attack_slider_.setBufferedToImage(true);
        updater_.addAttachment(attack_attachment_);
        addAndMakeVisible(attack_slider_);

        release_slider_.setBufferedToImage(true);
        updater_.addAttachment(release_attachment_);
        addAndMakeVisible(release_slider_);

        pump_slider_.setBufferedToImage(true);
        updater_.addAttachment(pump_attachment_);
        addAndMakeVisible(pump_slider_);

        smooth_slider_.setBufferedToImage(true);
        updater_.addAttachment(smooth_attachment_);
        addAndMakeVisible(smooth_slider_);
    }

    void MidControlPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillRect(getLocalBounds());
    }

    void MidControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);

        bound.removeFromTop(padding);
        bound.removeFromBottom(padding);

        const auto height = bound.getHeight();
        const auto half_height = (height - padding) / 2; {
            bound.removeFromRight(padding);
            auto t_bound = bound.removeFromRight(height);
            pump_slider_.setBounds(t_bound.removeFromTop(half_height));
            smooth_slider_.setBounds(t_bound.removeFromBottom(half_height));
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(height);
            release_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(height);
            attack_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(height);
            ratio_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(height);
            th_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            auto t_bound = bound.removeFromRight(height);
            knee_slider_.setBounds(t_bound.removeFromTop(half_height));
            curve_slider_.setBounds(t_bound.removeFromBottom(half_height));
        }
    }

    void MidControlPanel::repaintCallBack() {
        updater_.updateComponents();
    }
} // zlpanel
