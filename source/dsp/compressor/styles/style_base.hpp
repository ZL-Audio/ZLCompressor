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

#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    template<typename Derived, typename FloatType, bool IsPeakMix, bool UseSmooth, bool UsePunch>
    class CompressorStyleBase {
    public:
        CompressorStyleBase(KneeComputer<FloatType> &computer,
                            RMSTracker<FloatType, IsPeakMix> &tracker,
                            PSFollower<FloatType, UseSmooth, UsePunch> &follower)
            : computer_(computer), tracker_(tracker), follower_(follower) {
        }

        virtual ~CompressorStyleBase() = default;

        virtual void reset() = 0;

        void process(FloatType *buffer, const size_t num_samples) {
            switch (follower_.getCurrentPPState()) {
                case PPState::kOff: {
                    static_cast<Derived *>(this)->template processImpl<PPState::kOff>(buffer, num_samples);
                    break;
                }
                case PPState::kPunch: {
                    static_cast<Derived *>(this)->template processImpl<PPState::kOff>(buffer, num_samples);
                    break;
                }
                case PPState::kPump: {
                    static_cast<Derived *>(this)->template processImpl<PPState::kOff>(buffer, num_samples);
                    break;
                }
                default: break;
            }
        }

    protected:
        KneeComputer<FloatType> &computer_;
        RMSTracker<FloatType, IsPeakMix> &tracker_;
        PSFollower<FloatType, UseSmooth, UsePunch> &follower_;
    };
}
