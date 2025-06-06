// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "equalizer_controller.hpp"

namespace zlp {
    EqualizerController::EqualizerController() {
        on_indices_.reserve(kBandNum);
    }

    void EqualizerController::prepare(const juce::dsp::ProcessSpec &spec) {
        for (auto &filter: filters_) {
            filter.prepare(spec.sampleRate, static_cast<size_t>(spec.numChannels));
        }
        gain_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize), 0.01);
    }

    void EqualizerController::prepareBuffer() {
        for (auto &filter: filters_) {
            filter.prepareBuffer();
        }
    }

    void EqualizerController::process(std::array<double *, 2> pointers, const size_t num_samples) {
        gain_.process(pointers, num_samples);
        for (const auto &i: on_indices_) {
            switch (filter_status_[i].load(std::memory_order::relaxed)) {
                case kOff: {
                    break;
                }
                case kBypass: {
                    filters_[i].template process<true>(pointers, num_samples);
                    break;
                }
                case kOn: {
                    filters_[i].template process<false>(pointers, num_samples);
                    break;
                }
            }
        }
    }
}
