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

namespace zlgui::tooltip {
    class TooltipWindow final : public juce::TooltipWindow {
    public:
        explicit TooltipWindow(Component *parentComponent, const int millisecondsBeforeTipAppears = 700)
            : juce::TooltipWindow(parentComponent, millisecondsBeforeTipAppears) {
        }

        juce::String getTipFor(Component &c) override {
            return is_on_ ? juce::TooltipWindow::getTipFor(c) : juce::String();
        }

        void setON(const bool x) { is_on_ = x; }

    private:
        bool is_on_{true};
    };
}
