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

        void prepare(const double sample_rate, size_t max_num_samples) override {
            this->sample_rate_.store(sample_rate, std::memory_order::relaxed);
            this->max_num_samples_per_block_.store(static_cast<double>(max_num_samples), std::memory_order::relaxed);
            this->setTimeLength(this->time_length_.load(std::memory_order::relaxed));
            std::fill(this->current_mags_.begin(), this->current_mags_.end(), FloatType(-999));
        }

        std::pair<int, bool> run(const int num_to_read = static_cast<int>(PointNum),
                                 int tolerance = 0) {
            // update tolerance
            const int block_tolerance = static_cast<int>(std::ceil(
                this->max_num_samples_per_block_.load(std::memory_order::relaxed) /
                    this->sample_rate_.load(std::memory_order::relaxed) /
                        this->time_length_.load(std::memory_order::relaxed) *
                            static_cast<double>(PointNum) * 1.5));
            if (tolerance > 0) {
                tolerance = std::max(tolerance, block_tolerance);
            }
            // calculate the number of points put into circular buffers
            const int fifo_num_ready = this->abstract_fifo_.getNumReady();
            if (this->to_reset_.exchange(false, std::memory_order::acquire)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    std::fill(this->circular_mags_[i].begin(), this->circular_mags_[i].end(), -240.f);
                }
                // clear FIFOs
                this->abstract_fifo_.prepareToRead(fifo_num_ready);
                this->abstract_fifo_.finishRead(fifo_num_ready);
                return {0, true};
            }

            if (fifo_num_ready <= 0) return {0, true};

            const bool to_reset_shift = (fifo_num_ready - num_to_read > tolerance) || (fifo_num_ready < num_to_read);
            int num_ready = 0;
            if (to_reset_shift) {
                if (fifo_num_ready > tolerance / 2) {
                    num_ready = fifo_num_ready - tolerance / 2;
                } else {
                    num_ready = 0;
                }
            } else {
                num_ready = std::min(fifo_num_ready, num_to_read);
            }

            if (num_ready <= 0) return {0, to_reset_shift};
            const auto num_ready_shift = static_cast<size_t>(num_ready);
            // shift circular buffers
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circular_peak{this->circular_mags_[i]};
                std::rotate(circular_peak.begin(),
                            circular_peak.begin() + num_ready_shift,
                            circular_peak.end());
            }
            // read from FIFOs
            const auto range = this->abstract_fifo_.prepareToRead(num_ready);
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
            this->abstract_fifo_.finishRead(num_ready);

            return {num_ready, to_reset_shift};
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
