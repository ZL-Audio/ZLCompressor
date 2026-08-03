// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "equalize_controller.hpp"

namespace zlp {
    EqualizeController::EqualizeController() {
        on_indices_.reserve(kBandNum);
    }

    void EqualizeController::prepare(const double sample_rate, const size_t max_num_samples) {
        max_freq_ = getEQFreqMax(sample_rate);
        fft_analyzer_sender_.prepare(sample_rate, max_num_samples, {2}, 0.1);
        fft_analyzer_sender_.setON(0, true);
        for (size_t i = 0; i < kBandNum; ++i) {
            filter_paras_[i] = empty_filters_[i].getParas();
            filter_paras_[i].freq = std::min(filter_paras_[i].freq, max_freq_);
            filters_[i].prepare(sample_rate, 2, max_num_samples);
            filters_[i].updateParas(filter_paras_[i]);
        }
        gain_.prepare(sample_rate, max_num_samples, 0.01);
        for (size_t chan = 0; chan < 2; chan++) {
            solo_buffers_[chan].resize(max_num_samples);
            solo_pointers_[chan] = solo_buffers_[chan].data();
        }
        solo_filter_.prepare(sample_rate, 2, max_num_samples);
    }

    void EqualizeController::prepareBuffer() {
        if (!to_update_.check()) {
            return;
        }
        if (to_update_gain_.check()) {
            const auto c_gain_db = gain_db_.load(std::memory_order::relaxed);
            gain_.setGainDecibels(c_gain_db);
            c_gain_equal_zero_ = std::abs(c_gain_db) < 1e-3;
        }
        if (to_update_filter_status_.check()) {
            // cache new filter status
            for (size_t i = 0; i < kBandNum; ++i) {
                const auto new_filter_status = filter_status_[i].load(std::memory_order::relaxed);
                if (new_filter_status != c_filter_status_[i]) {
                    if (c_filter_status_[i] == FilterStatus::kOff) {
                        filters_[i].reset();
                    }
                    c_filter_status_[i] = new_filter_status;
                }
            }
            // cache new on indices
            on_indices_.clear();
            for (size_t i = 0; i < kBandNum; ++i) {
                if (c_filter_status_[i] != kOff) {
                    on_indices_.emplace_back(i);
                }
            }
        }
        if (to_update_fft_analyzer_.check()) {
            c_fft_analyzer_on_ = fft_analyzer_on_.load(std::memory_order::relaxed);
        }
        if (to_update_solo_.check()) {
            c_solo_band_ = solo_band_.load(std::memory_order::relaxed);
            c_solo_on_ = c_solo_band_ < kBandNum;
            if (c_solo_on_) {
                solo_filter_.reset();
                updateSoloFilter(filter_paras_[c_solo_band_], true);
            }
        }
        for (const auto& i : on_indices_) {
            if (empty_update_flags_[i].check()) {
                filter_paras_[i] = empty_filters_[i].getParas();
                filter_paras_[i].freq = std::min(filter_paras_[i].freq, max_freq_);
                filters_[i].updateParas(filter_paras_[i]);
                if (i == c_solo_band_ && c_solo_on_) {
                    updateSoloFilter(filter_paras_[i], false);
                }
            }
        }
    }

    void EqualizeController::process(std::array<double*, 2> pointers, const size_t num_samples) {
        prepareBuffer();
        if (!c_gain_equal_zero_) {
            gain_.process(pointers, num_samples);
        }
        if (c_solo_on_) {
            zldsp::vector::copy(solo_pointers_[0], pointers[0], num_samples);
            zldsp::vector::copy(solo_pointers_[1], pointers[1], num_samples);
            solo_filter_.template process<false>(solo_pointers_, num_samples);
        }
        for (const auto& i : on_indices_) {
            switch (c_filter_status_[i]) {
            case kOff: {
                break;
            }
            case kBypass: {
                filters_[i].template process<true>(pointers, num_samples);
                break;
            }
            case kOn: {
                filters_[i].template process<false>(pointers, num_samples);
                break;
            }
            }
        }
        if (c_fft_analyzer_on_) {
            fft_analyzer_sender_.process({pointers}, num_samples);
        }
    }

    void EqualizeController::updateSoloFilter(const zldsp::filter::FilterParameters& target, const bool force) {
        auto solo_paras = target;
        switch (solo_paras.filter_type) {
        case zldsp::filter::FilterType::kLowShelf:
        case zldsp::filter::FilterType::kHighPass: {
            solo_paras.filter_type = zldsp::filter::FilterType::kLowPass;
            break;
        }
        case zldsp::filter::FilterType::kHighShelf:
        case zldsp::filter::FilterType::kLowPass: {
            solo_paras.filter_type = zldsp::filter::FilterType::kHighPass;
            break;
        }
        case zldsp::filter::FilterType::kPeak:
        case zldsp::filter::FilterType::kNotch:
        case zldsp::filter::FilterType::kBandPass:
        case zldsp::filter::FilterType::kAllPass: {
            solo_paras.filter_type = zldsp::filter::FilterType::kBandPass;
            break;
        }
        case zldsp::filter::FilterType::kTiltShelf:
        case zldsp::filter::FilterType::kFlatTilt: {
            solo_paras.filter_type = zldsp::filter::FilterType::kTiltShelf;
            break;
        }
        }
        if (solo_paras.filter_type == zldsp::filter::FilterType::kTiltShelf) {
            solo_paras.q = std::sqrt(2.0) * 0.5;
        }
        if (force) {
            solo_filter_.forceUpdate(solo_paras);
        } else {
            solo_filter_.updateParas(solo_paras);
        }
    }
}
