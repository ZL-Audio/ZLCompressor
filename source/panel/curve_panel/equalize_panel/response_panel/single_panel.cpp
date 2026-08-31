// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

#include <algorithm>
#include <cmath>

namespace zlpanel {
    SinglePanel::SinglePanel(zlgui::UIBase& base,
                             size_t& selected_band_idx,
                             std::vector<size_t>& not_off_indices) :
        base_(base), selected_band_idx_(selected_band_idx), not_off_indices_(not_off_indices) {
        temp_y_.resize(kNumPoints);
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            for (auto& path : paths_[band].get_buffer()) {
                path.preallocateSpace(static_cast<int>(kNumPoints * 3 + 12));
            }
            for (auto& path : fills_[band].get_buffer()) {
                path.preallocateSpace(static_cast<int>(kNumPoints * 3 + 18));
            }
        }
        setInterceptsMouseClicks(false, false);
    }

    void SinglePanel::paint(juce::Graphics& g) {
        const auto selected_band = selected_band_idx_;
        if (selected_band < zlp::kBandNum) {
            for (const auto band : not_off_indices_) {
                if (band != selected_band) {
                    drawBand<false>(g, band);
                }
            }
            drawBand<true>(g, selected_band);
        } else {
            for (const auto band : not_off_indices_) {
                drawBand<false>(g, band);
            }
        }
    }

    void SinglePanel::resized() {
        const auto height = static_cast<float>(getHeight());
        center_y_.store(height * .5f, std::memory_order::relaxed);
        height_.store(height, std::memory_order::relaxed);
        lookAndFeelChanged();
    }

    void SinglePanel::updateDrawingParameters(
        const size_t band,
        const zlp::EqualizeController::FilterStatus filter_status) {
        if (filter_status == zlp::EqualizeController::kOff) {
            fill_alpha_[band] = 0.f;
            stroke_alpha_[band] = 0.f;
            return;
        }

        float multiplier = 1.f;
        if (filter_status == zlp::EqualizeController::kBypass) {
            multiplier *= kBypassAlphaMultiplier;
        }

        const auto is_selected = band == selected_band_idx_;
        if (is_selected) {
            fill_alpha_[band] = kFillingAlpha * multiplier;
        } else {
            fill_alpha_[band] = 0.f;
            if (selected_band_idx_ >= zlp::kBandNum) {
                multiplier *= kNoBandSelectedAlphaMultiplier;
            }
            multiplier *= kNotSelectedAlphaMultiplier;
        }
        stroke_alpha_[band] = multiplier;
        stroke_colour_[band] = base_.getColourBlendedWithBackground(base_.getColourMap1(band), multiplier);
    }

    void SinglePanel::run(const size_t band,
                          const zlp::EqualizeController::FilterStatus filter_status,
                          const bool to_update,
                          const std::span<const float> xs, const float scale, const float bias,
                          const std::span<const float> magnitudes,
                          const float center_x, const float center_curve_y, const float button_y,
                          const float left_x, const float right_x,
                          const bool is_all_pass, const bool is_first_order) {
        if (!to_update) {
            return;
        }

        auto& next_path = paths_[band].get_writer();
        auto& next_fill = fills_[band].get_writer();
        auto& next_button_line = button_lines_[band].get_writer();
        auto& next_all_pass_line = all_pass_lines_[band].get_writer();

        next_path.clear();
        next_fill.clear();
        next_button_line.setStart(-100.f, -100.f);
        next_button_line.setEnd(-100.f, -100.f);
        next_all_pass_line.setStart(-100.f, -100.f);
        next_all_pass_line.setEnd(-100.f, -100.f);

        if (filter_status != zlp::EqualizeController::kOff && !xs.empty()) {
            if (is_all_pass) {
                next_path.startNewSubPath(center_x, 0.f);
                next_path.lineTo(center_x, height_.load(std::memory_order::relaxed));
                if (!is_first_order) {
                    next_all_pass_line.setStart(left_x, 0.f);
                    next_all_pass_line.setEnd(right_x, 0.f);
                }
            } else if (magnitudes.size() == xs.size()) {
                temp_y_.resize(magnitudes.size());
                for (size_t i = 0; i < magnitudes.size(); ++i) {
                    temp_y_[i] = magnitudes[i] * scale + bias;
                }

                PathMinimizer<1> minimizer(next_path);
                minimizer.drawPath(xs, std::span<const float>(temp_y_));

                next_fill = next_path;
                const auto center_y = center_y_.load(std::memory_order::relaxed);
                next_fill.lineTo(xs.back(), center_y);
                next_fill.lineTo(xs.front(), center_y);
                next_fill.closeSubPath();

                if (std::abs(center_curve_y - button_y) > 1e-6f) {
                    next_button_line.setStart(center_x, center_curve_y);
                    next_button_line.setEnd(center_x, button_y);
                }
            }
        }

        paths_[band].publish();
        fills_[band].publish();
        button_lines_[band].publish();
        all_pass_lines_[band].publish();
    }

    template <bool thick>
    void SinglePanel::drawBand(juce::Graphics& g, const size_t band) {
        if (fill_alpha_[band] > .01f) {
            g.setColour(base_.getColourMap1(band).withAlpha(fill_alpha_[band]));
            fills_[band].pull();
            g.fillPath(fills_[band].get_reader());
        }

        if (stroke_alpha_[band] <= .01f) {
            return;
        }

        const auto thickness = thick ? curve_thickness_ * kThickMultiplier : curve_thickness_;
        g.setColour(stroke_colour_[band]);
        paths_[band].pull();
        g.strokePath(paths_[band].get_reader(), juce::PathStrokeType(thickness,
                                                                     juce::PathStrokeType::curved,
                                                                     juce::PathStrokeType::square));

        button_lines_[band].pull();
        if (const auto line = button_lines_[band].get_reader(); line.getEndX() > 0.f) {
            const auto top = std::min(line.getStartY(), line.getEndY());
            const auto bottom = std::max(line.getStartY(), line.getEndY());
            g.fillRect(line.getStartX() - thickness * .35f, top,
                       thickness * .7f, bottom - top);
        }

        if constexpr (thick) {
            all_pass_lines_[band].pull();
            const auto line = all_pass_lines_[band].get_reader();
            if (line.getEndX() > 0.f) {
                const auto center_y = center_y_.load(std::memory_order::relaxed);
                g.fillRect(line.getStartX() - thickness * .35f, center_y * .5f,
                           thickness * .7f, center_y);
                g.fillRect(line.getEndX() - thickness * .35f, center_y * .5f,
                           thickness * .7f, center_y);
            }
        }
    }

    void SinglePanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .175f * base_.getEQCurveThickness();
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            stroke_colour_[band] = base_.getColourBlendedWithBackground(
                base_.getColourMap1(band), stroke_alpha_[band]);
        }
    }
}
