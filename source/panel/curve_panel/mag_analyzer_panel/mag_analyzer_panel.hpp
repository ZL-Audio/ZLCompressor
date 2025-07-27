// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "mag_background_panel.hpp"
#include "computer_panel.hpp"
#include "peak_panel.hpp"
#include "rms_panel.hpp"
#include "separate_panel.hpp"

namespace zlpanel {
    class MagAnalyzerPanel final : public juce::Component {
    public:
        explicit MagAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~MagAnalyzerPanel() override;

        void resized() override;

        void repaintCallBackSlow();

        void repaintCallBack(double time_stamp);

        void run(const juce::Thread &thread);

        RMSPanel &getRMSPanel() noexcept {
            return rms_panel_;
        }

        ComputerPanel &getComputerPanel() noexcept {
            return computer_panel_;
        }

        SeparatePanel &getSeparatePanel() noexcept {
            return separate_panel_;
        }

    private:
        zlgui::UIBase &base_;
        MagBackgroundPanel background_panel_;
        PeakPanel peak_panel_;
        RMSPanel rms_panel_;
        ComputerPanel computer_panel_;
        SeparatePanel separate_panel_;

        zlgui::attachment::ComponentUpdater updater_;
        zlgui::slider::SnappingSlider threshold_slider_;
        zlgui::attachment::SliderAttachment<true> threshold_attachment_;
        zlgui::slider::SnappingSlider ratio_slider_;
        zlgui::attachment::SliderAttachment<true> ratio_attachment_;

        std::atomic<double> next_stamp_{0.};
        double rms_previous_stamp_{0.};
        std::atomic<bool> to_run_rms_{false};

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    };
} // zlpanel
