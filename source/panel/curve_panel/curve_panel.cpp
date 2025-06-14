// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor &p, zlgui::UIBase &base)
        : Thread("curve_panel"), p_ref_(p), base_(base),
          mag_analyzer_panel_(p, base),
          equalize_panel_(p, base),
          bottom_control_panel_(p, base),
          left_control_panel_(p, base),
          side_control_panel_(p, base),
          equalize_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideEQDisplay::kID)),
          side_control_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideControlDisplay::kID)) {
        addAndMakeVisible(mag_analyzer_panel_);
        addAndMakeVisible(bottom_control_panel_);
        addAndMakeVisible(left_control_panel_);

        addChildComponent(equalize_panel_);
        addChildComponent(side_control_panel_);
        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    void CurvePanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        notify();
    }

    void CurvePanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale); {
            auto bound = getLocalBounds();
            bound.removeFromLeft(button_height);
            mag_analyzer_panel_.setBounds(bound);
        } {
            auto bound = getLocalBounds();
            bound = bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            bottom_control_panel_.setBounds(bound);
        } {
            auto bound = getLocalBounds();
            bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            left_control_panel_.setBounds(bound.removeFromLeft(button_height));
        } {
            auto bound = getLocalBounds();
            bound.removeFromBottom(juce::roundToInt(base_.getFontSize() * 1.75f));
            bound.removeFromLeft(button_height);

            const auto width = side_control_panel_.getIdealWidth();
            const auto height = side_control_panel_.getIdealHeight();

            bound = bound.removeFromBottom(height);
            bound = bound.removeFromLeft((padding + slider_width) * 6 - button_height);

            equalize_large_bound_ = bound;
            side_control_panel_.setBounds(bound.removeFromLeft(width));
            equalize_small_bound_ = bound;

            if (side_control_panel_.isVisible()) {
                equalize_panel_.setBounds(equalize_small_bound_);
            } else {
                equalize_panel_.setBounds(equalize_large_bound_);
            }
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            mag_analyzer_panel_.run(*this);
            equalize_panel_.run(*this);
        }
    }

    void CurvePanel::repaintCallBack(const double time_stamp) {
        if (time_stamp - previous_time_stamp > 0.1) {
            const auto side_control_show = side_control_show_ref_.load(std::memory_order::relaxed) > .5f;
            const auto equalize_show = equalize_show_ref_.load(std::memory_order::relaxed) > .5f;
            if (side_control_panel_.isVisible() != side_control_show || equalize_panel_.isVisible() != equalize_show) {
                equalize_panel_.setBounds(side_control_show ? equalize_small_bound_ : equalize_large_bound_);
                side_control_panel_.setVisible(side_control_show);
            }
            if (p_ref_.getEqualizeController().getFFTAnalyzerON() != equalize_show) {
                p_ref_.getEqualizeController().setFFTAnalyzerON(equalize_show);
            }
            if (equalize_panel_.isVisible() != equalize_show) {
                equalize_panel_.setVisible(equalize_show);
                mag_analyzer_panel_.setComputerPanelVisible(!equalize_show);
            }
            previous_time_stamp = time_stamp;
            mag_analyzer_panel_.setRMSPanelVisible(!side_control_show && !equalize_show);
        }

        mag_analyzer_panel_.repaintCallBack(time_stamp);
        equalize_panel_.repaintCallBack(time_stamp);
        bottom_control_panel_.repaintCallBack(time_stamp);
        left_control_panel_.repaintCallBack(time_stamp);
        side_control_panel_.repaintCallBack(time_stamp);
    }
} // zlpanel
