// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <limits>

#include "../../vector/vector.hpp"
#include "../../container/fifo/fifo_base.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::analyzer {
    class MagRMSHistReceiver {
    public:
        explicit MagRMSHistReceiver() = default;

        /**
         * pull data from the given FIFO range and push RMS values into the hist
         * @param range
         * @param fifo
         * @param min_db
         * @param max_db
         * @return whether the hist has been updated
         */
        bool run(const zldsp::container::FIFORange range,
                 std::vector<std::vector<float>>& fifo,
                 const float min_db, const float max_db) {
            bool update_flag = false;
            auto process_segment = [&](size_t start, size_t size) {
                while (size > max_num_samples_ - current_num_samples_) {
                    const auto shift = max_num_samples_ - current_num_samples_;
                    for (size_t ch = 0; ch < fifo.size(); ++ch) {
                        sqr_sum_ += vector::sum_sqr(fifo[ch].data() + start, shift);
                    }

                    addToHist(min_db, max_db);

                    update_flag = true;
                    start += shift;
                    size -= shift;
                    current_num_samples_ = 0;
                    sqr_sum_ = 0.;
                }
                if (size > 0) {
                    for (size_t ch = 0; ch < fifo.size(); ++ch) {
                        sqr_sum_ += vector::sum_sqr(fifo[ch].data() + start, size);
                    }
                    current_num_samples_ += size;
                }
            };

            process_segment(static_cast<size_t>(range.start_index1), static_cast<size_t>(range.block_size1));
            process_segment(static_cast<size_t>(range.start_index2), static_cast<size_t>(range.block_size2));

            return update_flag;
        }

        void setHistSize(const size_t hist_size) {
            hist_.resize(hist_size);
            reset();
        }

        void setMaxNumSamples(const size_t max_num_samples) {
            max_num_samples_ = max_num_samples;
            reset();
        }

        void reset() {
            sqr_sum_ = 0.;
            current_num_samples_ = 0;
            std::ranges::fill(hist_, 0.);
        }

        void updateHeight(const float max_height, std::span<float> heights) const {
            const auto max_hist = std::max(vector::max_of(hist_.data(), hist_.size()), 10.0);
            const auto scale = static_cast<double>(max_height) / max_hist;
            heights[0] = static_cast<float>((hist_[0] * 0.75 + hist_[1] * .25) * scale);
            for (size_t i = 1; i < hist_.size() - 1; ++i) {
                heights[i] = static_cast<float>((hist_[i] * 0.5 + (hist_[i - 1] + hist_[i + 1]) * .25) * scale);
            }
            heights.back() = static_cast<float>((hist_.back() * 0.75 + hist_[hist_.size() - 2] * .25) * scale);
        }

    private:
        double sqr_sum_{0.};
        size_t max_num_samples_{0};
        size_t current_num_samples_{0};

        std::vector<double> hist_;

        void addToHist(const float min_db, const float max_db) {
            const auto db = chore::squareGainToDecibels(sqr_sum_ / static_cast<double>(max_num_samples_));
            if (db <= static_cast<double>(min_db)) {
                return;
            }
            const auto range_db = static_cast<double>(min_db - max_db);
            if (std::abs(range_db) < std::numeric_limits<double>::epsilon()) {
                return;
            }
            const auto proportion = std::clamp(
                (db - static_cast<double>(max_db)) / range_db, 0.0, 1.0);
            const auto idx = static_cast<size_t>(std::floor(proportion * static_cast<double>(hist_.size())));
            hist_[std::clamp(idx, static_cast<size_t>(0), hist_.size() - static_cast<size_t>(1))] += 1.0;
        }
    };
}
