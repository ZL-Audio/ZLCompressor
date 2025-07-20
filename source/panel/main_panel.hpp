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

#include "../PluginProcessor.hpp"
#include "../gui/gui.hpp"
#include "../state/state_definitions.hpp"
#include "helper/refresh_handler.hpp"
#include "curve_panel/curve_panel.hpp"
#include "control_panel/control_panel.hpp"
#include "top_panel/top_panel.hpp"
#include "ui_setting_panel/ui_setting_panel.hpp"

namespace zlpanel {
    class MainPanel final : public juce::Component,
                            private juce::ValueTree::Listener {
    public:
        explicit MainPanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~MainPanel() override;

        void resized() override;

        void repaintCallBack(const double time_stamp);

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        CurvePanel curve_panel_;
        ControlPanel control_panel_;
        TopPanel top_panel_;
        UISettingPanel ui_setting_panel_;

        RefreshHandler refresh_handler_;
        double previous_time_stamp_{-1.0};
        double refresh_rate_{-1.0};

        void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) override;
    };
} // zlpanel
