// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          knee_slider_("Knee", base_),
          knee_attachment_(knee_slider_.getSlider(), p_ref_.parameters_, zlp::PKneeW::kID, updater_.getFlag()),
          curve_slider_("Curve", base_),
          curve_attachment_(curve_slider_.getSlider(), p_ref_.parameters_, zlp::PCurve::kID, updater_.getFlag()),
          th_slider_("TH", base_),
          th_attachment_(th_slider_.getSlider1(), p_ref_.parameters_, zlp::PThreshold::kID, updater_.getFlag()),
          ratio_slider_("Ratio", base_),
          ratio_attachment_(ratio_slider_.getSlider1(), p_ref_.parameters_, zlp::PRatio::kID, updater_.getFlag()) {
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
    }

    void LeftControlPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillRect(getLocalBounds());
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto height = bound.getHeight();
        const auto padding = static_cast<int>(std::ceil(base_.getFontSize() * .1f));
        const auto half_height = height / 2 - padding * 4; {
            auto t_bound = bound.removeFromRight(height);
            t_bound = t_bound.withSizeKeepingCentre(height - 2 * padding, height - 2 * padding);
            ratio_slider_.setBounds(t_bound);
        } {
            auto t_bound = bound.removeFromRight(height);
            t_bound = t_bound.withSizeKeepingCentre(height - 2 * padding, height - 2 * padding);
            th_slider_.setBounds(t_bound);
        } {
            bound.removeFromRight(padding);
            auto t_bound = bound.removeFromRight(height);
            t_bound.removeFromTop(padding * 2);
            knee_slider_.setBounds(t_bound.removeFromTop(half_height));
            t_bound.removeFromBottom(padding * 2);
            curve_slider_.setBounds(t_bound.removeFromBottom(half_height));
        }
    }

    void LeftControlPanel::repaintCallBack() {
        updater_.updateComponents();
    }
} // zlpanel
