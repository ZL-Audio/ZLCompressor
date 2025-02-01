// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef SEPARATE_PANEL_HPP
#define SEPARATE_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlPanel {
    class SeparatePanel : public juce::Component {
    public:
        explicit SeparatePanel();

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        juce::ColourGradient gradient;
    };
} // zlPanel

#endif //SEPARATE_PANEL_HPP
