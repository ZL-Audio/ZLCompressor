// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../chore/thread/notifier.hpp"
#include "../dsp/filter/empty_filter/empty.hpp"
#include "../dsp/filter/filter.hpp"
#include "../dsp/analyzer/analyzer_base/analyzer_sender_base.hpp"
#include "../dsp/gain/gain.hpp"
#include "zlp_definitions.hpp"

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlp {
    class EqualizeController {
    public:
        static constexpr size_t kAnalyzerPointNum = 100;

        enum FilterStatus {
            kOff, kBypass, kOn
        };

        explicit EqualizeController();

        void prepare(double sample_rate, size_t max_num_samples);

        void process(std::array<double*, 2> pointers, size_t num_samples);

        void setFilterStatus(const size_t filter_idx, const FilterStatus filter_status) {
            filter_status_[filter_idx].store(filter_status, std::memory_order::relaxed);
            to_update_filter_status_.signal();
            to_update_.signal();
        }

        void setGain(const float db) {
            gain_db_.store(static_cast<double>(db), std::memory_order::relaxed);
            to_update_gain_.signal();
            to_update_.signal();
        }

        void setFilterFreq(const size_t idx, const double freq) {
            empty_filters_[idx].setFreq(freq);
            empty_update_flags_[idx].signal();
            to_update_.signal();
        }

        void setFilterGain(const size_t idx, const double gain) {
            empty_filters_[idx].setGain(gain);
            empty_update_flags_[idx].signal();
            to_update_.signal();
        }

        void setFilterQ(const size_t idx, const double q) {
            empty_filters_[idx].setQ(q);
            empty_update_flags_[idx].signal();
            to_update_.signal();
        }

        void setFilterType(const size_t idx, const zldsp::filter::FilterType type) {
            empty_filters_[idx].setFilterType(type);
            empty_update_flags_[idx].signal();
            to_update_.signal();
        }

        void setFilterOrder(const size_t idx, const size_t order) {
            empty_filters_[idx].setOrder(order);
            empty_update_flags_[idx].signal();
            to_update_.signal();
        }

        zldsp::filter::FilterType getFilterType(const size_t idx) const {
            return empty_filters_[idx].getFilterType();
        }

        double getFilterQ(const size_t idx) const {
            return empty_filters_[idx].getQ();
        }

        auto& getFFTAnalyzerSender() {
            return fft_analyzer_sender_;
        }

        void setFFTAnalyzerON(const bool f) {
            fft_analyzer_on_.store(f, std::memory_order::relaxed);
            to_update_fft_analyzer_.signal();
            to_update_.signal();
        }

        [[nodiscard]] bool getFFTAnalyzerON() const {
            return fft_analyzer_on_.load(std::memory_order::relaxed);
        }

        void setSoloBand(const size_t solo_band) {
            solo_band_.store(solo_band, std::memory_order::relaxed);
            to_update_solo_.signal();
            to_update_.signal();
        }

        size_t getSoloBand() const {
            return solo_band_.load(std::memory_order::relaxed);
        }

        bool getSoloOn() const {
            return c_solo_on_;
        }

        std::array<double*, 2>& getSoloPointers() {
            return solo_pointers_;
        }

    private:
        zlchore::thread::Notifier to_update_{true};
        zlchore::thread::Notifier to_update_gain_{true};
        std::atomic<double> gain_db_{0.f};
        zldsp::gain::Gain<double> gain_{};
        bool c_gain_equal_zero_{true};

        std::array<zldsp::filter::TDF<double, 16>, kBandNum> filters_{};
        std::array<zldsp::filter::Empty, kBandNum> empty_filters_{};
        std::array<zlchore::thread::Notifier, kBandNum> empty_update_flags_{};
        std::array<zldsp::filter::FilterParameters, kBandNum> filter_paras_{};
        double max_freq_{getEQFreqMax(48000.0)};
        zlchore::thread::Notifier to_update_filter_status_{true};
        std::array<std::atomic<FilterStatus>, kBandNum> filter_status_;
        std::array<FilterStatus, kBandNum> c_filter_status_{};
        std::vector<size_t> on_indices_{};

        std::atomic<bool> fft_analyzer_on_{false};
        bool c_fft_analyzer_on_{false};
        zlchore::thread::Notifier to_update_fft_analyzer_{true};
        zldsp::analyzer::AnalyzerSenderBase<double, 1> fft_analyzer_sender_{};

        zldsp::filter::TDF<double, 16> solo_filter_{};
        std::atomic<size_t> solo_band_{kBandNum};
        zlchore::thread::Notifier to_update_solo_{false};
        size_t c_solo_band_{kBandNum};
        bool c_solo_on_{false};
        std::array<std::vector<double>, 2> solo_buffers_;
        std::array<double*, 2> solo_pointers_{};

        void prepareBuffer();

        void updateSoloFilter(const zldsp::filter::FilterParameters& target, bool force);
    };
}
