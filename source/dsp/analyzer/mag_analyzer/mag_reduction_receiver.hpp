// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include "mag_receiver.hpp"

namespace zldsp::analyzer {
    class MagReductionReceiver {
    public:
        explicit MagReductionReceiver() = default;

        void run(const zldsp::container::FIFORange range,
                 const std::vector<std::vector<float>>& pre_fifo,
                 const std::vector<std::vector<float>>& post_fifo,
                 const MagType mag_type) {
            pre_receiver_.run(range, pre_fifo, mag_type);
            post_receiver_.run(range, post_fifo, mag_type);

            const auto& pre_dbs = pre_receiver_.getDBs();
            const auto& post_dbs = post_receiver_.getDBs();
            assert(pre_dbs.size() == post_dbs.size());
            reductions_.resize(pre_dbs.size());
            for (size_t chan = 0; chan < reductions_.size(); ++chan) {
                reductions_[chan] = post_dbs[chan] - pre_dbs[chan];
            }
        }

        static float calculateReduction(const zldsp::container::FIFORange range,
                                        const std::vector<std::vector<float>>& pre_fifo,
                                        const std::vector<std::vector<float>>& post_fifo,
                                        const MagType mag_type,
                                        const StereoType stereo_type) {
            const float pre_db = MagReceiver::calculate(range, pre_fifo, mag_type, stereo_type);
            const float post_db = MagReceiver::calculate(range, post_fifo, mag_type, stereo_type);

            return post_db - pre_db;
        }

        auto& getReductions() { return reductions_; }

    protected:
        MagReceiver pre_receiver_{}, post_receiver_{};
        std::vector<float> reductions_;
    };
}
