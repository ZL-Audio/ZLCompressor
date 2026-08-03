// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <vector>
#include <cmath>
#include <algorithm>

namespace zldsp::analyzer {
    class SpectrumSmoother {
    public:
        enum class SmoothMethod {
            kOCT, kERB
        };

        struct SmoothBounds {
            size_t pass1_start;
            size_t pass1_end;
            size_t pass2_start;
            size_t pass2_end;
            size_t target_start;
            size_t target_end;
        };

        explicit SpectrumSmoother() = default;

        void prepare(const size_t fft_size) {
            const size_t num_bins = fft_size / 2 + 1;
            low_idx_.resize(num_bins);
            high_idx_.resize(num_bins);
            count_req_.resize(num_bins);
            temp_cum_sum_.resize(num_bins + 1);
        }

        void setSmooth(const double smooth, const double sample_rate, const SmoothMethod method) {
            switch (method) {
            case SmoothMethod::kOCT: {
                const double factor = std::pow(2.0, smooth * 0.5);
                const double factor_rep = 1.0 / factor;
                const size_t max_idx = low_idx_.size();

                for (size_t i = 0; i < low_idx_.size(); ++i) {
                    const double lower = static_cast<double>(i) * factor_rep;
                    const double upper = static_cast<double>(i) * factor;

                    low_idx_[i] = static_cast<size_t>(std::round(lower));
                    high_idx_[i] = std::min(max_idx, static_cast<size_t>(std::round(upper) + 1.0));

                    const size_t bin_count = high_idx_[i] - low_idx_[i];
                    count_req_[i] = 1.0f / static_cast<float>(std::max<size_t>(1, bin_count));
                }
                break;
            }
            case SmoothMethod::kERB: {
                const auto factor = smooth * 0.5;
                const auto num_bins = low_idx_.size();
                const auto fft_size = (num_bins - 1) * 2;
                const auto delta_f = sample_rate / static_cast<double>(fft_size);
                const auto const_term = 24.7 / delta_f;
                const auto max_idx_dbl = static_cast<double>(num_bins - 1);

                for (size_t i = 0; i < num_bins; ++i) {
                    const double erb_bins = 0.107939 * static_cast<double>(i) + const_term;
                    const double half_width = erb_bins * factor;
                    const double lower = std::max(0.0, static_cast<double>(i) - half_width);
                    const double upper = std::min(max_idx_dbl, static_cast<double>(i) + half_width);

                    low_idx_[i] = static_cast<size_t>(std::round(lower));
                    high_idx_[i] = std::min(num_bins, static_cast<size_t>(std::round(upper) + 1.0));

                    const size_t bin_count = high_idx_[i] - low_idx_[i];
                    count_req_[i] = 1.0f / static_cast<float>(std::max<size_t>(1, bin_count));
                }
                break;
            }
            }
        }

        void smooth(const std::span<float> spectrum_abs_sqr) {
            applyBoxcarAverage(spectrum_abs_sqr, 0, spectrum_abs_sqr.size(), 0, spectrum_abs_sqr.size());
            applyBoxcarAverage(spectrum_abs_sqr, 0, spectrum_abs_sqr.size(), 0, spectrum_abs_sqr.size());
        }

    protected:
        std::vector<size_t> low_idx_, high_idx_;
        std::vector<float> count_req_;
        std::vector<double> temp_cum_sum_;

        void applyBoxcarAverage(const std::span<float> data,
                                const size_t source_start, const size_t source_end,
                                const size_t target_start, const size_t target_end) {
            temp_cum_sum_[source_start] = 0.0;
            for (size_t i = source_start; i < source_end; ++i) {
                temp_cum_sum_[i + 1] = temp_cum_sum_[i] + static_cast<double>(data[i]);
            }
            for (size_t i = target_start; i < target_end; ++i) {
                data[i] = static_cast<float>(
                    temp_cum_sum_[high_idx_[i]] - temp_cum_sum_[low_idx_[i]]) * count_req_[i];
            }
        }
    };
}
