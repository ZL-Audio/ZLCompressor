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
#include "../../interface_definitions.hpp"

namespace zlgui::combobox {
    class ClickComboboxButtonLookAndFeel final : public juce::LookAndFeel_V4 {
    public:
        explicit ClickComboboxButtonLookAndFeel(UIBase &base, const juce::String &label)
            : base_(base), label_string_(label) {
        }

        void drawDrawableButton(juce::Graphics &g, juce::DrawableButton &button,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::ignoreUnused(button, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

            g.setColour(base_.getTextColor());
            g.setFont(base_.getFontSize() * font_scale_);
            g.drawText(label_string_, button.getLocalBounds().toFloat(), justification_);
        }

        inline void setFontScale(const float x) { font_scale_ = x; }

        inline void setJustification(const juce::Justification j) { justification_ = j; }

    private:
        UIBase &base_;
        juce::String label_string_;

        float font_scale_{kFontNormal};
        juce::Justification justification_{juce::Justification::centred};
    };
}
