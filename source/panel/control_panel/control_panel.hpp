// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "control_background.hpp"
#include "mid_control_panel.hpp"
#include "right_control_panel.hpp"
#include "bottom_control_panel.hpp"

namespace zlpanel {
    class ControlPanel : public juce::Component {
    public:
        explicit ControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                              const multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void repaintCallBackSlow();

        int getIdealWidth() const;

        int getIdealHeight() const;

    private:
        zlgui::UIBase& base_;

        ControlBackground control_background_;
        MidControlPanel mid_control_panel_;
        RightControlPanel right_control_panel_;
        BottomControlPanel bottom_control_panel_;
    };
}
