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
        fft_analyzer_sender_.prepare(sample_rate, {2});
        fft_analyzer_sender_.setON(0, true);
        for (auto& filter : filters_) {
            filter.prepare(sample_rate, 2);
        }
        gain_.prepare(sample_rate, max_num_samples, 0.01);
        for (size_t chan = 0; chan < 2; chan++) {
            solo_buffers_[chan].resize(max_num_samples);
            solo_pointers_[chan] = solo_buffers_[chan].data();
        }
        solo_filter_.prepare(sample_rate, 2);
    }

    void EqualizeController::prepareBuffer() {
        if (to_update_gain_.exchange(false, std::memory_order::acquire)) {
            const auto c_gain_db = gain_db_.load(std::memory_order::relaxed);
            gain_.setGainDecibels(c_gain_db);
            c_gain_equal_zero_ = std::abs(c_gain_db) < 1e-3;
        }
        if (to_update_filter_status_.exchange(false, std::memory_order::acquire)) {
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
        if (solo_band_.load(std::memory_order::relaxed) != c_solo_band_) {
            c_solo_band_ = solo_band_.load(std::memory_order::relaxed);
            c_solo_on_ = c_solo_band_ < kBandNum;
            if (c_solo_on_) {
                solo_filter_.reset();
                updateSoloFilter();
            }
        }
        for (const auto& i : on_indices_) {
            if (filters_[i].prepareBuffer() && i == c_solo_band_) {
                updateSoloFilter();
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
        if (fft_analyzer_on_.load(std::memory_order::relaxed)) {
            fft_analyzer_sender_.process({pointers}, num_samples);
        }
    }

    void EqualizeController::updateSoloFilter() {
        auto& target_filter{filters_[c_solo_band_]};
        switch (target_filter.getFilterType<false>()) {
        case zldsp::filter::FilterType::kLowShelf:
        case zldsp::filter::FilterType::kHighPass: {
            solo_filter_.setFilterType(zldsp::filter::FilterType::kLowPass);
            break;
        }
        case zldsp::filter::FilterType::kHighShelf:
        case zldsp::filter::FilterType::kLowPass: {
            solo_filter_.setFilterType(zldsp::filter::FilterType::kHighPass);
            break;
        }
        case zldsp::filter::FilterType::kPeak:
        case zldsp::filter::FilterType::kBandShelf:
        case zldsp::filter::FilterType::kNotch:
        case zldsp::filter::FilterType::kBandPass: {
            solo_filter_.setFilterType(zldsp::filter::FilterType::kBandPass);
            break;
        }
        case zldsp::filter::FilterType::kTiltShelf: {
            solo_filter_.setFilterType(zldsp::filter::FilterType::kTiltShelf);
            break;
        }
        }
        solo_filter_.setOrder(target_filter.getOrder<false>());
        solo_filter_.setFreq(target_filter.getFreq<false>());
        solo_filter_.setQ(target_filter.getQ<false>());
        if (target_filter.getFilterType<false>() == zldsp::filter::FilterType::kTiltShelf) {
            solo_filter_.setQ(std::sqrt(2.0) * 0.5);
            solo_filter_.setGain(target_filter.getGain<false>());
        }
        else {
            solo_filter_.setQ(target_filter.getQ<false>());
        }
        solo_filter_.prepareBuffer();
    }
}
