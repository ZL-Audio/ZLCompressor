// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../computer/computer.hpp"
#include "../tracker/tracker.hpp"
#include "../follower/follower.hpp"
#include "../../chore/decibels.hpp"

namespace zldsp::compressor {
    template<typename FloatType, bool IsPeakMix, bool UseSmooth, bool UsePunch>
    class ClassicCompressor : public CompressorStyleBase<ClassicCompressor<FloatType, IsPeakMix, UseSmooth, UsePunch>,
                FloatType, IsPeakMix, UseSmooth, UsePunch> {
    public:
        using base = CompressorStyleBase<ClassicCompressor, FloatType, IsPeakMix, UseSmooth, UsePunch>;

        ClassicCompressor(KneeComputer<FloatType> &computer,
                          RMSTracker<FloatType, IsPeakMix> &tracker,
                          PSFollower<FloatType, UseSmooth, UsePunch> &follower)
            : base(computer, tracker, follower) {
        }

        void reset() override {
            base::follower_.reset(FloatType(0));
            x0_ = FloatType(0);
        }

        template<PPState CurrentPPState>
        void processImpl(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                // pass the feedback sample through the tracker
                base::tracker_.processSample(x0_);
                // get the db from the tracker
                const auto input_db = base::tracker_.getMomentaryDB();
                // pass through the computer and the follower
                const auto smooth_reduction_db = -base::follower_.template processSample<CurrentPPState>(
                    input_db - base::computer_.eval(input_db));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * chore::decibelsToGain(smooth_reduction_db);
                buffer[i] = smooth_reduction_db;
            }
        }

    private:
        FloatType x0_{FloatType(0)};
    };
}
