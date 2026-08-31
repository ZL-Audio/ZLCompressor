// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../../gui/gui.hpp"

namespace zlpanel {
    class SoloPanel final : public juce::Component {
    public:
        explicit SoloPanel(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void updateX(float left_x, float right_x) {
            left_x_ = left_x;
            right_x_ = right_x;
        }

    private:
        zlgui::UIBase& base_;
        float left_x_{0.f}, right_x_{1e15f};
    };
}
