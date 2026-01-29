// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "meter_background_panel.hpp"
#include "meter_display_panel.hpp"

namespace zlpanel {
    class MeterPanel final : public juce::Component {
    public:
        explicit MeterPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~MeterPanel() override;

        MeterDisplayPanel& getDisplayPanel() {
            return meter_display_panel_;
        }

        void resized() override;

        void repaintCallBackSlow();

    private:
        MeterBackgroundPanel meter_background_panel_;
        MeterDisplayPanel meter_display_panel_;
    };
}
