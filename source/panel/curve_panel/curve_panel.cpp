// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor& p, zlgui::UIBase& base,
                           multilingual::TooltipHelper& tooltip_helper) :
        Thread("curve_panel"), p_ref_(p), base_(base),
        analyzer_max_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMaxDB::kID)),
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)),
        mag_analyzer_panel_(p, base),
        separate_panel_(base),
        equalize_panel_(p, base),
        left_control_panel_(p, base, tooltip_helper),
        side_control_panel_(p, base, tooltip_helper),
        analyzer_setting_panel_(p, base, tooltip_helper),
        equalize_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideEQDisplay::kID)),
        side_control_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideControlDisplay::kID)),
        computer_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PComputerCurveDisplay::kID)),
        rms_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PRMSAnalyzerDisplay::kID)) {
        addAndMakeVisible(mag_analyzer_panel_);
        addChildComponent(separate_panel_);
        addAndMakeVisible(left_control_panel_);

        addChildComponent(equalize_panel_);
        addChildComponent(side_control_panel_);
        addChildComponent(analyzer_setting_panel_);
        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    void CurvePanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    void CurvePanel::paintOverChildren(juce::Graphics& g) {
        juce::ignoreUnused(g);
        equalize_panel_.repaintCallBackAfter();
        notify();
    }

    void CurvePanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto left_padding = (getWidth() - (padding * 11 + slider_width * 7 + small_slider_width * 2)) / 2;
        {
            auto bound = getLocalBounds();
            bound.removeFromLeft(button_size);
            mag_analyzer_panel_.setBounds(bound);
        }
        {
            auto bound = getLocalBounds();
            left_control_panel_.setBounds(bound.removeFromLeft(button_size));
        }
        {
            auto bound = getLocalBounds();
            bound.removeFromBottom(getControlPanelHeight(font_size) - padding);
            bound.removeFromLeft(button_size);

            const auto width = side_control_panel_.getIdealWidth();
            const auto ideal_height = std::min(side_control_panel_.getIdealHeight(), bound.getHeight());
            const auto height = std::max(static_cast<int>(static_cast<float>(bound.getHeight()) * .618f), ideal_height);

            bound = bound.removeFromBottom(height);
            bound = bound.removeFromLeft(left_padding + (padding + slider_width) * 6 + button_size);

            equalize_large_bound_ = bound;
            side_control_panel_.setBounds(bound.removeFromLeft(width));
            equalize_small_bound_ = bound;

            if (side_control_panel_.isVisible()) {
                equalize_panel_.setBounds(equalize_small_bound_);
            } else {
                equalize_panel_.setBounds(equalize_large_bound_);
            }
            separate_panel_.setBounds(getLocalBounds().withWidth(equalize_large_bound_.getWidth()));
        }
        {
            auto bound = getLocalBounds();
            bound.removeFromLeft(button_size);
            bound = bound.removeFromLeft(analyzer_setting_panel_.getIdealWidth());
            analyzer_setting_panel_.setBounds(bound.removeFromTop(analyzer_setting_panel_.getIdealHeight()));
        }
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            const MagDBRange mag_db_range{
                zlstate::PAnalyzerMaxDB::getDBFromIndex(
                    analyzer_max_db_ref_.load(std::memory_order::relaxed)),
                zlstate::PAnalyzerMinDB::getDBFromIndex(
                    analyzer_min_db_ref_.load(std::memory_order::relaxed))};
            mag_analyzer_panel_.run(*this, mag_db_range);
            if (threadShouldExit()) {
                return;
            }
            if (equalize_show_ref_.load(std::memory_order::relaxed) > .5f) {
                equalize_panel_.run(*this);
            }
        }
    }

    void CurvePanel::repaintCallBackSlow() {
        const auto sample_rate = p_ref_.getAtomicSampleRate();
        if (std::abs(sample_rate - c_sample_rate_) > 1.0) {
            c_sample_rate_ = sample_rate;
            equalize_panel_.updateSampleRate(sample_rate);
        }

        const auto side_control_show = side_control_show_ref_.load(std::memory_order::relaxed) > .5f;
        const auto equalize_show = equalize_show_ref_.load(std::memory_order::relaxed) > .5f;
        const auto computer_show = computer_show_ref_.load(std::memory_order::relaxed) > .5f;
        const auto rms_show = rms_show_ref_.load(std::memory_order::relaxed) > .5f;
        if (side_control_panel_.isVisible() != side_control_show || equalize_panel_.isVisible() != equalize_show) {
            equalize_panel_.setBounds(side_control_show ? equalize_small_bound_ : equalize_large_bound_);
            side_control_panel_.setVisible(side_control_show);
        }
        if (p_ref_.getEqualizeController().getFFTAnalyzerON() != equalize_show) {
            p_ref_.getEqualizeController().setFFTAnalyzerON(equalize_show);
        }
        if (equalize_panel_.isVisible() != equalize_show) {
            equalize_panel_.setVisible(equalize_show);
            separate_panel_.setVisible(equalize_show);
        }
        const auto actual_rms_show = rms_show && !side_control_show && !equalize_show;
        mag_analyzer_panel_.getRMSPanel().setVisible(actual_rms_show);
        mag_analyzer_panel_.getSeparatePanel().setVisible(actual_rms_show || computer_show);
        mag_analyzer_panel_.getComputerPanel().setVisible(computer_show);

        left_control_panel_.repaintCallBackSlow();
        side_control_panel_.repaintCallBackSlow();
        mag_analyzer_panel_.repaintCallBackSlow();
        equalize_panel_.repaintCallBackSlow();
        analyzer_setting_panel_.repaintCallBackSlow();
    }

    void CurvePanel::repaintCallBack(const double time_stamp) {
        mag_analyzer_panel_.repaintCallBack(time_stamp);
        if (equalize_panel_.isVisible()) {
            equalize_panel_.repaintCallBack(time_stamp);
        }
    }
}
