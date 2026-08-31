// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

#include <algorithm>

namespace zlpanel {
    SumPanel::SumPanel(zlgui::UIBase& base) : base_(base) {
        temp_y_.resize(kNumPoints);
        for (auto& path : path_.get_buffer()) {
            path.preallocateSpace(static_cast<int>(kNumPoints * 3 + 12));
        }
        setInterceptsMouseClicks(false, false);
    }

    void SumPanel::paint(juce::Graphics& g) {
        path_.pull();
        const auto& path = path_.get_reader();
        if (path.isEmpty()) {
            return;
        }
        g.setColour(base_.getColourMap2(0));
        g.strokePath(path, juce::PathStrokeType(curve_thickness_,
                                                juce::PathStrokeType::curved,
                                                juce::PathStrokeType::butt));
    }

    void SumPanel::resized() {
        lookAndFeelChanged();
    }

    void SumPanel::run(
        const bool to_update, const bool has_not_off_filter,
        const std::span<const size_t> on_indices,
        const std::span<const float> xs, const float scale, const float bias,
        const std::array<zldsp::vector::aligned_vector<float>, zlp::kBandNum>& magnitudes) {
        if (!to_update || xs.empty()) {
            return;
        }

        auto& path = path_.get_writer();
        path.clear();

        if (on_indices.empty()) {
            if (has_not_off_filter) {
                path.startNewSubPath(xs.front(), bias);
                path.lineTo(xs.back(), bias);
            }
            path_.publish();
            return;
        }

        temp_y_.resize(xs.size());
        std::fill(temp_y_.begin(), temp_y_.end(), bias);
        for (size_t i = 0; i < temp_y_.size(); ++i) {
            float sum = 0.f;
            for (const auto band : on_indices) {
                sum += magnitudes[band][i];
            }
            temp_y_[i] = sum * scale + bias;
        }

        PathMinimizer<1> minimizer(path);
        minimizer.drawPath(xs, std::span<const float>(temp_y_));
        path_.publish();
    }

    void SumPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .275f * base_.getEQCurveThickness();
    }
}
