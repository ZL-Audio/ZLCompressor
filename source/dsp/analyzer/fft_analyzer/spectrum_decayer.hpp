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
#include <vector>

namespace zldsp::analyzer {
    class SpectrumDecayer {
    public:
        explicit SpectrumDecayer() = default;

        void prepare(const size_t fft_size) {
            state_.resize(fft_size / 2 + 1);
            decay_mul_.resize(fft_size / 2 + 1);
            std::ranges::fill(state_.begin(), state_.end(), -240.f);
            std::ranges::fill(decay_mul_.begin(), decay_mul_.end(), 1.f);
        }

        void setDecaySpeed(const float refresh_rate, const float decay_per_second) {
            decay_per_call_ = decay_per_second / refresh_rate;
        }

        void decay(std::span<float> spectrum_db, const bool frozen = false) {
            if (frozen) {
                for (size_t i = 0; i < spectrum_db.size(); ++i) {
                    spectrum_db[i] = std::max(spectrum_db[i], state_[i]);
                    state_[i] = spectrum_db[i];
                }
            } else {
                for (size_t i = 0; i < spectrum_db.size(); ++i) {
                    if (spectrum_db[i] < state_[i]) {
                        spectrum_db[i] = std::max(spectrum_db[i], state_[i] + decay_mul_[i] * decay_per_call_);
                        state_[i] = spectrum_db[i];
                        decay_mul_[i] = std::min(decay_mul_[i] * 1.033f, 10.f);
                    } else {
                        state_[i] = spectrum_db[i];
                        decay_mul_[i] = 1.f;
                    }
                }
            }
        }

    private:
        std::vector<float> state_{};
        std::vector<float> decay_mul_{};
        float decay_per_call_{0.};
    };
}
