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
    template<typename FloatType>
    class OpticalCompressor final : public CompressorStyleBase<FloatType>{
    public:
        using base = CompressorStyleBase<FloatType>;

        OpticalCompressor(ComputerBase<FloatType> &computer,
                          RMSTracker<FloatType> &tracker,
                          FollowerBase<FloatType> &follower)
            : base(computer, tracker, follower) {
        }

        void reset() override {
            base::follower_.reset(FloatType(0));
        }

        template <bool UseRMS = false>
        void process(FloatType *buffer, const size_t num_samples) {
            auto vector = kfr::make_univector(buffer, num_samples);
            if (UseRMS) {
                // pass through the tracker
                for (size_t i = 0; i < num_samples; ++i) {
                    base::tracker_.processSample(vector[i]);
                    vector[i] = base::tracker_.getMomentarySquare();
                }
                const auto mean_scale = FloatType(1) / static_cast<FloatType>(base::tracker_.getCurrentBufferSize());
                vector = kfr::sqrt(vector * mean_scale);
            } else {
                vector = kfr::abs(vector);
            }
            // pass through the follower
            for (size_t i = 0; i < num_samples; ++i) {
                vector[i] = base::follower_.processSample(vector[i]);
            }
            // transfer to db
            vector = FloatType(20) * kfr::log10(kfr::max(vector, FloatType(1e-12)));
            // pass through the computer
            for (size_t i = 0; i < num_samples; ++i) {
                vector[i] = base::computer_.eval(vector[i]);
            }
        }
    };
}
