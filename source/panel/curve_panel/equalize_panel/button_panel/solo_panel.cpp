// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

namespace zlpanel {
    SoloPanel::SoloPanel(zlgui::UIBase& base) : base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void SoloPanel::paint(juce::Graphics& g) {
        const auto bound = getLocalBounds().toFloat();
        g.setColour(base_.getBackgroundColour().withAlpha(.625f));
        if (left_x_ > 0.f) {
            g.fillRect(juce::Rectangle<float>{0.f, 0.f, left_x_, bound.getHeight()});
        }
        if (right_x_ < bound.getWidth()) {
            g.fillRect(juce::Rectangle<float>{right_x_, 0.f,
                                              bound.getWidth() - right_x_, bound.getHeight()});
        }
    }
}
