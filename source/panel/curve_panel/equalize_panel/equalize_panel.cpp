// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "equalize_panel.hpp"

namespace zlpanel {
    EqualizePanel::EqualizePanel(PluginProcessor& processor, zlgui::UIBase& base)
        : p_ref_(processor), base_{base},
          background_panel_(processor, base),
          fft_analyzer_panel_(processor, base),
          response_panel_(processor, base),
          button_panel_(processor, base, selected_band_idx_) {
        juce::ignoreUnused(base_);

        addAndMakeVisible(background_panel_);
        addAndMakeVisible(fft_analyzer_panel_);
        addAndMakeVisible(response_panel_);
        addChildComponent(button_panel_);

        setInterceptsMouseClicks(true, true);

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            auto para_ID = zlp::PFilterStatus::kID + std::to_string(band);
            p_ref_.parameters_.addParameterListener(para_ID, this);
            parameterChanged(para_ID, p_ref_.parameters_.getRawParameterValue(para_ID)->load());
        }
    }

    EqualizePanel::~EqualizePanel() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            auto para_ID = zlp::PFilterStatus::kID + std::to_string(band);
            p_ref_.parameters_.removeParameterListener(para_ID, this);
        }
        p_ref_.getEqualizeController().setFFTAnalyzerON(false);
    }

    void EqualizePanel::run(juce::Thread& thread) {
        juce::ignoreUnused(thread);
        std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> c_filter_status{};
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            c_filter_status[band] = filter_status_[band].load(std::memory_order::relaxed);
        }
        bool to_update_sum{false};
        if (to_update_filter_status_.exchange(false, std::memory_order::acquire)) {
            to_update_sum = true;
            to_update_visibility_.store(true, std::memory_order::release);
        }
        fft_analyzer_panel_.run();
        if (thread.threadShouldExit()) {
            return;
        }
        response_panel_.run(c_filter_status, to_update_sum);
    }

    void EqualizePanel::resized() {
        const auto bound = getLocalBounds();
        background_panel_.setBounds(bound);
        fft_analyzer_panel_.setBounds(bound);
        response_panel_.setBounds(bound);
        button_panel_.setBounds(bound);

        const auto popup_height = static_cast<float>(button_panel_.getPopupPanel().getIdealHeight());
        const auto popup_width = static_cast<float>(button_panel_.getPopupPanel().getIdealWidth());
        popup_top_center_ = {popup_width * .5f, 0.f};
        popup_bottom_center_ = {popup_width * .5f, popup_height};
        previous_popup_target_pos_ = {-100.f, -100.f};
        shift_x_max = static_cast<float>(bound.getWidth()) - popup_width;
    }

    void EqualizePanel::repaintCallBackSlow() {
        button_panel_.repaintCallBackSlow();

        const auto mouse_over = isMouseOverOrDragging(true);
        response_panel_.setMouseOver(mouse_over);
        background_panel_.setMouseOver(mouse_over);
        button_panel_.setVisible(mouse_over);
    }

    void EqualizePanel::repaintCallBack(double) {
        if (previous_band_idx_ != selected_band_idx_) {
            previous_band_idx_ = selected_band_idx_;
            button_panel_.updateBand();
            button_panel_.getPopupPanel().setVisible(false);
            response_panel_.updateBand(selected_band_idx_);
            popup_update_wait_count_ = 2;
        }
        if (to_update_visibility_.exchange(false, std::memory_order::acquire)) {
            std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> c_filter_status{};
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                c_filter_status[band] = filter_status_[band].load(std::memory_order::relaxed);
            }
            response_panel_.setBandStatus(c_filter_status);
            button_panel_.setBandStatus(c_filter_status);
        }

        if (button_panel_.isVisible()) {
            juce::Point<float> popup_target_pos{0.f, -1e6f};
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto button_pos = response_panel_.getBandButtonPos(band);
                button_panel_.getDragger(band).updateButton(button_pos);
                if (band == selected_band_idx_) {
                    popup_target_pos = button_pos;
                }
            }

            if (popup_update_wait_count_ > 0) {
                popup_update_wait_count_ -= 1;
                if (popup_update_wait_count_ == 0) {
                    button_panel_.getPopupPanel().setVisible(selected_band_idx_ != zlp::kBandNum);
                }
            }
            if (popup_update_wait_count_ <= 0) {
                if (std::abs(previous_popup_target_pos_.x - popup_target_pos.x) > 1e-3f ||
                    std::abs(previous_popup_target_pos_.y - popup_target_pos.y) > 1e-3f) {
                    previous_popup_target_pos_ = popup_target_pos;

                    const auto bound = getLocalBounds().toFloat();
                    const auto pos_y_scale = popup_target_pos.y / bound.getHeight();
                    const auto popup_direction_down = pos_y_scale < .25f || (pos_y_scale > .5f && pos_y_scale < .75f);

                    const auto shift_y = popup_direction_down
                                             ? popup_target_pos.y - popup_top_center_.y + base_.getFontSize()
                                             : popup_target_pos.y - popup_bottom_center_.y - base_.getFontSize();

                    const float shift_x = std::clamp(popup_target_pos.x - popup_top_center_.x, 0.f, shift_x_max);

                    button_panel_.getPopupPanel().setTransform(juce::AffineTransform::translation(shift_x, shift_y));
                }
            }
        }

        repaint();
    }

    void EqualizePanel::repaintCallBackAfter() {
        button_panel_.repaintCallBackAfter();
    }

    void EqualizePanel::mouseEnter(const juce::MouseEvent&) {
        background_panel_.setMouseOver(true);
    }

    void EqualizePanel::visibilityChanged() {
        p_ref_.getEqualizeController().setFFTAnalyzerON(isVisible());
        if (!isVisible()) {
            button_panel_.turnOffSolo();
        }
    }

    void EqualizePanel::parameterChanged(const juce::String& parameter_ID, float new_value) {
        const auto band = static_cast<size_t>(parameter_ID.getTrailingIntValue());
        filter_status_[band].store(static_cast<zlp::EqualizeController::FilterStatus>(
                                       std::round(new_value)), std::memory_order::relaxed);
        to_update_filter_status_.store(true, std::memory_order::release);
    }
} // zlpanel
