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
    class MagAnalyzerSender : public AnalyzerSenderBase<FloatType, kNum> {
    public:
        void prepare(const double sample_rate, size_t max_num_samples, std::array<size_t, kNum> num_channels,
                     const double fifo_size_second) {
            this->lock_.lock();
            sample_rate_ = sample_rate;
            this->max_num_samples_ = max_num_samples;

            this->setFIFOSize(std::max(max_num_samples,
                                       static_cast<size_t>(std::round(sample_rate * fifo_size_second))),
                              num_channels);

            this->lock_.unlock();
        }

        [[nodiscard]] double getSampleRate() const {
            return sample_rate_;
        }

        [[nodiscard]] size_t getMaxNumSamples() const {
            return max_num_samples_;
        }

    protected:
        double sample_rate_{48000.0};
        size_t max_num_samples_{1};
    };
}
