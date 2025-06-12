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
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_ref_.addParameterListener(kIDs[i], this);
            parameterChanged(kIDs[i], kDefaultVs[i]);
        }
        for (size_t band = 0; band < kBandNum; ++band) {
            const auto suffix = std::to_string(band);
            for (size_t i = 0; i < kBandIDs.size(); ++i) {
                const auto temp_id = kBandIDs[i] + suffix;
                parameters_ref_.addParameterListener(temp_id, this);
                parameterChanged(temp_id, kBandDefaultVs[i]);
            }
        }
    }

    EqualizerAttach::~EqualizerAttach() {
        for (auto &ID: kIDs) {
            parameters_ref_.removeParameterListener(ID, this);
        }
        for (size_t band = 0; band < kBandNum; ++band) {
            const auto suffix = std::to_string(band);
            for (size_t i = 0; i < kBandIDs.size(); ++i) {
                const auto temp_id = kBandIDs[i] + suffix;
                parameters_ref_.removeParameterListener(temp_id, this);
            }
        }
    }

    void EqualizerAttach::parameterChanged(const juce::String &parameter_ID, const float new_value) {
        const auto idx = static_cast<size_t>(parameter_ID.getTrailingIntValue());
        if (parameter_ID == PSideGain::kID) {
            controller_ref_.setGain(new_value);
        } else if (parameter_ID.startsWith(PFilterStatus::kID)) {
            controller_ref_.setFilterStatus(idx, static_cast<EqualizerController::FilterStatus>(new_value));
        } else if (parameter_ID.startsWith(PFreq::kID)) {
            controller_ref_.getFilter(idx).setFreq(static_cast<double>(new_value));
        } else if (parameter_ID.startsWith(PGain::kID)) {
            controller_ref_.getFilter(idx).setGain(static_cast<double>(new_value));
        } else if (parameter_ID.startsWith(PQ::kID)) {
            controller_ref_.getFilter(idx).setQ(static_cast<double>(new_value));
        } else if (parameter_ID.startsWith(PFilterType::kID)) {
            controller_ref_.getFilter(idx).setFilterType(
                static_cast<zldsp::filter::FilterType>(std::round(new_value)));
        } else if (parameter_ID.startsWith(POrder::kID)) {
            controller_ref_.getFilter(idx).setOrder(
                POrder::kOrderArray[static_cast<size_t>(std::round(new_value))]);
        }
    }
}
