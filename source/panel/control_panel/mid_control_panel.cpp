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
    MidControlPanel::MidControlPanel(PluginProcessor &p, zlgui::UIBase &base,
                                     multilingual::TooltipHelper &tooltip_helper)
        : p_ref_(p), base_(base),
          knee_slider_("Knee", base_,
                       tooltip_helper.getToolTipText(multilingual::kKnee)),
          knee_attachment_(knee_slider_.getSlider(), p_ref_.parameters_, zlp::PKneeW::kID, updater_),
          curve_slider_("Curve", base_,
                        tooltip_helper.getToolTipText(multilingual::kCurve)),
          curve_attachment_(curve_slider_.getSlider(), p_ref_.parameters_, zlp::PCurve::kID, updater_),
          th_slider_("Threshold", base_,
                     tooltip_helper.getToolTipText(multilingual::kThreshold), 1.25f),
          th_attachment_(th_slider_.getSlider1(), p_ref_.parameters_, zlp::PThreshold::kID, updater_),
          ratio_slider_("Ratio", base_,
                        tooltip_helper.getToolTipText(multilingual::kRatio), 1.25f),
          ratio_attachment_(ratio_slider_.getSlider1(), p_ref_.parameters_, zlp::PRatio::kID, updater_),
          attack_slider_("Attack", base_,
                         tooltip_helper.getToolTipText(multilingual::kAttack), 1.25f),
          attack_attachment_(attack_slider_.getSlider1(), p_ref_.parameters_, zlp::PAttack::kID, updater_),
          release_slider_("Release", base_,
                          tooltip_helper.getToolTipText(multilingual::kRelease), 1.25f),
          release_attachment_(release_slider_.getSlider1(), p_ref_.parameters_, zlp::PRelease::kID, updater_),
          pump_slider_("Pump", base_,
                       tooltip_helper.getToolTipText(multilingual::kPump)),
          pump_attachment_(pump_slider_.getSlider(), p_ref_.parameters_, zlp::PPump::kID, updater_),
          smooth_slider_("Smooth", base_,
                         tooltip_helper.getToolTipText(multilingual::kSmooth)),
          smooth_attachment_(smooth_slider_.getSlider(), p_ref_.parameters_, zlp::PSmooth::kID, updater_) {
        juce::ignoreUnused(p_ref_, base_);

        knee_slider_.setBufferedToImage(true);
        addAndMakeVisible(knee_slider_);

        curve_slider_.setBufferedToImage(true);
        addAndMakeVisible(curve_slider_);

        th_slider_.setBufferedToImage(true);
        addAndMakeVisible(th_slider_);

        ratio_slider_.setBufferedToImage(true);
        addAndMakeVisible(ratio_slider_);

        attack_slider_.setBufferedToImage(true);
        addAndMakeVisible(attack_slider_);

        release_slider_.setBufferedToImage(true);
        addAndMakeVisible(release_slider_);

        pump_slider_.setBufferedToImage(true);
        addAndMakeVisible(pump_slider_);

        smooth_slider_.setBufferedToImage(true);
        addAndMakeVisible(smooth_slider_);

        setOpaque(true);
    }

    void MidControlPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillRect(getLocalBounds());
    }

    int MidControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        // const auto small_slider_width = juce::roundToInt(base_.getFontSize() * kSmallSliderScale);
        return padding * 7 + slider_width * 6;
    }

    void MidControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        bound.removeFromTop(padding);
        bound.removeFromBottom(padding); {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(slider_width);
            release_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(slider_width);
            attack_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            auto t_bound = bound.removeFromRight(slider_width);
            const auto extra_padding = (t_bound.getHeight() - 2 * slider_height) / 4;
            t_bound.removeFromTop(extra_padding);
            t_bound.removeFromBottom(extra_padding);
            pump_slider_.setBounds(t_bound.removeFromTop(slider_height));
            smooth_slider_.setBounds(t_bound.removeFromBottom(slider_height));
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(slider_width);
            ratio_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            const auto t_bound = bound.removeFromRight(slider_width);
            th_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            auto t_bound = bound.removeFromRight(slider_width);
            const auto extra_padding = (t_bound.getHeight() - 2 * slider_height) / 4;
            t_bound.removeFromTop(extra_padding);
            t_bound.removeFromBottom(extra_padding);
            knee_slider_.setBounds(t_bound.removeFromTop(slider_height));
            curve_slider_.setBounds(t_bound.removeFromBottom(slider_height));
        }
    }

    void MidControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
} // zlpanel
