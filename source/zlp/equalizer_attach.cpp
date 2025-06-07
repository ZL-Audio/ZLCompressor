// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "equalizer_attach.hpp"

namespace zlp {
    EqualizerAttach::EqualizerAttach(juce::AudioProcessor &processor,
                                     juce::AudioProcessorValueTreeState &parameters,
                                     EqualizerController &controller)
        : processor_ref_(processor),
          parameters_ref_(parameters),
          controller_ref_(controller) {
        juce::ignoreUnused(processor_ref_);
        for (auto &ID: kIDs) {
            parameters_ref_.addParameterListener(ID, this);
        }
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameterChanged(kIDs[i], kDefaultVs[i]);
        }
    }

    EqualizerAttach::~EqualizerAttach() {
        for (auto &ID: kIDs) {
            parameters_ref_.removeParameterListener(ID, this);
        }
    }

    void EqualizerAttach::parameterChanged(const juce::String &parameter_ID, const float new_value) {
        if (parameter_ID == PSideGain::kID) {
            controller_ref_.setGain(new_value);
        }
    }
}
