// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlpanel {
    MainPanel::MainPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : base_(base),
          curve_panel_(processor, base_),
          control_panel_(processor, base_) {
        juce::ignoreUnused(base_);
        addAndMakeVisible(curve_panel_);
        addAndMakeVisible(control_panel_);
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds(); {
            const auto height = static_cast<float>(bound.getHeight());
            const auto width = static_cast<float>(bound.getWidth());
            if (height < width * 0.47f) {
                bound.setHeight(juce::roundToInt(width * .47f));
            } else if (height > width * 1.f) {
                bound.setWidth(juce::roundToInt(height * 1.f));
            }
        }

        const auto font_size = static_cast<float>(bound.getWidth()) * 0.0163f;
        base_.setFontSize(font_size);

        const auto controlBound = bound.removeFromBottom(juce::roundToInt(font_size * 7.348f));
        control_panel_.setBounds(controlBound);

        curve_panel_.setBounds(bound);
    }

    void MainPanel::repaintCallBack(const double time_stamp) {
        curve_panel_.repaintCallBack(time_stamp);
        control_panel_.repaintCallBack(time_stamp);
    }
} // zlpanel
