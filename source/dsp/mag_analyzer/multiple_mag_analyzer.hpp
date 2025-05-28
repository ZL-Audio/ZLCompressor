// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "multiple_mag_base.hpp"

namespace zldsp::analyzer {
    template<typename FloatType, size_t MagNum, size_t PointNum>
    class MultipleMagAnalyzer : public MultipleMagBase<FloatType, MagNum, PointNum> {
    public:
        explicit MultipleMagAnalyzer() = default;

        ~MultipleMagAnalyzer() override = default;

        void prepare(const double sample_rate) override {
            this->sample_rate_.store(sample_rate, std::memory_order::relaxed);
            this->setTimeLength(this->time_length_.load(std::memory_order::relaxed));
            std::fill(this->current_mags_.begin(), this->current_mags_.end(), FloatType(-999));
        }

        int run(const int num_to_read = PointNum) {
            // calculate the number of points put into circular buffers
            const int fifo_num_ready = this->abstract_fifo_.getNumReady();
            if (this->to_reset_.exchange(false, std::memory_order::acquire)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    std::fill(this->circular_mags_[i].begin(), this->circular_mags_[i].end(), -240.f);
                }
                // clear FIFOs
                zldsp::container::AbstractFIFO::Range range{};
                this->abstract_fifo_.prepareToRead(fifo_num_ready, range);
                this->abstract_fifo_.finishedRead(fifo_num_ready);
                return 0;
            }
            const int num_ready = fifo_num_ready >= static_cast<int>(PointNum / 2)
                                      ? fifo_num_ready
                                      : std::min(fifo_num_ready, num_to_read);
            if (num_ready <= 0) return 0;
            const auto num_ready_shift = static_cast<size_t>(num_ready);
            // shift circular buffers
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circular_peak{this->circular_mags_[i]};
                std::rotate(circular_peak.begin(),
                            circular_peak.begin() + num_ready_shift,
                            circular_peak.end());
            }
            // read from FIFOs
            zldsp::container::AbstractFIFO::Range range{};
            this->abstract_fifo_.prepareToRead(num_ready, range);
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circular_peak{this->circular_mags_[i]};
                auto &peak_fifo{this->mag_fifos_[i]};
                size_t j = circular_peak.size() - static_cast<size_t>(num_ready);
                if (range.block_size1 > 0) {
                    std::copy(&peak_fifo[static_cast<size_t>(range.start_index1)],
                              &peak_fifo[static_cast<size_t>(range.start_index1 + range.block_size1)],
                              &circular_peak[j]);
                    j += static_cast<size_t>(range.block_size1);
                }
                if (range.block_size2 > 0) {
                    std::copy(&peak_fifo[static_cast<size_t>(range.start_index2)],
                              &peak_fifo[static_cast<size_t>(range.start_index2 + range.block_size2)],
                              &circular_peak[j]);
                }
            }
            this->abstract_fifo_.finishedRead(num_ready);

            return num_ready;
        }

        void createPath(std::span<float> xs, std::array<std::span<float>, MagNum> ys,
            const float width, const float height, const float shift = 0.f,
            const float min_db = -72.f, const float max_db = 0.f) {
            const auto delta_x = width / static_cast<float>(PointNum - 1);
            xs[0] = -shift * delta_x;
            for (size_t idx = 1; idx < PointNum; ++idx) {
                xs[idx] = xs[idx - 1] + delta_x;
            }
            const float scale = height / (max_db - min_db);
            for (size_t i = 0; i < MagNum; ++i) {
                auto mag_vector = kfr::make_univector(this->circular_mags_[i]);
                auto y_vector = kfr::make_univector(ys[i]);
                y_vector = (max_db - mag_vector) * scale;
            }
        }
    };
}
