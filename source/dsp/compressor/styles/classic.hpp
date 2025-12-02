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
#include "../tracker/tracker.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    template <typename FloatType>
    class ClassicCompressor final {
    public:
        ClassicCompressor() = default;

        template <typename F>
        void reset(F& follower, const FloatType v = FloatType(0)) {
            follower.reset(v);
            x0_ = FloatType(v);
        }

        template <typename C, typename F, PPState pp_state = PPState::kOff, SState s_state = SState::kOff>
        void process(C& computer, F& follower,
                     FloatType* buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                FloatType input_db;
                input_db = chore::gainToDecibels(std::abs(x0_));
                // pass through the computer and the follower
                const auto smooth_reduction_db = -follower.template processSample<pp_state, s_state>(
                    -computer.eval(input_db));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * chore::decibelsToGain(smooth_reduction_db);
                buffer[i] = smooth_reduction_db;
            }
        }

        template <typename C, typename F, PPState pp_state = PPState::kOff, SState s_state = SState::kOff>
        void process(C& computer, F& follower, RMSTracker<FloatType>& tracker,
                     FloatType* buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                FloatType input_db;
                tracker.processSample(x0_);
                // get the db from the tracker
                input_db = tracker.getMomentaryDB();
                // pass through the computer and the follower
                const auto smooth_reduction_db = -follower.template processSample<pp_state, s_state>(
                    -computer.eval(input_db));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * chore::decibelsToGain(smooth_reduction_db);
                buffer[i] = smooth_reduction_db;
            }
        }

    private:
        FloatType x0_{FloatType(0)};
    };
}
