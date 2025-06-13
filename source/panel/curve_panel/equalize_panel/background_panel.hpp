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

#include "../../../PluginProcessor.hpp"
#include "../../helper/helper.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class BackgroundPanel final : public juce::Component {
    public:
        explicit BackgroundPanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~BackgroundPanel() override;

        void paint(juce::Graphics &g) override;

        void setMouseOver(bool is_mouse_on);

        void repaintCallBack();

    private:
        zlgui::UIBase &base_;
        bool old_is_mouse_on_{false}, is_mouse_on_{false};
    };
} // zlpanel
