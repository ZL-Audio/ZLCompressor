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
#include "mag_analyzer_panel/mag_analyzer_panel.hpp"
#include "equalize_panel/equalize_panel.hpp"
#include "left_control_panel.hpp"
#include "bottom_control_panel.hpp"
#include "side_control_panel.hpp"

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

        void repaintCallBackSlow();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        MagAnalyzerPanel mag_analyzer_panel_;
        EqualizePanel equalize_panel_;
        BottomControlPanel bottom_control_panel_;
        LeftControlPanel left_control_panel_;
        SideControlPanel side_control_panel_;

        juce::Rectangle<int> equalize_large_bound_, equalize_small_bound_;

        double previous_time_stamp{0.0};

        std::atomic<float> &equalize_show_ref_, &side_control_show_ref_;

        void run() override;
    };
} // zlpanel
