// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "mag_background_panel.hpp"

namespace zlpanel {
    MagBackgroundPanel::MagBackgroundPanel(PluginProcessor& processor, zlgui::UIBase& base)
        : mag_gridground_panel_(processor, base),
          clipper_panel_(processor, base) {
        addAndMakeVisible(mag_gridground_panel_);
        addChildComponent(clipper_panel_);

        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    void MagBackgroundPanel::resized() {
        mag_gridground_panel_.setBounds(getLocalBounds());
        clipper_panel_.setBounds(getLocalBounds());
    }

    void MagBackgroundPanel::repaintCallBackSlow() {
        mag_gridground_panel_.repaintCallBackSlow();
        clipper_panel_.repaintCallBackSlow();
    }
} // zlpanel
