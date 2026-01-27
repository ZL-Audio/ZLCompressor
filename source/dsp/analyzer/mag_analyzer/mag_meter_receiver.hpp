// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../vector/kfr_import.hpp"
#include "../../container/fifo/fifo_base.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::analyzer {
    template <size_t kNum>
    class MagMeterReceiver {
    public:
        explicit MagMeterReceiver() = default;

        void run(const zldsp::container::FIFORange range,
                 std::array<std::vector<std::vector<float>>, kNum>& sample_fifo,
                 const MagType mag_type) {
            for (size_t idx = 0; idx < kNum; ++idx) {
                dbs_[idx].resize(sample_fifo[idx].size());
            }

            for (size_t idx = 0; idx < kNum; ++idx) {
                for (size_t chan = 0; chan < sample_fifo[idx].size(); ++chan) {
                    float value{0.f};
                    auto analyze_expr = [&](const auto& expression) {
                        if (mag_type == MagType::kPeak) {
                            value = std::max(value, kfr::absmaxof(expression));
                        } else {
                            value += kfr::sumsqr(expression);
                        }
                    };
                    if (range.block_size1 > 0) {
                        analyze_expr(kfr::make_univector(
                            sample_fifo[idx][chan].data() + static_cast<size_t>(range.start_index1),
                            static_cast<size_t>(range.block_size1)));
                    }
                    if (range.block_size2 > 0) {
                        analyze_expr(kfr::make_univector(
                            sample_fifo[idx][chan].data() + static_cast<size_t>(range.start_index2),
                            static_cast<size_t>(range.block_size2)));
                    }
                    if (mag_type == MagType::kPeak) {
                        dbs_[idx][chan] = chore::gainToDecibels(value);
                    } else {
                        dbs_[idx][chan] = chore::squareGainToDecibels(
                            value / static_cast<float>(range.block_size1 + range.block_size2));
                    }
                }
            }
        }

    protected:
        std::array<std::vector<float>, kNum> dbs_;
    };
}
