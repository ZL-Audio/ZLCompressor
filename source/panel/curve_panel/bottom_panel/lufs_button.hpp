// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class LUFSButton final : public juce::Component {
    public:
        explicit LUFSButton(PluginProcessor &p, zlgui::UIBase &base,
                            multilingual::TooltipHelper &tooltip_helper);

        ~LUFSButton() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        bool mouse_over_{false};

        const std::unique_ptr<juce::Drawable> learn_drawable_;
        zlgui::button::ClickButton learn_button_;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;
    };
} // zlpanel
