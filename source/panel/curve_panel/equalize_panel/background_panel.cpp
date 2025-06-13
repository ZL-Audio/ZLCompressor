// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "background_panel.hpp"

namespace zlpanel {
    BackgroundPanel::BackgroundPanel(PluginProcessor &, zlgui::UIBase &base)
        : base_(base) {
        setBufferedToImage(true);
    }

    BackgroundPanel::~BackgroundPanel() {
    }


    void BackgroundPanel::paint(juce::Graphics &g) {
        if (is_mouse_on_) {
            g.fillAll(base_.getBackgroundColor().withAlpha(.75f));
        } else {
            const auto bound = getLocalBounds().toFloat();
            const juce::ColourGradient gradient1{
                base_.getBackgroundColor().withAlpha(0.f),
                bound.getX() + bound.getWidth() * .5f, bound.getY(),
                base_.getBackgroundColor().withAlpha(.75f),
                bound.getX() + bound.getWidth() * .5f, bound.getBottom(), false
            };
            g.setGradientFill(gradient1);
            g.fillRect(getLocalBounds());
        }
    }

    void BackgroundPanel::setMouseOver(const bool is_mouse_on) {
        is_mouse_on_ = is_mouse_on;
    }

    void BackgroundPanel::repaintCallBack() {
        if (old_is_mouse_on_ != is_mouse_on_) {
            old_is_mouse_on_ = is_mouse_on_;
            repaint();
        }
    }
} // zlpanel
