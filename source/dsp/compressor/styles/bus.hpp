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

namespace zldsp::compressor {
    template<typename FloatType, bool UseCurve, bool IsPeakMix, bool UseSmooth, bool UsePunch>
    class BusCompressor {
    public:
        BusCompressor(KneeComputer<FloatType, UseCurve> &computer_,
                          RMSTracker<FloatType, IsPeakMix> &tracker_,
                          PSFollower<FloatType, UseSmooth, UsePunch> &follower_)
            : computer(computer_), tracker(tracker_), follower(follower_) {
        }

        void reset() {
            follower.reset(FloatType(0));
            g0 = FloatType(1);
        }

        void process(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                buffer[i] = processSample(buffer[i]);
            }
        }

    private:
        KneeComputer<FloatType, UseCurve> &computer;
        RMSTracker<FloatType, IsPeakMix> &tracker;
        PSFollower<FloatType, UseSmooth, UsePunch> &follower;
        FloatType g0{FloatType(1)};

        template<typename Type>
        static Type decibelsToGain(const Type decibels) {
            return std::pow(static_cast<Type>(10.0), decibels * static_cast<Type>(0.05));
        }

        FloatType processSample(FloatType x) {
            tracker.processSample(x * g0);
            const auto inputDB = tracker.getMomentaryDB();
            const auto smoothReductionDB = follower.processSample(inputDB - computer.processSample(inputDB));
            g0 = decibelsToGain(smoothReductionDB);
            return -smoothReductionDB;
        }
    };
}
