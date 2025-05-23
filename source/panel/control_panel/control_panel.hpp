// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "left_control_panel.hpp"
#include "mid_control_panel.hpp"
#include "right_control_panel.hpp"

namespace zlpanel {
    class ControlPanel : public juce::Component {
    public:
        explicit ControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        double previous_time_stamp{0.};
        LeftControlPanel left_control_panel_;
        // MidControlPanel mid_control_panel_;
        // RightControlPanel right_control_panel_;
    };
} // zlpanel
