// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "mag_grid_panel.hpp"
#include "../../mag_db_range.hpp"

namespace zlpanel {
    MagGridPanel::MagGridPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        mag_max_db_id_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMaxDB::kID)),
        mag_min_db_id_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        setInterceptsMouseClicks(false, false);

        setBufferedToImage(true);
    }

    void MagGridPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());

        const auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.125f;
        g.setColour(base_.getTextColour().withAlpha(.1f));
        for (const auto scale : {1.f, 2.f, 3.f, 4.f, 5.f}) {
            const auto y = bound.getHeight() * scale / 6.f;
            const auto rect = juce::Rectangle<float>({bound.getX(), y, bound.getWidth(), thickness});
            g.fillRect(rect);
        }

        const auto text_height = static_cast<float>(juce::roundToInt(base_.getFontSize() * 1.75f));
        const auto text_width = static_cast<float>(juce::roundToInt(base_.getFontSize() * 2.6f));
        const auto right_padding = static_cast<float>(juce::roundToInt(base_.getFontSize() * kPaddingScale) / 2);
        g.setColour(base_.getTextColour().withAlpha(.5f));
        g.setFont(base_.getFontSize());
        const MagDBRange db_range{mag_max_db_, mag_range_db_};
        for (const auto scale : {1.f, 2.f, 3.f, 4.f, 5.f, 6.f}) {
            const auto y = bound.getHeight() * scale / 6.f;
            auto rect = juce::Rectangle<float>({bound.getX(), y - text_height, bound.getWidth(), text_height});
            rect.removeFromRight(right_padding);
            rect = rect.removeFromRight(text_width);
            const auto value = db_range.getDBAtYProportion(scale / 6.f);
            const auto rounded_value = std::round(value);
            if (std::abs(value - rounded_value) < .01f) {
                g.drawText(std::to_string(static_cast<int>(rounded_value)), rect,
                           juce::Justification::bottomRight, false);
            }
        }
    }

    void MagGridPanel::repaintCallBackSlow() {
        const auto c_mag_max_db_id = mag_max_db_id_ref_.load(std::memory_order::relaxed);
        const auto c_mag_min_db_id = mag_min_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(c_mag_max_db_id - mag_max_db_id_) > 1e-3f ||
            std::abs(c_mag_min_db_id - mag_min_db_id_) > 1e-3f) {
            mag_max_db_id_ = std::round(c_mag_max_db_id);
            mag_min_db_id_ = std::round(c_mag_min_db_id);
            mag_max_db_ = zlstate::PAnalyzerMaxDB::getDBFromIndex(c_mag_max_db_id);
            mag_range_db_ = zlstate::PAnalyzerMinDB::getDBFromIndex(c_mag_min_db_id);
            repaint();
        }
    }
}
