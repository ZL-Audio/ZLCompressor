// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlpanel {
    MainPanel::MainPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : p_ref_(processor), base_(base),
          tooltip_helper_(
              static_cast<multilingual::TooltipLanguage>(std::round(
                  p_ref_.state_.getRawParameterValue(zlstate::PTooltipLang::kID)->load(std::memory_order::relaxed)))
          ),
          curve_panel_(processor, base_, tooltip_helper_),
          control_panel_(processor, base_, tooltip_helper_),
          top_panel_(processor, base_, tooltip_helper_),
          ui_setting_panel_(processor, base_),
          tooltipLAF(base_), tooltipWindow(&curve_panel_),
          refresh_handler_(zlstate::PTargetRefreshSpeed::kRates[base_.getRefreshRateID()]){
        juce::ignoreUnused(base_);
        addAndMakeVisible(curve_panel_);
        addAndMakeVisible(control_panel_);
        addAndMakeVisible(top_panel_);
        addChildComponent(ui_setting_panel_);

        tooltipWindow.setLookAndFeel(&tooltipLAF);
        tooltipWindow.setOpaque(false);
        tooltipWindow.setBufferedToImage(true);

        base_.getPanelValueTree().addListener(this);

        startTimerHz(10);
    }

    MainPanel::~MainPanel() {
        base_.getPanelValueTree().removeListener(this);
        stopTimer();
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds();

        // set actual width/height
        {
            const auto height = static_cast<float>(bound.getHeight());
            const auto width = static_cast<float>(bound.getWidth());
            if (height < width * 0.47f) {
                bound.setHeight(juce::roundToInt(width * .47f));
            } else if (height > width * 1.f) {
                bound.setWidth(juce::roundToInt(height * 1.f));
            }
        }

        const auto font_size = static_cast<float>(bound.getWidth()) * 0.016f;
        base_.setFontSize(font_size);

        ui_setting_panel_.setBounds(bound);

        const auto controlBound = bound.removeFromBottom(juce::roundToInt(font_size * 7.348f));
        control_panel_.setBounds(controlBound);

        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        top_panel_.setBounds(bound.removeFromTop(button_height));

        curve_panel_.setBounds(bound);
    }

    void MainPanel::repaintCallBack(const double time_stamp) {
        if (refresh_handler_.tick(time_stamp)) {
            if (time_stamp - previous_time_stamp_ > 0.1) {
                previous_time_stamp_ = time_stamp;
                control_panel_.repaintCallBackSlow();
                top_panel_.repaintCallBackSlow();
                curve_panel_.repaintCallBackSlow();
            }

            curve_panel_.repaintCallBack(time_stamp);

            const auto c_refresh_rate = refresh_handler_.getActualRefreshRate();
            if (std::abs(c_refresh_rate - refresh_rate_) > 1e-3) {
                refresh_rate_ = c_refresh_rate;
                p_ref_.getEqualizeController().getFFTAnalyzer().setRefreshRate(
                    static_cast<float>(refresh_rate_));
            }
        }
    }

    void MainPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kUISettingPanel, property)) {
            const auto ui_setting_visibility = static_cast<bool>(base_.getPanelProperty(
                zlgui::PanelSettingIdx::kUISettingPanel));
            curve_panel_.setVisible(!ui_setting_visibility);
            ui_setting_panel_.setVisible(ui_setting_visibility);
        }
    }

    void MainPanel::timerCallback() {
        if (juce::Process::isForegroundProcess()) {
            if (getCurrentlyFocusedComponent() != this) {
                grabKeyboardFocus();
            }
            stopTimer();
        }
    }
} // zlpanel
