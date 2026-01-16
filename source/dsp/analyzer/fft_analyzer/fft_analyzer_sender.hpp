// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../analyzer_base/analyzer_sender_base.hpp"

namespace zldsp::analyzer {
    /**
     * an analyzer sender which pushes input samples into FIFOs
     * @tparam FloatType the float type of input audio buffers
     * @tparam kNum the number of analyzers
     */
    template <typename FloatType, size_t kNum>
    class FFTAnalyzerSender : public AnalyzerSenderBase<FloatType, kNum> {
    public:
        explicit FFTAnalyzerSender(const size_t default_fft_order = 12) :
            AnalyzerSenderBase<FloatType, kNum>() {
            default_fft_order_ = default_fft_order;
        }

        void prepare(double sample_rate, std::array<size_t, kNum> num_channels) {
            this->lock_.lock();
            this->sample_rate_ = sample_rate;
            num_channels_ = num_channels;

            if (sample_rate <= 50000) {
                fft_order_ = default_fft_order_;
            } else if (sample_rate <= 100000) {
                fft_order_ = default_fft_order_ + 1;
            } else if (sample_rate <= 200000) {
                fft_order_ = default_fft_order_ + 2;
            } else {
                fft_order_ = default_fft_order_ + 3;
            }
            this->setFIFOSize(1 << fft_order_, num_channels);

            this->lock_.unlock();
        }

        size_t getFFTOrder() const {
            return fft_order_;
        }

        std::array<size_t, kNum>& getNumChannels() {
            return num_channels_;
        }

    protected:
        size_t default_fft_order_{12};
        size_t fft_order_{12};
        std::array<size_t, kNum> num_channels_;
    };
}
