// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "clipper_panel.hpp"
#include "mag_grid_panel.hpp"

namespace zlpanel {
    class MagBackgroundPanel final : public juce::Component {
    public:
        explicit MagBackgroundPanel(PluginProcessor& processor, zlgui::UIBase& base);

        void repaintCallBackSlow();

        void resized() override;

    private:
        MagGridgroundPanel mag_gridground_panel_;
        ClipperPanel clipper_panel_;
    };
} // zlpanel
