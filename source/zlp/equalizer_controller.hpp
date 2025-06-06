// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../dsp/filter/filter.hpp"
#include "../dsp/fft_analyzer/fft_analyzer.hpp"
#include "../dsp/gain/origin_gain.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlp {
    class EqualizerController {
    public:
        static constexpr size_t kBandNum = 8;

        enum FilterStatus {
            kOff, kBypass, kOn
        };

        explicit EqualizerController();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(std::array<double *, 2> pointers, size_t num_samples);

    private:
        zldsp::gain::Gain<double> gain_{};

        std::array<zldsp::filter::IIR<double, 16>, kBandNum> filters_{};
        zldsp::analyzer::MultipleFFTAnalyzer<double, 2, 100> fft_analyzer_;

        std::array<std::atomic<FilterStatus>, kBandNum> filter_status_;
        std::vector<size_t> on_indices_{};

        void prepareBuffer();
    };
}
