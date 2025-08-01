// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../dsp/filter/filter.hpp"
#include "../dsp/fft_analyzer/fft_analyzer.hpp"
#include "../dsp/gain/origin_gain.hpp"
#include "zlp_definitions.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlp {
    class EqualizeController {
    public:
        static constexpr size_t kAnalyzerPointNum = 100;
        enum FilterStatus {
            kOff, kBypass, kOn
        };

        explicit EqualizeController();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(std::array<double *, 2> pointers, size_t num_samples);

        void setFilterStatus(const size_t filter_idx, const FilterStatus filter_status) {
            filter_status_[filter_idx].store(filter_status, std::memory_order::relaxed);
            to_update_filter_status_.store(true, std::memory_order::release);
        }

        void setGain(const float db) {
            gain_db_.store(static_cast<double>(db), std::memory_order::relaxed);
            to_update_gain_.store(true, std::memory_order::release);
        }

        zldsp::filter::IIR<double, 16> &getFilter(const size_t idx) {
            return filters_[idx];
        }

        zldsp::analyzer::MultipleFFTAnalyzer<double, 1, 100>& getFFTAnalyzer() {
            return fft_analyzer_;
        }

        void setFFTAnalyzerON(const bool f) {
            fft_analyzer_on_.store(f, std::memory_order::relaxed);
            fft_analyzer_.setON({f});
        }

        [[nodiscard]] bool getFFTAnalyzerON() const {
            return fft_analyzer_on_.load(std::memory_order::relaxed);
        }

    private:
        std::atomic<bool> to_update_gain_{true};
        std::atomic<double> gain_db_{0.f};
        zldsp::gain::Gain<double> gain_{};
        bool c_gain_equal_zero_{true};

        std::array<zldsp::filter::IIR<double, 16>, kBandNum> filters_{};
        std::atomic<bool> to_update_filter_status_{true};
        std::array<std::atomic<FilterStatus>, kBandNum> filter_status_;
        std::array<FilterStatus, kBandNum> c_filter_status_{};
        std::vector<size_t> on_indices_{};

        std::atomic<bool> fft_analyzer_on_{false};
        bool c_fft_analyzer_on_{false};
        zldsp::analyzer::MultipleFFTAnalyzer<double, 1, kAnalyzerPointNum> fft_analyzer_;

        void prepareBuffer();
    };
}
