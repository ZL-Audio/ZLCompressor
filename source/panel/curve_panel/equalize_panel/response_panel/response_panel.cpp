// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "response_panel.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace zlpanel {
    ResponsePanel::ResponsePanel(PluginProcessor& processor, zlgui::UIBase& base,
                                 size_t& selected_band_idx) :
        Thread("equalizer response"),
        p_ref_(processor), base_(base),
        single_panel_(base, selected_band_idx, message_not_off_indices_),
        sum_panel_(base),
        eq_max_db_idx_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PEQMaxDB::kID)) {
        xs_.resize(kNumPoints);
        ws_.resize(kNumPoints);
        on_indices_.reserve(zlp::kBandNum);

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            magnitudes_[band].resize(kNumPoints);
            points_[band][4].store(-10000.f, std::memory_order::relaxed);

            const auto suffix = std::to_string(band);
            for (const auto& id : kBandIDs) {
                const auto parameter_id = id + suffix;
                p_ref_.parameters_.addParameterListener(parameter_id, this);
                parameterChanged(parameter_id,
                                 p_ref_.parameters_.getRawParameterValue(parameter_id)->load(
                                     std::memory_order::relaxed));
            }
        }

        addAndMakeVisible(single_panel_);
        addAndMakeVisible(sum_panel_);
        setInterceptsMouseClicks(false, false);
    }

    ResponsePanel::~ResponsePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto suffix = std::to_string(band);
            for (const auto& id : kBandIDs) {
                p_ref_.parameters_.removeParameterListener(id + suffix, this);
            }
        }
    }

    void ResponsePanel::resized() {
        const auto bound = getLocalBounds();
        single_panel_.setBounds(bound);
        sum_panel_.setBounds(bound);

        width_.store(static_cast<float>(bound.getWidth()), std::memory_order::relaxed);
        height_.store(static_cast<float>(bound.getHeight()), std::memory_order::relaxed);
        font_size_.store(base_.getFontSize(), std::memory_order::relaxed);
        to_update_bound_.signal();
    }

    void ResponsePanel::repaintCallBack() {
        if (!message_to_update_panels_.check()) {
            return;
        }

        message_not_off_indices_.clear();
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto status = filter_status_[band].load(std::memory_order::relaxed);
            single_panel_.updateDrawingParameters(band, status);
            if (status != zlp::EqualizeController::kOff) {
                message_not_off_indices_.emplace_back(band);
            }
        }
    }

    void ResponsePanel::updateSampleRate(const double sample_rate) {
        sample_rate_.store(sample_rate, std::memory_order::relaxed);
    }

    void ResponsePanel::updateBand() {
        message_to_update_panels_.signal();
    }

    void ResponsePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            if (threadShouldExit()) {
                break;
            }
            if (!updateCurveParameters()) {
                continue;
            }
            if (!updateCurveMagnitudes()) {
                break;
            }

            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto to_update = to_update_curve_flags_[band];
                const auto parameters = ideal_[band].getParas();
                single_panel_.run(
                    band, c_filter_status_[band], to_update,
                    std::span<const float>(xs_), c_scale_, c_bias_,
                    std::span<const float>(magnitudes_[band]),
                    points_[band][0].load(std::memory_order::relaxed),
                    points_[band][3].load(std::memory_order::relaxed),
                    points_[band][4].load(std::memory_order::relaxed),
                    points_[band][1].load(std::memory_order::relaxed),
                    points_[band][2].load(std::memory_order::relaxed),
                    parameters.filter_type == zldsp::filter::kAllPass,
                    parameters.order == 1);
                to_update_curve_flags_[band] = false;
                if (threadShouldExit()) {
                    break;
                }
            }
            if (threadShouldExit()) {
                break;
            }

            sum_panel_.run(to_update_sum_, has_not_off_filter_,
                           std::span<const size_t>(on_indices_),
                           std::span<const float>(xs_), c_scale_, c_bias_, magnitudes_);
            to_update_sum_ = false;
        }
    }

    void ResponsePanel::parameterChanged(const juce::String& parameter_ID, const float value) {
        const auto band = static_cast<size_t>(parameter_ID.getTrailingIntValue());
        if (band >= zlp::kBandNum) {
            return;
        }

        if (parameter_ID.startsWith(zlp::PFilterStatus::kID)) {
            filter_status_[band].store(
                static_cast<zlp::EqualizeController::FilterStatus>(std::round(value)),
                std::memory_order::relaxed);
            to_update_filter_status_.signal();
            message_to_update_panels_.signal();
        } else if (parameter_ID.startsWith(zlp::PFilterType::kID)) {
            empty_[band].setFilterType(static_cast<zldsp::filter::FilterType>(std::round(value)));
            to_update_empty_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::POrder::kID)) {
            const auto order_idx = static_cast<size_t>(std::clamp(
                static_cast<int>(std::round(value)), 0,
                static_cast<int>(zlp::POrder::kOrderArray.size() - 1)));
            empty_[band].setOrder(zlp::POrder::kOrderArray[order_idx]);
            to_update_empty_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::PFreq::kID)) {
            empty_[band].setFreq(value);
            to_update_empty_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::PGain::kID)) {
            empty_[band].setGain(value);
            to_update_empty_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::PQ::kID)) {
            empty_[band].setQ(value);
            to_update_empty_flags_[band].signal();
        }
    }

    bool ResponsePanel::updateCurveParameters() {
        if (to_update_filter_status_.check()) {
            bool status_changed = false;
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto status = filter_status_[band].load(std::memory_order::relaxed);
                if (status != c_filter_status_[band]) {
                    c_filter_status_[band] = status;
                    to_update_curve_flags_[band] = true;
                    status_changed = true;
                }
            }
            if (status_changed) {
                on_indices_.clear();
                has_not_off_filter_ = false;
                for (size_t band = 0; band < zlp::kBandNum; ++band) {
                    if (c_filter_status_[band] != zlp::EqualizeController::kOff) {
                        has_not_off_filter_ = true;
                    }
                    if (c_filter_status_[band] == zlp::EqualizeController::kOn) {
                        on_indices_.emplace_back(band);
                    }
                }
                to_update_sum_ = true;
                message_to_update_panels_.signal();
            }
        }

        if (const auto sample_rate = sample_rate_.load(std::memory_order::relaxed);
            std::abs(sample_rate - c_sample_rate_) > 1.0) {
            c_sample_rate_ = sample_rate;
            c_slider_max_ = zlp::getEQFreqMax(sample_rate);
            fft_max_ = zlp::getEQFFTMax(sample_rate);
            for (auto& filter : ideal_) {
                filter.prepare(sample_rate);
            }

            const auto log_range = std::log(fft_max_ / static_cast<double>(zlp::kEQMinFreq));
            const auto frequency_scale = 2.0 * std::numbers::pi / sample_rate;
            for (size_t i = 0; i < kNumPoints; ++i) {
                const auto portion = static_cast<double>(i) / static_cast<double>(kNumPoints - 1);
                const auto frequency = static_cast<double>(zlp::kEQMinFreq) * std::exp(portion * log_range);
                ws_[i] = static_cast<float>(frequency * frequency_scale);
            }
            std::fill(to_update_curve_flags_.begin(), to_update_curve_flags_.end(), true);
            to_update_sum_ = true;
        }

        if (to_update_bound_.check()) {
            c_width_ = width_.load(std::memory_order::relaxed);
            c_height_ = height_.load(std::memory_order::relaxed);
            c_font_size_ = font_size_.load(std::memory_order::relaxed);
            if (c_width_ > 0.f) {
                const auto interval = c_width_ / static_cast<float>(kNumPoints - 1);
                for (size_t i = 0; i < kNumPoints; ++i) {
                    xs_[i] = static_cast<float>(i) * interval;
                }
            }
            c_eq_max_db_idx_ = -1.f;
            std::fill(to_update_curve_flags_.begin(), to_update_curve_flags_.end(), true);
            to_update_sum_ = true;
        }

        if (const auto eq_max_db_idx = eq_max_db_idx_ref_.load(std::memory_order::relaxed);
            std::abs(eq_max_db_idx - c_eq_max_db_idx_) > .1f) {
            c_eq_max_db_idx_ = eq_max_db_idx;
            const auto idx = static_cast<size_t>(std::clamp(
                static_cast<int>(std::round(eq_max_db_idx)), 0,
                static_cast<int>(zlstate::PEQMaxDB::kDBs.size() - 1)));
            const auto max_db = zlstate::PEQMaxDB::kDBs[idx];
            const auto plot_height = std::max(c_height_ - 2.f * c_font_size_, 0.f);
            c_scale_ = -plot_height * .5f / max_db;
            c_bias_ = c_height_ * .5f;
            std::fill(to_update_curve_flags_.begin(), to_update_curve_flags_.end(), true);
            to_update_sum_ = true;
        }

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (to_update_empty_flags_[band].check()) {
                to_update_curve_flags_[band] = true;
                if (c_filter_status_[band] == zlp::EqualizeController::kOn) {
                    to_update_sum_ = true;
                }
            }
        }

        return c_sample_rate_ > 0.0 && c_width_ > 0.f && c_height_ > 0.f;
    }

    bool ResponsePanel::updateCurveMagnitudes() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (!to_update_curve_flags_[band]) {
                continue;
            }
            if (c_filter_status_[band] == zlp::EqualizeController::kOff) {
                points_[band][4].store(-10000.f, std::memory_order::relaxed);
                continue;
            }

            auto parameters = empty_[band].getParas();
            parameters.freq = std::clamp(parameters.freq,
                                         static_cast<double>(zlp::kEQMinFreq), c_slider_max_);
            ideal_[band].forceUpdate(parameters);
            ideal_[band].updateMagnitudeSquare(ws_, magnitudes_[band]);
            zldsp::vector::sqr_mag_to_db(magnitudes_[band].data(), magnitudes_[band].size());

            const auto center_w = static_cast<float>(
                parameters.freq * 2.0 * std::numbers::pi / c_sample_rate_);
            const auto center_magnitude = zldsp::chore::squareGainToDecibels(
                ideal_[band].getCenterMagnitudeSquare(center_w));
            const auto [left_x, center_x, right_x] = getLeftCenterRightX(parameters);

            points_[band][0].store(center_x, std::memory_order::relaxed);
            points_[band][1].store(left_x, std::memory_order::relaxed);
            points_[band][2].store(right_x, std::memory_order::relaxed);
            points_[band][3].store(c_scale_ * center_magnitude + c_bias_, std::memory_order::relaxed);
            points_[band][4].store(c_scale_ * getButtonMagnitude(parameters) + c_bias_,
                                   std::memory_order::relaxed);

            if (threadShouldExit()) {
                return false;
            }
        }
        return true;
    }

    float ResponsePanel::getButtonMagnitude(const zldsp::filter::FilterParameters& parameters) {
        switch (parameters.filter_type) {
        case zldsp::filter::kPeak:
            return static_cast<float>(parameters.gain);
        case zldsp::filter::kLowShelf:
        case zldsp::filter::kHighShelf:
        case zldsp::filter::kTiltShelf:
        case zldsp::filter::kFlatTilt:
            return static_cast<float>(parameters.gain * .5);
        case zldsp::filter::kLowPass:
        case zldsp::filter::kHighPass:
        case zldsp::filter::kNotch:
        case zldsp::filter::kBandPass:
        case zldsp::filter::kAllPass:
            return 0.f;
        }
        return 0.f;
    }

    std::tuple<float, float, float> ResponsePanel::getLeftCenterRightX(
        zldsp::filter::FilterParameters parameters) const {
        const auto frequency_to_x = static_cast<double>(c_width_)
            / std::log(fft_max_ / static_cast<double>(zlp::kEQMinFreq));
        const auto center_x = std::log(parameters.freq / static_cast<double>(zlp::kEQMinFreq)) * frequency_to_x;

        switch (parameters.filter_type) {
        case zldsp::filter::kPeak:
        case zldsp::filter::kBandPass:
        case zldsp::filter::kNotch: {
            const auto bandwidth = parameters.freq / parameters.q;
            const auto left_frequency = .5 * bandwidth
                * (std::sqrt(4.0 * parameters.q * parameters.q + 1.0) - 1.0);
            const auto right_frequency = left_frequency + bandwidth;
            return {
                static_cast<float>(std::log(left_frequency / zlp::kEQMinFreq) * frequency_to_x),
                static_cast<float>(center_x),
                static_cast<float>(std::log(right_frequency / zlp::kEQMinFreq) * frequency_to_x)
            };
        }
        case zldsp::filter::kAllPass: {
            if (parameters.order == 1) {
                parameters.q = std::sqrt(2.0) * .5;
            }
            const auto bandwidth = parameters.freq / parameters.q;
            const auto left_frequency = .5 * bandwidth
                * (std::sqrt(4.0 * parameters.q * parameters.q + 1.0) - 1.0);
            const auto right_frequency = left_frequency + bandwidth;
            return {
                static_cast<float>(std::log(left_frequency / zlp::kEQMinFreq) * frequency_to_x),
                static_cast<float>(center_x),
                static_cast<float>(std::log(right_frequency / zlp::kEQMinFreq) * frequency_to_x)
            };
        }
        case zldsp::filter::kTiltShelf:
        case zldsp::filter::kFlatTilt:
            return {0.f, static_cast<float>(center_x), c_width_};
        case zldsp::filter::kLowShelf:
        case zldsp::filter::kHighPass:
            return {0.f, static_cast<float>(center_x), static_cast<float>(center_x)};
        case zldsp::filter::kHighShelf:
        case zldsp::filter::kLowPass:
            return {static_cast<float>(center_x), static_cast<float>(center_x), c_width_};
        }
        return {static_cast<float>(center_x), static_cast<float>(center_x), static_cast<float>(center_x)};
    }
}
