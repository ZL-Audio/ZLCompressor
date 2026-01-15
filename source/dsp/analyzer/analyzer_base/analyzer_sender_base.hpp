// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <array>
#include <span>

#include "../../container/abstract_fifo.hpp"
#include "../../lock/spin_lock.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::analyzer {
    /**
     * an analyzer sender which pushes input samples into FIFOs
     * @tparam FloatType the float type of input audio buffers
     * @tparam kNum the number of analyzers
     */
    template <typename FloatType, size_t kNum>
    class AnalyzerSenderBase {
    public:
        explicit AnalyzerSenderBase() = default;

        virtual ~AnalyzerSenderBase() = default;

        virtual void prepare(double sample_rate, size_t max_num_samples, std::array<size_t, kNum> num_channels) = 0;

        /**
         * push input samples into FIFOs
         * @param buffers
         * @param num_samples
         */
        void process(std::array<std::span<FloatType*>, kNum> buffers, const size_t num_samples) {
            // calculate free space
            const int free_space = std::min(static_cast<int>(num_samples), abstract_fifo_.getNumFree());
            if (free_space == 0) { return; }
            // push samples
            const auto range = abstract_fifo_.prepareToWrite(free_space);
            for (size_t i = 0; i < kNum; ++i) {
                if (!is_on_[i]) { continue; }
                const auto buffer = buffers[i];
                if (range.block_size1 > 0) {
                    for (size_t chan = 0; chan < buffer.size(); ++chan) {
                        zldsp::vector::copy(sample_fifos_[i][chan].data() + static_cast<size_t>(range.start_index1),
                                            buffer[chan],
                                            static_cast<size_t>(range.block_size1));
                    }
                }
                if (range.block_size2 > 0) {
                    for (size_t chan = 0; chan < buffer.size(); ++chan) {
                        zldsp::vector::copy(sample_fifos_[i][chan].data() + static_cast<size_t>(range.start_index2),
                                            buffer[chan] + static_cast<size_t>(range.block_size1),
                                            static_cast<size_t>(range.block_size2));
                    }
                }
            }
            abstract_fifo_.finishWrite(free_space);
        }

        void setON(const size_t idx, const bool on) {
            is_on_[idx] = on;
        }

        zldsp::container::AbstractFIFO& getAbstractFIFO() {
            return abstract_fifo_;
        }

        std::array<std::vector<std::vector<float>>, kNum>& getSampleFIFOs() {
            return sample_fifos_;
        }

        zldsp::lock::SpinLock& getLock() {
            return lock_;
        }

        double getSampleRate() const {
            return sample_rate_;
        }

        size_t getMaxNumSamples() const {
            return max_num_samples_;
        }

    protected:
        zldsp::lock::SpinLock lock_;

        std::array<std::vector<std::vector<float>>, kNum> sample_fifos_;
        zldsp::container::AbstractFIFO abstract_fifo_{0};

        std::array<bool, kNum> is_on_{};

        double sample_rate_{48000.0};

        size_t max_num_samples_{1};

        void setFIFOSize(const size_t fifo_size, std::array<size_t, kNum>& num_channels) {
            abstract_fifo_.setCapacity(static_cast<int>(fifo_size));
            for (size_t i = 0; i < kNum; ++i) {
                sample_fifos_[i].resize(num_channels[i]);
                for (size_t chan = 0; chan < num_channels[i]; ++chan) {
                    sample_fifos_[i][chan].resize(fifo_size);
                    std::fill(sample_fifos_[i][chan].begin(), sample_fifos_[i][chan].end(), 0.f);
                }
            }
        }
    };
}
