// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlpanel {
    MainPanel::MainPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : base_(base), curve_panel_(processor) {
        addAndMakeVisible(curve_panel_);
        juce::ignoreUnused(base_);
    }

    void MainPanel::resized() {
        curve_panel_.setBounds(getLocalBounds());
    }

    void MainPanel::repaintCallBack(const double time_stamp) {
        curve_panel_.repaintCallBack(time_stamp);
    }
} // zlpanel
