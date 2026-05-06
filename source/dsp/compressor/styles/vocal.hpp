// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../chore/decibels.hpp"
#include "../tracker/tracker.hpp"
#include "../follower/follower.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    template <typename FloatType>
    class VocalCompressor final {
    public:
        VocalCompressor() = default;

        template <typename F>
        void reset(F& follower, const FloatType v = FloatType(0)) {
            follower.reset(v);
            x0_ = FloatType(v);
        }

        template <typename C, typename F, PPState pp_state = PPState::kOff, SState s_state = SState::kOff>
        void process(C& computer, F& follower,
                     FloatType* __restrict buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const FloatType input_db = chore::gainToDecibels(std::abs(x0_));
                // pass through the computer and the follower
                const auto smooth_reduction_gain = -follower.template processSample<pp_state, s_state>(
                    -chore::decibelsToGain(computer.eval(input_db)));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * smooth_reduction_gain;
                buffer[i] = smooth_reduction_gain;
            }
            vector::mag_to_db(buffer, num_samples);
        }

        template <typename C, typename F, PPState pp_state = PPState::kOff, SState s_state = SState::kOff>
        void process(C& computer, F& follower, RMSTracker<FloatType>& tracker,
                     FloatType* __restrict buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                tracker.processSample(x0_);
                // get the db from the tracker
                const FloatType input_db = tracker.getMomentaryDB();
                // pass through the computer and the follower
                const auto smooth_reduction_gain = -follower.template processSample<pp_state, s_state>(
                    -chore::decibelsToGain(computer.eval(input_db)));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * smooth_reduction_gain;
                buffer[i] = smooth_reduction_gain;
            }
            vector::mag_to_db(buffer, num_samples);
        }

    private:
        FloatType x0_{FloatType(0)};
    };
}
