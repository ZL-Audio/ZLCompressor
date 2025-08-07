// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "mag_grid_panel.hpp"

namespace zlpanel {
    MagGridgroundPanel::MagGridgroundPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : base_(base),
          mag_min_db_id_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        setInterceptsMouseClicks(false, false);

        setBufferedToImage(true);
    }

    void MagGridgroundPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColor());

        const auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.125f;
        g.setColour(base_.getTextColor().withAlpha(.1f));
        for (const auto scale : {1.f, 2.f, 3.f, 4.f, 5.f}) {
            const auto y = bound.getHeight() * scale / 6.f;
            const auto rect = juce::Rectangle<float>({bound.getX(), y, bound.getWidth(), thickness});
            g.fillRect(rect);
        }

        const auto text_height = static_cast<float>(juce::roundToInt(base_.getFontSize() * 1.75f));
        const auto text_width = static_cast<float>(juce::roundToInt(base_.getFontSize()  * 2.6f));
        const auto right_padding = static_cast<float>(juce::roundToInt(base_.getFontSize() * kPaddingScale) / 2);
        g.setColour(base_.getTextColor().withAlpha(.5f));
        g.setFont(base_.getFontSize());
        for (const auto scale : {2.f, 4.f}) {
            const auto y = bound.getHeight() * scale / 6.f;
            auto rect = juce::Rectangle<float>({bound.getX(), y - text_height, bound.getWidth(), text_height});
            rect.removeFromRight(right_padding);
            rect = rect.removeFromRight(text_width);
            const auto text = std::to_string(static_cast<int>(std::round(mag_min_db_ * scale / 6.f)));
            g.drawText(text, rect, juce::Justification::bottomRight, false);
        }
    }

    void MagGridgroundPanel::repaintCallBackSlow() {
        const auto c_mag_min_db_id = mag_min_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(c_mag_min_db_id - mag_min_db_id_) > 1e-3f) {
            mag_min_db_id_ = std::round(c_mag_min_db_id);
            mag_min_db_ = zlstate::PAnalyzerMinDB::getMinDBFromIndex(c_mag_min_db_id);
            repaint();
        }
    }
} // zlpanel
