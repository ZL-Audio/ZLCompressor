// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

namespace zlpanel {
    inline constexpr float kFontSizeOverWidth = 0.016f;
    inline constexpr float kPaddingScale = .5f;
    inline constexpr float kSliderWidthScale = 6.5f;
    inline constexpr float kButtonScale = 2.f;
    inline constexpr float kBoxHeightScale = 1.75f;
    inline constexpr float kSliderHeightScale = 2.8f;
    inline constexpr float kSmallSliderWidthScale = 5.5f;
    inline constexpr float kSliderDraggingDistanceScale = 6.5f;

    inline int getPaddingSize(const float font_size) {
        return static_cast<int>(std::round(font_size * kPaddingScale));
    }

    inline int getSliderWidth(const float font_size) {
        return static_cast<int>(std::round(font_size * kSliderWidthScale));
    }

    inline int getSliderHeight(const float font_size) {
        return static_cast<int>(std::round(font_size * kSliderHeightScale));
    }

    inline int getSliderDraggingDistance(const float font_size) {
        return static_cast<int>(std::round(font_size * kSliderDraggingDistanceScale));
    }

    inline int getButtonSize(const float font_size) {
        return static_cast<int>(std::round(font_size * kButtonScale));
    }

    inline int getBoxHeight(const float font_size) {
        return static_cast<int>(std::round(font_size * kBoxHeightScale));
    }

    inline int getSmallSliderWidth(const float font_size) {
        return static_cast<int>(std::round(font_size * kSmallSliderWidthScale));
    }

    inline int getControlPanelHeight(const float font_size) {
        return static_cast<int>(std::round(font_size * 7.348f)) + getPaddingSize(font_size) + getButtonSize(font_size);
    }
}
