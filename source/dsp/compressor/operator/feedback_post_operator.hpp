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

#include "forward_post_operator.hpp"

namespace zlCompressor {
    template<typename FloatType, bool isLogDomain,
        bool useCurve, bool useBound,
        bool isPeakMix,
        bool useSmooth, bool usePunch>
    class FeedbackPostOperator {
    public:
        FeedbackPostOperator(KneeComputer<FloatType, useCurve, useBound> &computer,
                             RMSTracker<FloatType, isPeakMix> &tracker,
                             PSFollower<FloatType, useSmooth, usePunch> &follower)
            : forwardOp(computer, tracker, follower) {
        }

        template<bool returnDB = false>
        FloatType processSample(FloatType x) {
            const auto reduction = forwardOp.processSample(x0);
            x0 = x * reduction;
            if (returnDB) {
                return juce::Decibels::gainToDecibels(reduction, FloatType(-240));
            } else {
                return reduction;
            }
        }

    private:
        ForwardPostOperator<FloatType, isLogDomain, useCurve, useBound, isPeakMix, useSmooth, usePunch> forwardOp;
        FloatType x0{FloatType(0)};
    };
}
