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

        void prepare(const double sample_rate) override {
            this->sample_rate_.store(sample_rate);
            this->setTimeLength(0.001f * 999.0f);
            std::fill(this->current_mags_.begin(), this->current_mags_.end(), FloatType(-999));
        }

        void run() {
            if (this->to_reset_.exchange(false)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    auto &cumulative_count{cumulative_counts_[i]};
                    std::fill(cumulative_count.begin(), cumulative_count.end(), 0.);
                }
            }

            const int num_ready = this->abstract_fifo_.getNumReady();
            zldsp::container::AbstractFIFO::Range range{};
            this->abstract_fifo_.prepareToRead(num_ready, range);
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
            this->abstract_fifo_.finishedRead(num_ready);

            std::array<double, MagNum> maximum_counts{};
            for (size_t i = 0; i < MagNum; ++i) {
                maximum_counts[i] = *std::max_element(cumulative_counts_[i].begin(), cumulative_counts_[i].end());
            }
            const auto maximum_count = std::max(999.0 / this->time_length_.load(),
                                                *std::max_element(maximum_counts.begin(), maximum_counts.end()));
            const auto maximum_count_r = 1.0 / maximum_count;
            for (size_t i = 0; i < MagNum; ++i) {
                auto &cumulative_count{cumulative_counts_[i]};
                auto &avg_count{avg_counts_[i]};
                zldsp::vector::multiply(avg_count.data(), cumulative_count.data(), maximum_count_r, avg_count.size());
            }
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, MagNum> paths,
                        const std::array<bool, MagNum> is_close_path,
                        const juce::Rectangle<float> bound, size_t end_idx) {
            end_idx = std::min(end_idx, BinNum);
            const auto delta_y = bound.getHeight() / static_cast<float>(end_idx - 1);
            for (size_t i = 0; i < MagNum; ++i) {
                const auto y = bound.getY();
                const auto &avg_count{avg_counts_[i]};
                constexpr size_t idx = 0;
                const auto x = bound.getX() + static_cast<float>(avg_count[idx]) * bound.getWidth();
                if (is_close_path[i]) {
                    paths[i].get().startNewSubPath(bound.getTopLeft());
                    paths[i].get().lineTo(x, y);
                } else {
                    paths[i].get().startNewSubPath(x, y);
                }
            }

            for (size_t i = 0; i < MagNum; ++i) {
                auto y = delta_y;
                const auto &avg_count{avg_counts_[i]};
                for (size_t idx = 1; idx < end_idx; ++idx) {
                    const auto x = bound.getX() + static_cast<float>(avg_count[idx]) * bound.getWidth();
                    paths[i].get().lineTo(x, y);
                    y += delta_y;
                }
            }
            for (size_t i = 0; i < MagNum; ++i) {
                if (is_close_path[i]) {
                    paths[i].get().lineTo(bound.getBottomLeft());
                    paths[i].get().closeSubPath();
                }
            }
        }

    protected:
        std::array<std::array<double, BinNum>, MagNum> cumulative_counts_{};
        std::array<std::array<double, BinNum>, MagNum> avg_counts_{};

        static inline void updateHist(std::array<double, BinNum> &hist, const double x) {
            const auto idx = static_cast<size_t>(std::max(0., std::round(-x)));
            if (idx < BinNum) {
                zldsp::vector::multiply(hist.data(),0.999999, hist.size());
                hist[idx] += 1.;
            }
        }
    };
}
