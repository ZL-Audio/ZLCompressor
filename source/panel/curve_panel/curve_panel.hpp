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

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "rms_panel.hpp"
#include "peak_panel.hpp"
#include "computer_panel.hpp"
#include "separate_panel.hpp"
#include "bottom_control_panel.hpp"

namespace zlpanel {
    class CurvePanel final : public juce::Component,
                             private juce::Thread {
    public:
        explicit CurvePanel(PluginProcessor &p, zlgui::UIBase &base);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void paintOverChildren(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        zlgui::UIBase &base_;
        PeakPanel peak_panel_;
        RMSPanel rms_panel_;
        ComputerPanel computer_panel_;
        SeparatePanel separate_panel_;
        BottomControlPanel bottom_control_panel_;
        std::atomic<double> next_stamp_{0.};
        double rms_previous_stamp_{0.};
        std::atomic<bool> to_run_rms_{false};
        int repaint_count_{3};

        void run() override;
    };
} // zlpanel
