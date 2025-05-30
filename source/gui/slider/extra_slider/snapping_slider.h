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

namespace zlgui::slider {
    class SnappingSlider final : public juce::Slider {
    public:
        explicit SnappingSlider(UIBase &base, const juce::String &name = "") : juce::Slider(name), base_(base) {
        }

        void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override {
            juce::MouseWheelDetails w = wheel;
            w.deltaX *= base_.getSensitivity(kMouseWheel);
            w.deltaY *= base_.getSensitivity(kMouseWheel);
            if (e.mods.isShiftDown()) {
                const auto dir = base_.getIsMouseWheelShiftReverse() ? -1.f : 1.f;
                w.deltaX *= base_.getSensitivity(kMouseWheelFine) * dir;
                w.deltaY *= base_.getSensitivity(kMouseWheelFine) * dir;
            }
            Slider::mouseWheelMove(e, w);
        }

    private:
        UIBase &base_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnappingSlider)
    };
}
