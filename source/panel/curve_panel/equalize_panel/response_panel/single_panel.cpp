// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "single_panel.hpp"

namespace zlpanel {
    SinglePanel::SinglePanel(PluginProcessor &processor, zlgui::UIBase &base,
                             const size_t band_idx,
                             zldsp::filter::Ideal<float, 16> &filter)
        : p_ref_(processor), base_(base), band_idx_(band_idx), filter_(filter) {
        path_.preallocateSpace(kWsFloat.size() * 3 + 12);
        next_path_.preallocateSpace(kWsFloat.size() * 3 + 12);

        const std::string suffix = std::to_string(band_idx_);
        for (auto &ID: kBandIDs) {
            auto para_ID = ID + suffix;
            p_ref_.parameters_.addParameterListener(para_ID, this);
            parameterChanged(para_ID, p_ref_.parameters_.getRawParameterValue(para_ID)->load());
        }

        setInterceptsMouseClicks(false, false);
    }

    SinglePanel::~SinglePanel() {
        const std::string suffix = std::to_string(band_idx_);
        for (auto &ID: kBandIDs) {
            p_ref_.parameters_.removeParameterListener(ID + suffix, this);
        }
    }

    void SinglePanel::paint(juce::Graphics &g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getColorMap1(band_idx_));
        g.strokePath(path_, juce::PathStrokeType(base_.getFontSize() * .15f * curve_thickness_scale,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        g.drawLine(line_, base_.getFontSize() * .075f);
    }

    bool SinglePanel::run(const std::span<float> xs, std::span<float> ys,
                          const juce::Rectangle<float> &bound, const bool force) {
        if (!filter_.getUpdateFlag().exchange(false, std::memory_order::acquire) && !force) {
            return false;
        }
        const auto center_freq = filter_.getFreq();
        const auto center_gain = filter_.getGain();
        const auto filter_type = filter_.getFilterType();
        filter_.updateParas();
        filter_.updateMagnitude(kWsFloat, ys);

        const auto scale = -bound.getHeight() * .5f / 30.f;
        const auto bias = bound.getCentreY();
        auto ys_vector = kfr::make_univector(ys);
        ys_vector = ys_vector * scale + bias;

        next_path_.clear();
        PathMinimizer minimizer(next_path_);
        minimizer.startNewSubPath(xs[0], ys[0]);
        for (size_t i = 1; i < std::min(xs.size(), ys.size()); ++i) {
            minimizer.lineTo(xs[i], ys[i]);
        }
        minimizer.finish();

        const auto center_w = center_freq / 24000.f * static_cast<float>(std::numbers::pi);
        const auto button_curve_x = bound.getWidth() * std::log(center_freq / 10.f) / std::log(2200.f);
        const auto button_curve_y = filter_.getDB(center_w) * scale + bias;

        const auto button_x = button_curve_x;
        float button_y{bias};
        switch (filter_type) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf:
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf: {
                button_y = button_curve_y;
                break;
            }
            case zldsp::filter::FilterType::kTiltShelf: {
                button_y = center_gain * scale * .5f + bias;
                break;
            }
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass:
            case zldsp::filter::FilterType::kNotch: {
                button_y = bias;
                break;
            }
        }
        next_line_ = juce::Line<float>(button_x, button_y, button_curve_x, button_curve_y);
        button_pos_.store({button_x, button_y});

        std::lock_guard<std::mutex> lock{mutex_};
        path_.swapWithPath(next_path_);
        line_ = next_line_;

        return true;
    }

    void SinglePanel::parameterChanged(const juce::String &parameter_ID, const float new_value) {
        if (parameter_ID.startsWith(zlp::PFreq::kID)) {
            filter_.setFreq(new_value);
        } else if (parameter_ID.startsWith(zlp::PGain::kID)) {
            filter_.setGain(new_value);
        } else if (parameter_ID.startsWith(zlp::PQ::kID)) {
            filter_.setQ(new_value);
        } else if (parameter_ID.startsWith(zlp::PFilterType::kID)) {
            filter_.setFilterType(static_cast<zldsp::filter::FilterType>(std::round(new_value)));
        } else if (parameter_ID.startsWith(zlp::POrder::kID)) {
            filter_.setOrder(zlp::POrder::kOrderArray[static_cast<size_t>(std::round(new_value))]);
        }
    }
} // zlpanel
