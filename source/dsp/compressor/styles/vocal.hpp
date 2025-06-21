// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../chore/decibels.hpp"
#include "style_base.hpp"

namespace zldsp::compressor {
    template<typename FloatType>
    class VocalCompressor final : public CompressorStyleBase<FloatType> {
    public:
        using base = CompressorStyleBase<FloatType>;

        VocalCompressor(ComputerBase<FloatType> &computer,
                        RMSTracker<FloatType> &tracker,
                        FollowerBase<FloatType> &follower)
            : base(computer, tracker, follower) {
        }

        void reset(const FloatType v = FloatType(0)) {
            base::follower_.reset(v);
            x0_ = FloatType(v);
        }

        template<bool UseRMS = false>
        void process(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                FloatType input_db;
                if constexpr (UseRMS) {
                    // pass the feedback sample through the tracker
                    base::tracker_.processSample(x0_);
                    // get the db from the tracker
                    input_db = base::tracker_.getMomentaryDB();
                } else {
                    input_db = chore::gainToDecibels(std::abs(x0_));
                }
                // pass through the computer and the follower
                const auto smooth_reduction_gain = -base::follower_.processSample(
                    -chore::decibelsToGain(base::computer_.eval(input_db)));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * smooth_reduction_gain;
                buffer[i] = smooth_reduction_gain;
            }
            auto vector = kfr::make_univector(buffer, num_samples);
            vector = FloatType(20) * kfr::log10(vector);
        }

    private:
        FloatType x0_{FloatType(0)};
    };
}
