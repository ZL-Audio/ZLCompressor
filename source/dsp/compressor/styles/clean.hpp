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
    class CleanCompressor final {
    public:
        CleanCompressor() = default;

        template <typename F>
        static void reset(F& follower, const FloatType v = FloatType(0)) {
            follower.reset(v);
        }

        template <typename C, typename F, PPState pp_state = PPState::kOff, SState s_state = SState::kOff>
        static void process(C& computer, F& follower,
                            FloatType* buffer, const size_t num_samples) {
            auto vector = kfr::make_univector(buffer, num_samples);
            vector = FloatType(20) * kfr::log10(kfr::max(kfr::abs(vector), FloatType(1e-12)));
            // pass through the computer and the follower
            for (size_t i = 0; i < num_samples; ++i) {
                vector[i] = -follower.template processSample<pp_state, s_state>(
                    -computer.eval(vector[i]));
            }
        }

        template <typename C, typename F, PPState pp_state = PPState::kOff, SState s_state = SState::kOff>
        static void process(C& computer, F& follower, RMSTracker<FloatType>& tracker,
                            FloatType* buffer, const size_t num_samples) {
            auto vector = kfr::make_univector(buffer, num_samples);
            // pass through the tracker
            for (size_t i = 0; i < num_samples; ++i) {
                tracker.processSample(vector[i]);
                vector[i] = tracker.getMomentarySquare();
            }
            // transfer square sum to db
            const auto mean_scale = FloatType(1) / static_cast<FloatType>(tracker.getCurrentBufferSize());
            vector = FloatType(10) * kfr::log10(kfr::max(vector * mean_scale, FloatType(1e-12)));
            // pass through the computer and the follower
            for (size_t i = 0; i < num_samples; ++i) {
                vector[i] = -follower.template processSample<pp_state, s_state>(
                    -computer.eval(vector[i]));
            }
        }
    };
}
