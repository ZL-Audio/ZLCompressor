// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "style_base.hpp"

namespace zldsp::compressor {
    template<typename FloatType, bool IsPeakMix>
    class CleanCompressor : public CompressorStyleBase<CleanCompressor<FloatType, IsPeakMix>,
                FloatType, IsPeakMix> {
    public:
        using base = CompressorStyleBase<CleanCompressor, FloatType, IsPeakMix>;

        CleanCompressor(ComputerBase<FloatType> &computer,
                        RMSTracker<FloatType, IsPeakMix> &tracker,
                        FollowerBase<FloatType> &follower)
            : base(computer, tracker, follower) {
        }

        void reset() override {
            base::follower_.reset(FloatType(0));
        }

        void process(FloatType *buffer, const size_t num_samples) override {
            auto vector = kfr::make_univector(buffer, num_samples);
            // pass through the tracker
            for (size_t i = 0; i < num_samples; ++i) {
                base::tracker_.processSample(vector[i]);
                vector[i] = std::max(base::tracker_.getMomentarySquare(), FloatType(1e-10));
            }
            // transfer square sum to db
            const auto mean_scale = FloatType(1) / static_cast<FloatType>(base::tracker_.getCurrentBufferSize());
            vector = FloatType(10) * kfr::log10(vector * mean_scale);
            // pass through the computer and the follower
            for (size_t i = 0; i < num_samples; ++i) {
                vector[i] = -base::follower_.processSample(-base::computer_.eval(vector[i]));
            }
        }
    };
}
