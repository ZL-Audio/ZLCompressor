// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {

class LogoPanel : public juce::Component {
public:
    explicit LogoPanel(PluginProcessor &p, zlgui::UIBase &base);

    void paint(juce::Graphics &g) override;

    void mouseEnter(const juce::MouseEvent &event) override;

    void mouseExit(const juce::MouseEvent &event) override;

private:
    zlgui::UIBase &base_;
    std::unique_ptr<juce::Drawable> brand_drawable_, logo_drawable_;
    juce::Justification justification_{juce::Justification::topLeft};
};

}
