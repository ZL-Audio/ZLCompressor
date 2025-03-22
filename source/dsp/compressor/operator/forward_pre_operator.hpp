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

#include "helper.hpp"

namespace zlCompressor {
    template<typename FloatType, bool isLogDomain,
        bool useCurve, bool useBound,
        bool isPeakMix,
        bool useSmooth, bool usePunch>
    class ForwardPreOperator {
    public:
        ForwardPreOperator(KneeComputer<FloatType, useCurve, useBound> &computer,
                            RMSTracker<FloatType, isPeakMix> &tracker,
                            PSFollower<FloatType, useSmooth, usePunch> &follower)
            : m_computer(computer), m_tracker(tracker), m_follower(follower) {
        }

        FloatType processSample(FloatType x) {
            m_tracker.processSample(x);
            FloatType smoothInputDB;
            if (isLogDomain) {
                const auto smoothInputGain = m_follower.processSample(m_tracker.getMomentaryGain());
                smoothInputDB = gainToDecibels(smoothInputGain);
            } else {
                smoothInputDB = m_follower.processSample(m_tracker.getMomentaryDB());
            }
            return m_computer.processSample(smoothInputDB) - smoothInputDB;
        }

    private:
        KneeComputer<FloatType, useCurve, useBound> &m_computer;
        RMSTracker<FloatType, isPeakMix> &m_tracker;
        PSFollower<FloatType, useSmooth, usePunch> &m_follower;
    };
}
