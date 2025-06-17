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
    EqualizePanel::EqualizePanel(PluginProcessor &processor, zlgui::UIBase &base)
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

    void EqualizePanel::run(juce::Thread &thread) {
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
        if (thread.threadShouldExit()) {
            return;
        }
    }

    void EqualizePanel::resized() {
        const auto bound = getLocalBounds();
        background_panel_.setBounds(bound);
        fft_analyzer_panel_.setBounds(bound);
        response_panel_.setBounds(bound);
        button_panel_.setBounds(bound);
    }

    void EqualizePanel::repaintCallBackSlow() {
        if (to_update_visibility_.exchange(false, std::memory_order::acquire)) {
            std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> c_filter_status{};
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                c_filter_status[band] = filter_status_[band].load(std::memory_order::relaxed);
            }
            response_panel_.setBandStatus(c_filter_status);
            button_panel_.setBandStatus(c_filter_status);
        }
        button_panel_.repaintCallBackSlow();
        const auto mouse_over = isMouseOverOrDragging(true);
        if (mouse_over != mouse_over_) {
            mouse_over_ = mouse_over;
            response_panel_.setMouseOver(mouse_over);
            background_panel_.setMouseOver(mouse_over);
            button_panel_.setVisible(mouse_over);
        }
    }

    void EqualizePanel::repaintCallBack(double) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto button_pos = response_panel_.getBandButtonPos(band);
            button_panel_.getDragger(band).updateButton(button_pos);
        }
        repaint();
    }

    void EqualizePanel::mouseEnter(const juce::MouseEvent &) {
        background_panel_.setMouseOver(true);
    }

    void EqualizePanel::visibilityChanged() {
        p_ref_.getEqualizeController().setFFTAnalyzerON(isVisible());
    }

    void EqualizePanel::parameterChanged(const juce::String &parameter_ID, float new_value) {
        const auto band = static_cast<size_t>(parameter_ID.getTrailingIntValue());
        filter_status_[band].store(static_cast<zlp::EqualizeController::FilterStatus>(
                                       std::round(new_value)), std::memory_order::relaxed);
        to_update_filter_status_.store(true, std::memory_order::release);
    }
} // zlpanel
