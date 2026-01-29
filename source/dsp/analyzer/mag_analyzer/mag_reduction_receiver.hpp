// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cassert>
#include "../../vector/kfr_import.hpp"
#include "../../container/fifo/fifo_base.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::analyzer {
    class MagReductionReceiver {
    public:
        explicit MagReductionReceiver() = default;

        void run(const zldsp::container::FIFORange range,
                 std::vector<std::vector<float>>& pre_fifo,
                 std::vector<std::vector<float>>& post_fifo) {
            assert(pre_fifo.size() == post_fifo.size());
            reductions_.resize(pre_fifo.size());
            for (size_t chan = 0; chan < pre_fifo.size(); ++chan) {
                double pre_sqrsum{1e-24};
                double post_sqrsum{1e-24};
                auto analyze_range = [&](const int start_index, const int block_size) {
                    auto pre_v = kfr::make_univector(
                        pre_fifo[chan].data() + static_cast<size_t>(start_index), static_cast<size_t>(block_size));
                    auto post_v = kfr::make_univector(
                        post_fifo[chan].data() + static_cast<size_t>(start_index), static_cast<size_t>(block_size));
                    pre_sqrsum += kfr::sumsqr(pre_v);
                    post_sqrsum += kfr::sumsqr(post_v);
                };
                if (range.block_size1 > 0) {
                    analyze_range(range.start_index1, range.block_size1);
                }
                if (range.block_size2 > 0) {
                    analyze_range(range.start_index2, range.block_size2);
                }
                const auto block_size = static_cast<double>(range.block_size1 + range.block_size2);
                const auto post_db = chore::squareGainToDecibels(post_sqrsum / block_size);
                const auto pre_db = chore::squareGainToDecibels(pre_sqrsum / block_size);
                reductions_[chan] = static_cast<float>(post_db - pre_db);
            }
        }

        auto& getReductions() {
            return reductions_;
        }

    protected:
        std::vector<float> reductions_;
    };
}
