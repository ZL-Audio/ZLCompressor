// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlpanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : pRef(p),
          ui_base_(base), name_laf_(base),
          refresh_rate_box_(zlstate::PTargetRefreshSpeed::kChoices, base),
          fft_tilt_slider_("Tilt", base),
          fft_speed_slider_("Speed", base),
          mag_curve_slider_("Mag", base),
          eq_curve_slider_("EQ", base) {
        juce::ignoreUnused(pRef);
        name_laf_.setFontScale(zlgui::kFontHuge);

        refresh_rate_label_.setText("Refresh Rate", juce::dontSendNotification);
        refresh_rate_label_.setJustificationType(juce::Justification::centredRight);
        refresh_rate_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(refresh_rate_label_);
        addAndMakeVisible(refresh_rate_box_);

        fft_label_.setText("FFT", juce::dontSendNotification);
        fft_label_.setJustificationType(juce::Justification::centredRight);
        fft_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(fft_label_);
        addAndMakeVisible(fft_tilt_slider_);
        addAndMakeVisible(fft_speed_slider_);

        curve_thick_label_.setText("Curve Thickness", juce::dontSendNotification);
        curve_thick_label_.setJustificationType(juce::Justification::centredRight);
        curve_thick_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(curve_thick_label_);
        addAndMakeVisible(mag_curve_slider_);
        addAndMakeVisible(eq_curve_slider_);
    }

    void OtherUISettingPanel::loadSetting() {
        refresh_rate_box_.getBox().setSelectedId(static_cast<int>(ui_base_.getRefreshRateID()) + 1);
        fft_tilt_slider_.getSlider().setValue(static_cast<double>(ui_base_.getFFTExtraTilt()));
        fft_speed_slider_.getSlider().setValue(static_cast<double>(ui_base_.getFFTExtraSpeed()));
        mag_curve_slider_.getSlider().setValue(ui_base_.getMagCurveThickness());
        eq_curve_slider_.getSlider().setValue(ui_base_.getEQCurveThickness());
    }

    void OtherUISettingPanel::saveSetting() {
        ui_base_.setRefreshRateID(static_cast<size_t>(refresh_rate_box_.getBox().getSelectedId() - 1));
        ui_base_.setFFTExtraTilt(static_cast<float>(fft_tilt_slider_.getSlider().getValue()));
        ui_base_.setFFTExtraSpeed(static_cast<float>(fft_speed_slider_.getSlider().getValue()));
        ui_base_.setMagCurveThickness(static_cast<float>(mag_curve_slider_.getSlider().getValue()));
        ui_base_.setEQCurveThickness(static_cast<float>(eq_curve_slider_.getSlider().getValue()));
        ui_base_.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    void OtherUISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat(); {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            refresh_rate_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            refresh_rate_box_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            fft_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            fft_tilt_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
            local_bound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            fft_speed_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        } {
            bound.removeFromTop(ui_base_.getFontSize());
            auto local_bound = bound.removeFromTop(ui_base_.getFontSize() * 3);
            curve_thick_label_.setBounds(local_bound.removeFromLeft(bound.getWidth() * .3f).toNearestInt());
            local_bound.removeFromLeft(bound.getWidth() * .05f);
            const auto s_width = (bound.getWidth() * .5f - ui_base_.getFontSize() * 2.f) * 0.3f;
            mag_curve_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
            local_bound.removeFromLeft(ui_base_.getFontSize() * 2.f);
            eq_curve_slider_.setBounds(local_bound.removeFromLeft(s_width).toNearestInt());
        }
    }
} // zlpanel
