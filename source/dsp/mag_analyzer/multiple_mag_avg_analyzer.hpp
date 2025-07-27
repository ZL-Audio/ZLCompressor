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
    template<typename FloatType, size_t MagNum, size_t BinNum>
    class MultipleMagAvgAnalyzer : public MultipleMagBase<FloatType, MagNum, 1000> {
    public:
        explicit MultipleMagAvgAnalyzer() = default;

        ~MultipleMagAvgAnalyzer() override = default;

        void prepare(const double sample_rate, size_t max_num_samples) override {
            this->sample_rate_.store(sample_rate, std::memory_order::relaxed);
            this->max_num_samples_per_block_.store(static_cast<double>(max_num_samples), std::memory_order::relaxed);
            this->setTimeLength(0.001f * 999.0f);
            std::fill(this->current_mags_.begin(), this->current_mags_.end(), FloatType(-999));
        }

        void run() {
            if (this->to_reset_.exchange(false, std::memory_order::acquire)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    auto &cumulative_count{cumulative_counts_[i]};
                    std::fill(cumulative_count.begin(), cumulative_count.end(), 0.);
                }
            }

            const int num_ready = this->abstract_fifo_.getNumReady();
            const auto range = this->abstract_fifo_.prepareToRead(num_ready);
            for (size_t i = 0; i < MagNum; ++i) {
                auto &mag_fifo{this->mag_fifos_[i]};
                auto &cumulative_count{cumulative_counts_[i]};
                for (auto idx = range.start_index1; idx < range.start_index1 + range.block_size1; ++idx) {
                    updateHist(cumulative_count, mag_fifo[static_cast<size_t>(idx)]);
                }
                for (auto idx = range.start_index2; idx < range.start_index2 + range.block_size2; ++idx) {
                    updateHist(cumulative_count, mag_fifo[static_cast<size_t>(idx)]);
                }
            }
            this->abstract_fifo_.finishRead(num_ready);

            std::array<float, MagNum> maximum_counts{};
            for (size_t i = 0; i < MagNum; ++i) {
                maximum_counts[i] = *std::max_element(cumulative_counts_[i].begin(), cumulative_counts_[i].end());
            }
            const auto maximum_count = std::max(999.f / this->time_length_.load(),
                                                *std::max_element(maximum_counts.begin(), maximum_counts.end()));
            const auto maximum_count_r = 1.f / maximum_count;
            for (size_t i = 0; i < MagNum; ++i) {
                auto &cumulative_count{cumulative_counts_[i]};
                auto &avg_count{avg_counts_[i]};
                zldsp::vector::multiply(avg_count.data(), cumulative_count.data(), maximum_count_r, avg_count.size());
            }
        }

        void createPath(std::array<std::span<float>, MagNum> xs, std::span<float> ys, size_t end_idx,
                        const float width, const float height) {
            end_idx = std::min(std::min(end_idx, BinNum), ys.size());
            const auto delta_y = height / static_cast<float>(end_idx - 1);
            ys[0] = 0.f;
            for (size_t i = 1; i < end_idx; ++i) {
                ys[i] = ys[i - 1] + delta_y;
            }
            for (size_t i = 0; i < MagNum; ++i) {
                auto avg_vector = kfr::make_univector(avg_counts_[i].data(), end_idx);
                auto x_vector = kfr::make_univector(xs[i].data(), end_idx);
                x_vector = avg_vector * width;
                zldsp::vector::multiply(xs[i].data(), avg_counts_[i].data(), width, end_idx);
            }
        }

    protected:
        std::array<std::array<float, BinNum>, MagNum> cumulative_counts_{};
        std::array<std::array<float, BinNum>, MagNum> avg_counts_{};

        static inline void updateHist(std::array<float, BinNum> &hist, const float x) {
            const auto idx = static_cast<size_t>(std::max(0.f, std::round(-x)));
            if (idx < BinNum) {
                zldsp::vector::multiply(hist.data(), 0.999999f, hist.size());
                hist[idx] += 1.f;
            }
        }
    };
}
