// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

namespace zlpanel {
    SinglePanel::SinglePanel(zlgui::UIBase& base,
                             const size_t band_idx,
                             zldsp::filter::Ideal<float, 16>& filter) :
        base_(base), band_idx_(band_idx), filter_(filter) {
        button_pos_.store({0.f, -1e6f});
        for (auto& path : path_.get_buffer()) {
            path.preallocateSpace(400 * 3 + 12);
        }

        setInterceptsMouseClicks(false, false);
    }

    void SinglePanel::paint(juce::Graphics& g) {
        path_.pull();
        g.setColour(base_.getColourMap1(band_idx_));
        g.strokePath(path_.get_reader(), juce::PathStrokeType(curve_thickness_ * curve_thickness_scale,
                                                              juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::rounded));
        line_.pull();
        g.drawLine(line_.get_reader(), line_thickness_);
    }

    void SinglePanel::resized() {
        lookAndFeelChanged();
    }

    void SinglePanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .15f * base_.getEQCurveThickness();
        line_thickness_ = base_.getFontSize() * .075f * base_.getEQCurveThickness();
    }

    void SinglePanel::run(const std::span<const float> xs, const std::span<const float> dbs,
                          const juce::Rectangle<float>& bound, const float max_db,
                          const double sample_rate, const double fft_max) {
        const auto center_freq = filter_.getFreq();
        const auto center_gain = filter_.getGain();
        const auto filter_type = filter_.getFilterType();
        const auto scale = -bound.getHeight() * .5f / max_db;
        const auto bias = bound.getCentreY();
        ys_.resize(dbs.size());
        for (size_t i = 0; i < dbs.size(); ++i) {
            ys_[i] = dbs[i] * scale + bias;
        }

        auto& next_path{path_.get_writer()};
        next_path.clear();
        PathMinimizer<1> minimizer(next_path);
        minimizer.startNewSubPath(xs[0], ys_[0]);
        for (size_t i = 1; i < std::min(xs.size(), ys_.size()); ++i) {
            minimizer.lineTo(xs[i], ys_[i]);
        }
        minimizer.finish();

        const auto center_w = static_cast<float>(center_freq * 2.0 * std::numbers::pi / sample_rate);
        const auto button_curve_x = static_cast<float>(
            bound.getX() + bound.getWidth() * std::log(center_freq / zlp::kEQMinFreq)
            / std::log(fft_max / zlp::kEQMinFreq));
        const auto button_db = zldsp::chore::squareGainToDecibels(
            filter_.getCenterMagnitudeSquare(center_w));
        const auto button_curve_y = button_db * scale + bias;

        const auto button_x = button_curve_x;
        float button_y{bias};
        switch (filter_type) {
        case zldsp::filter::FilterType::kPeak:
        case zldsp::filter::FilterType::kLowShelf:
        case zldsp::filter::FilterType::kHighShelf:
        case zldsp::filter::FilterType::kFlatTilt: {
            button_y = button_curve_y;
            break;
        }
        case zldsp::filter::FilterType::kTiltShelf: {
            button_y = static_cast<float>(center_gain * static_cast<double>(scale) * .5 + bias);
            break;
        }
        case zldsp::filter::FilterType::kLowPass:
        case zldsp::filter::FilterType::kHighPass:
        case zldsp::filter::FilterType::kBandPass:
        case zldsp::filter::FilterType::kNotch:
        case zldsp::filter::FilterType::kAllPass: {
            button_y = bias;
            break;
        }
        }
        auto& next_line{line_.get_writer()};
        next_line = juce::Line<float>(button_x, button_y, button_curve_x, button_curve_y);
        button_pos_.store({static_cast<float>(button_x), button_y});

        path_.publish();
        line_.publish();
    }

    void SinglePanel::visibilityChanged() {
        if (!isVisible()) {
            const auto bound = getLocalBounds();
            button_pos_.store({0.f, -static_cast<float>(bound.getHeight() * 2)});
        }
    }
}
