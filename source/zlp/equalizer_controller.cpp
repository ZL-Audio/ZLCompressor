// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "equalizer_controller.hpp"

namespace zlp {
    EqualizerController::EqualizerController() {
        on_indices_.reserve(kBandNum);
    }

    void EqualizerController::prepare(const juce::dsp::ProcessSpec &spec) {
        for (auto &filter: filters_) {
            filter.prepare(spec.sampleRate, 2);
        }
        gain_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize), 0.01);
    }

    void EqualizerController::prepareBuffer() {
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
        for (const auto &i: on_indices_) {
            filters_[i].prepareBuffer();
        }
        c_fft_analyzer_on_ = fft_analyzer_on_.load(std::memory_order::relaxed);
    }

    void EqualizerController::process(std::array<double *, 2> pointers, const size_t num_samples) {
        prepareBuffer();
        if (!c_gain_equal_zero_) {
            gain_.process(pointers, num_samples);
        }
        for (const auto &i: on_indices_) {
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
            fft_analyzer_.process({pointers}, num_samples);
        }
    }
}
