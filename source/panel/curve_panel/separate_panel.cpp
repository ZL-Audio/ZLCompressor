// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "separate_panel.hpp"

namespace zlPanel {
    SeparatePanel::SeparatePanel() {
        setBufferedToImage(true);
    }

    void SeparatePanel::paint(juce::Graphics &g) {
        g.setGradientFill(gradient);
        g.fillRect(getLocalBounds());
        g.fillAll(juce::Colours::black.withAlpha(.25f));
    }

    void SeparatePanel::resized() {
        auto bound = getLocalBounds().toFloat();
        gradient.point1 = juce::Point<float>(bound.getX(), 0.f);
        gradient.point2 = juce::Point<float>(bound.getRight(), 0.f);
        gradient.isRadial = false;
        gradient.clearColours();

        gradient.addColour(0.0,
                           juce::Colours::black.withAlpha(1.f));
        gradient.addColour(1.0,
                           juce::Colours::black.withAlpha(0.f));
    }
} // zlPanel