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

        setInterceptsMouseClicks(true, true);
    }

    BackgroundPanel::~BackgroundPanel() {
    }

    void BackgroundPanel::paint(juce::Graphics &g) {
        if (is_mouse_on_) {
            g.fillAll(base_.getBackgroundColor().withAlpha(.875f));
            g.setFont(base_.getFontSize());
            g.setColour(base_.getTextColor().withAlpha(.375f));
            for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
                g.drawText(kBackgroundFreqsNames[i], text_bounds_[i], juce::Justification::bottomRight);
            }
            g.setColour(base_.getTextColor().withAlpha(.0625f));
            g.fillRectList(rect_list_);
        } else {
            const auto bound = getLocalBounds().toFloat();
            const juce::ColourGradient gradient1{
                base_.getBackgroundColor().withAlpha(0.f),
                bound.getX(), bound.getY(),
                base_.getBackgroundColor().withAlpha(.875f),
                bound.getX(), bound.getBottom(), false
            };
            g.setGradientFill(gradient1);
            g.fillRect(getLocalBounds());

        }
    }

    void BackgroundPanel::resized() {
        rect_list_.clear();
        auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.1f;
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            const auto x = kBackgroundFreqs[i] * bound.getWidth() + bound.getX();
            rect_list_.add({x - thickness * .5f, bound.getY(), thickness, bound.getHeight()});
            text_bounds_[i] = juce::Rectangle<float>(x - base_.getFontSize() * 3 - base_.getFontSize() * 0.125f,
                                                   bound.getBottom() - base_.getFontSize() * 2,
                                                   base_.getFontSize() * 3, base_.getFontSize() * 2);
        }

        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * base_.getFontSize());

        for (auto &d: kBackgroundDBs) {
            const auto y = d * bound.getHeight() + bound.getY();
            rect_list_.add({bound.getX(), y - thickness * .5f, bound.getWidth(), thickness});
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
