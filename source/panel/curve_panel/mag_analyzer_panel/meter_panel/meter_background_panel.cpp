// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "meter_background_panel.hpp"

#include "../../../helper/panel_constants.hpp"

namespace zlpanel {
    MeterBackgroundPanel::MeterBackgroundPanel(PluginProcessor&, zlgui::UIBase& base) :
        base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void MeterBackgroundPanel::paint(juce::Graphics& g) {
        auto bound = getLocalBounds().toFloat();
        const auto font_size = base_.getFontSize();
        bound.removeFromTop(static_cast<float>(getTopPanelHeight(font_size)));

        const auto thickness = base_.getFontSize() * 0.125f;
        g.setColour(base_.getTextColour().withAlpha(.1f));
        for (const auto scale : {0.f, 1.f, 2.f, 3.f, 4.f, 5.f}) {
            const auto y = bound.getHeight() * scale / 6.f + bound.getY() - thickness * .5f;
            const auto rect = juce::Rectangle<float>({bound.getX(), y, bound.getWidth(), thickness});
            g.fillRect(rect);
        }
    }
}
