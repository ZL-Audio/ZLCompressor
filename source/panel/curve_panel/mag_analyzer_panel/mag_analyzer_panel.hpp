// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "mag_background_panel/mag_background_panel.hpp"
#include "computer_panel.hpp"
#include "peak_panel.hpp"
#include "rms_panel.hpp"
#include "separate_panel.hpp"
#include "meter_panel.hpp"

namespace zlpanel {
    class MagAnalyzerPanel final : public juce::Component {
    public:
        explicit MagAnalyzerPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~MagAnalyzerPanel() override;

        void resized() override;

        void repaintCallBackSlow();

        void repaintCallBack(double time_stamp);

        void run(const juce::Thread& thread);

        RMSPanel& getRMSPanel() noexcept {
            return rms_panel_;
        }

        ComputerPanel& getComputerPanel() noexcept {
            return computer_panel_;
        }

        SeparatePanel& getSeparatePanel() noexcept {
            return separate_panel_;
        }

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& meter_display_ref_;

        zldsp::analyzer::FIFOTransferBuffer<3> transfer_buffer_{};

        size_t peak_consumer_id_{0}, meter_consumer_id_{1};

        MagBackgroundPanel background_panel_;
        PeakPanel peak_panel_;
        RMSPanel rms_panel_;
        ComputerPanel computer_panel_;
        SeparatePanel separate_panel_;
        MeterPanel meter_panel_;

        zlgui::attachment::ComponentUpdater updater_;
        zlgui::slider::SnappingSlider threshold_slider_;
        zlgui::attachment::SliderAttachment<true> threshold_attachment_;
        zlgui::slider::SnappingSlider ratio_slider_;
        zlgui::attachment::SliderAttachment<true> ratio_attachment_;

        std::atomic<double> next_stamp_{0.};
        double rms_previous_stamp_{0.};

        double sample_rate_{0.0};
        size_t max_sum_samples_{0};

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        void updateBounds();
    };
}
