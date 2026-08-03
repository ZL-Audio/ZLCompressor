// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlpanel {
    SumPanel::SumPanel(zlgui::UIBase& base) :
        base_{base} {
        ys_.resize(400);
        for (auto& path : path_.get_buffer()) {
            path.preallocateSpace(400 * 3 + 12);
        }

        setInterceptsMouseClicks(false, false);
    }

    SumPanel::~SumPanel() = default;

    void SumPanel::paint(juce::Graphics& g) {
        path_.pull();
        g.setColour(base_.getColourMap2(0));
        g.strokePath(path_.get_reader(), juce::PathStrokeType(curve_thickness_,
                                                              juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::rounded));
    }

    void SumPanel::resized() {
        lookAndFeelChanged();
    }

    void SumPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .2f * base_.getEQCurveThickness();
    }

    void SumPanel::run(const std::span<const float> xs,
                       const std::array<zldsp::vector::aligned_vector<float>, zlp::kBandNum>& mags,
                       const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum>& filter_status,
                       const juce::Rectangle<float>& bound, const float max_db) {
        ys_.resize(xs.size());
        std::fill(ys_.begin(), ys_.end(), 0.f);
        bool has_on_filter{false};
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (filter_status[band] == zlp::EqualizeController::FilterStatus::kOn) {
                has_on_filter = true;
                for (size_t i = 0; i < ys_.size(); ++i) {
                    ys_[i] += mags[band][i];
                }
            }
        }

        if (!has_on_filter) {
            std::fill(ys_.begin(), ys_.end(), bound.getCentreY());
        } else {
            const auto scale = -bound.getHeight() * .5f / max_db;
            for (auto& y : ys_) {
                y = y * scale + bound.getCentreY();
            }
        }

        auto& next_path{path_.get_writer()};
        next_path.clear();
        PathMinimizer<1> minimizer(next_path);
        minimizer.startNewSubPath(xs[0], ys_[0]);
        for (size_t i = 1; i < std::min(xs.size(), ys_.size()); ++i) {
            minimizer.lineTo(xs[i], ys_[i]);
        }
        minimizer.finish();
        path_.publish();
    }
}
