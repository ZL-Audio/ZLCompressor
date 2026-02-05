// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "mag_receiver_base.hpp"
#include "../../chore/decibels.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    class MagReceiver {
    public:
        explicit MagReceiver() = default;

        void run(const zldsp::container::FIFORange range,
                 const std::vector<std::vector<float>>& fifo,
                 const MagType mag_type) {
            dbs_.resize(fifo.size());
            for (size_t chan = 0; chan < fifo.size(); ++chan) {
                if (mag_type == MagType::kRMS) {
                    dbs_[chan] = chore::squareGainToDecibels(MagAnalyzerOps::calculateMS(range, fifo[chan]));
                } else {
                    dbs_[chan] = chore::gainToDecibels(MagAnalyzerOps::calculatePeak(range, fifo[chan]));
                }
            }
        }

        static float calculate(const zldsp::container::FIFORange range,
                               const std::vector<std::vector<float>>& fifo,
                               const MagType mag_type,
                               const StereoType stereo_type) {
            if (range.block_size1 + range.block_size2 == 0) {
                return 0.f;
            }
            if (fifo.size() != 2 || stereo_type == StereoType::kStereo) {
                switch (mag_type) {
                case MagType::kRMS: {
                    float sum_sqr{0.f};
                    for (size_t chan = 0; chan < fifo.size(); ++chan) {
                        sum_sqr += MagAnalyzerOps::calculateMS(range, fifo[chan]);
                    }
                    return chore::squareGainToDecibels(sum_sqr);
                }
                case MagType::kPeak: {
                    float peak{0.f};
                    for (size_t chan = 0; chan < fifo.size(); ++chan) {
                        peak = std::max(peak, MagAnalyzerOps::calculatePeak(range, fifo[chan]));
                    }
                    return chore::gainToDecibels(peak);
                }
                default:
                    return 0.f;
                }
            }
            float value{0.f};
            auto analyze_expr = [&](const auto& expression) {
                if (mag_type == MagType::kPeak) {
                    value = std::max(value, kfr::absmaxof(expression));
                } else {
                    value += kfr::sumsqr(expression);
                }
            };
            auto process_segment = [&](const size_t start, const size_t size) {
                if (size == 0) {
                    return;
                }
                auto vL = kfr::make_univector(fifo[0].data() + start, size);
                auto vR = kfr::make_univector(fifo[1].data() + start, size);

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
            };
            process_segment(static_cast<size_t>(range.start_index1), static_cast<size_t>(range.block_size1));
            process_segment(static_cast<size_t>(range.start_index2), static_cast<size_t>(range.block_size2));
            switch (mag_type) {
            case MagType::kRMS: {
                return chore::squareGainToDecibels(value / static_cast<float>(range.block_size1 + range.block_size2));
            }
            case MagType::kPeak: {
                return chore::gainToDecibels(value);
            }
            default:
                return 0.f;
            }
        }

        const std::vector<float>& getDBs() { return dbs_; }

    protected:
        std::vector<float> dbs_;
    };
}
