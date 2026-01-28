// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "meter_panel.hpp"

namespace zlpanel {
    MeterPanel::MeterPanel(PluginProcessor& p, zlgui::UIBase& base) :
        meter_background_panel_(p, base),
        meter_display_panel_(p, base) {

        meter_background_panel_.setBufferedToImage(true);
        addAndMakeVisible(meter_background_panel_);

        addAndMakeVisible(meter_display_panel_);
    }

    MeterPanel::~MeterPanel() = default;

    void MeterPanel::resized() {
        meter_background_panel_.setBounds(getLocalBounds());
        meter_display_panel_.setBounds(getLocalBounds());
    }
}
