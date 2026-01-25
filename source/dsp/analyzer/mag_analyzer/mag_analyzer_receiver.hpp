// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "mag_analyzer_sender.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::analyzer {
    template <size_t kNum>
    class MagAnalyzerReceiver {
    public:
        explicit MagAnalyzerReceiver() = default;

        void run(const zldsp::container::AbstractFIFO::Range range,
                 std::array<std::vector<std::vector<float>>, kNum>& sample_fifo,
                 const MagType mag_type,
                 const StereoType stereo_type) {

            for (size_t idx = 0; idx < kNum; ++idx) {
                float value = 0.f;
                const auto& channels = sample_fifo[idx];
                const bool use_special_stereo = (channels.size() == 2 && stereo_type != StereoType::kStereo);

                auto analyze_expr = [&](const auto& expression) {
                    if (mag_type == MagType::kPeak) {
                        value = std::max(value, kfr::absmaxof(expression));
                    } else {
                        value += kfr::sumsqr(expression);
                    }
                };

                auto process_segment = [&](size_t start, size_t size) {
                    if (size == 0) {
                        return;
                    }

                    if (use_special_stereo) {
                        auto vL = kfr::make_univector(channels[0].data() + start, size);
                        auto vR = kfr::make_univector(channels[1].data() + start, size);

                        switch (stereo_type) {
                        case StereoType::kLeft:
                            analyze_expr(vL);
                            break;
                        case StereoType::kRight:
                            analyze_expr(vR);
                            break;
                        case StereoType::kMid:
                            analyze_expr(kSqrt2Over2 * (vL + vR));
                            break;
                        case StereoType::kSide:
                            analyze_expr(kSqrt2Over2 * (vL - vR));
                            break;
                        case StereoType::kStereo:
                        default:
                            break;
                        }
                    } else {
                        for (size_t ch = 0; ch < channels.size(); ++ch) {
                            analyze_expr(kfr::make_univector(channels[ch].data() + start, size));
                        }
                    }
                };

                process_segment(static_cast<size_t>(range.start_index1), static_cast<size_t>(range.block_size1));
                process_segment(static_cast<size_t>(range.start_index2), static_cast<size_t>(range.block_size2));

                if (mag_type == MagType::kPeak) {
                    dbs_[idx] = chore::gainToDecibels(value);
                } else {
                    dbs_[idx] = chore::squareGainToDecibels(
                        value / static_cast<float>(range.block_size1 + range.block_size2));
                }
            }
        }

        void updateY(const float height, const float max_db, const float min_db,
                     std::array<std::span<float>, kNum> ys) const {
            const auto scale = height / (min_db - max_db);
            for (size_t idx = 0; idx < kNum; ++idx) {
                auto y = ys[idx];
                std::ranges::rotate(y, y.begin() + 1);
                y.back() = (dbs_[idx] - max_db) * scale;
            }
        }

    protected:
        std::array<float, kNum> dbs_;
    };
}
