// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "control_background.hpp"

namespace zlpanel {
    ControlBackground::ControlBackground(zlgui::UIBase& base, const float alpha,
                                         const std::array<bool, 4> hide_shadow) :
        base_(base), alpha_(alpha), hide_shadow_(hide_shadow) {
        setInterceptsMouseClicks(false, false);
        setAlpha(.9f);
    }

    void ControlBackground::paint(juce::Graphics& g) {
        auto original_bound = getLocalBounds();
        if (hide_shadow_[0] || hide_shadow_[2]) {
            const auto width = original_bound.getWidth();
            original_bound.setWidth(original_bound.getWidth() * 2);
            if (hide_shadow_[0] && hide_shadow_[2]) {
                original_bound.setX(-width / 2);
            } else if (hide_shadow_[0]) {
                original_bound.setX(-width);
            }
        }
        if (hide_shadow_[1] || hide_shadow_[3]) {
            const auto height = original_bound.getHeight();
            original_bound.setHeight(original_bound.getHeight() * 2);
            if (hide_shadow_[1] && hide_shadow_[3]) {
                original_bound.setY(-height / 2);
            } else if (hide_shadow_[1]) {
                original_bound.setY(-height);
            }
        }
        const auto padding = getPaddingSize(base_.getFontSize());
        const auto bound = original_bound.reduced(padding);
        juce::Path path;
        path.addRoundedRectangle(bound.toFloat(), static_cast<float>(padding));

        const juce::DropShadow shadow{base_.getTextColour().withAlpha(alpha_), padding, {0, 0}};
        shadow.drawForPath(g, path);
        g.setColour(base_.getBackgroundColour());
        g.fillPath(path);
    }
}
