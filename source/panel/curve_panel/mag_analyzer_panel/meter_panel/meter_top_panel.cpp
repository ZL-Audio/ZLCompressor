// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "meter_top_panel.hpp"

namespace zlpanel {
    MeterTopPanel::MeterTopPanel(zlgui::UIBase& base) :
        base_(base) {
        setInterceptsMouseClicks(false, false);
    }

    void MeterTopPanel::paint(juce::Graphics& g) {
        auto bound = getLocalBounds().toFloat();
        g.setColour(base_.getBackgroundColour().withAlpha(.5f));
        g.fillRect(bound);

        const auto meter_width = bound.getWidth() * .45f;
        g.setFont(base_.getFontSize());
        g.setColour(base_.getTextColour());
        g.drawText(formatValue(std::abs(reduction_value_)), bound.removeFromLeft(meter_width),
                   juce::Justification::centred, false);
        g.setColour(out_value_ < 0.f
            ? base_.getTextColour()
            : base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        if (out_value_ < -120.f) {
        } else {
            g.drawText(formatValue(out_value_), bound.removeFromRight(meter_width),
                       juce::Justification::centred, false);
        }
    }

    void MeterTopPanel::updateValue(const float reduction_value, const float out_value) {
        if (std::abs(reduction_value - reduction_value_) > 0.05f
            || std::abs(out_value - out_value_) > 0.05f) {
            reduction_value_ = reduction_value;
            out_value_ = out_value;
            repaint();
        }
    }

    std::string MeterTopPanel::formatValue(const float value) {
        std::stringstream ss;
        if (std::abs(value) < 100.f) {
            ss << std::fixed << std::setprecision(1) << value;
        } else {
            ss << std::fixed << std::setprecision(0) << value;
        }
        return ss.str();
    }
}
