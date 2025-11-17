// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "equalize_background_panel.hpp"
#include "fft_analyzer_panel.hpp"
#include "response_panel/response_panel.hpp"
#include "button_panel/button_panel.hpp"

namespace zlpanel {
    class EqualizePanel final : public juce::Component,
                                private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit EqualizePanel(PluginProcessor& processor, zlgui::UIBase& base);

        ~EqualizePanel() override;

        void run(juce::Thread& thread);

        void resized() override;

        void repaintCallBackSlow();

        void repaintCallBack(double time_stamp);

        void repaintCallBackAfter();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        size_t selected_band_idx_{zlp::kBandNum}, previous_band_idx_{zlp::kBandNum};

        EqualizerBackgroundPanel background_panel_;
        FFTAnalyzerPanel fft_analyzer_panel_;
        ResponsePanel response_panel_;
        ButtonPanel button_panel_;
        juce::Point<float> popup_top_center_{}, popup_bottom_center_{}, previous_popup_target_pos_{};
        float shift_x_max{};

        std::array<std::atomic<zlp::EqualizeController::FilterStatus>, zlp::kBandNum> filter_status_{};
        std::atomic<bool> to_update_filter_status_{false};
        std::atomic<bool> to_update_visibility_{false};

        int popup_update_wait_count_{0};

        void mouseEnter(const juce::MouseEvent& event) override;

        void visibilityChanged() override;

        void parameterChanged(const juce::String& parameter_ID, float new_value) override;
    };
} // zlpanel
