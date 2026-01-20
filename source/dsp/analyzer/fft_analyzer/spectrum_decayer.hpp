// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>

#include "../../vector/kfr_import.hpp"

namespace zldsp::analyzer {
    class SpectrumDecayer {
    public:
        explicit SpectrumDecayer() = default;

        void prepare(const size_t fft_size) {
            state_.resize(fft_size / 2 + 1);
            std::ranges::fill(state_.begin(), state_.end(), -240.f);
        }

        void setDecaySpeed(const float refresh_rate, const float decay_per_second) {
            decay_per_call_ = decay_per_second / refresh_rate;
        }

        void decay(std::span<float> spectrum_db, const bool frozen = false) {
            auto v = kfr::make_univector(spectrum_db);
            auto s = state_.slice(0, v.size());
            if (frozen) {
                v = kfr::max(s, v);
            } else {
                v = kfr::max(s + decay_per_call_, v);
            }
            s = v;
        }

    private:
        kfr::univector<float> state_{};
        float decay_per_call_{0.};
    };
}
