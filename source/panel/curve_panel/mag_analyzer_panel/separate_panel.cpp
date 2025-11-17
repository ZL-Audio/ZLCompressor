// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "separate_panel.hpp"

namespace zlpanel {
    SeparatePanel::SeparatePanel(zlgui::UIBase& base)
        : base_(base) {
        setInterceptsMouseClicks(false, false);

        setBufferedToImage(true);
    }

    void SeparatePanel::paint(juce::Graphics& g) {
        g.setGradientFill(gradient_);
        g.fillRect(getLocalBounds());
    }

    void SeparatePanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        gradient_.point1 = juce::Point<float>(bound.getX(), bound.getY());
        gradient_.point2 = juce::Point<float>(bound.getRight(), bound.getY());
        gradient_.isRadial = false;
        gradient_.clearColours();

        gradient_.addColour(0.0,
                            base_.getBackgroundColour().withAlpha(1.f));
        gradient_.addColour(1.0,
                            base_.getBackgroundColour().withAlpha(0.f));
    }
} // zlpanel
