// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "computer_panel.hpp"
#include "peak_panel.hpp"
#include "rms_panel.hpp"
#include "separate_panel.hpp"

namespace zlpanel {
    class MagAnalyzerPanel final : public juce::Component {
    public:
        explicit MagAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~MagAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack(double time_stamp);

        void run();

    private:
        zlgui::UIBase &base_;
        PeakPanel peak_panel_;
        RMSPanel rms_panel_;
        ComputerPanel computer_panel_;
        SeparatePanel separate_panel_;
        std::atomic<double> next_stamp_{0.};
        double rms_previous_stamp_{0.};
        std::atomic<bool> to_run_rms_{false};
        int repaint_count_{3};
    };
} // zlpanel
