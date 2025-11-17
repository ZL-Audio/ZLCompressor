// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "zlp_definitions.hpp"
#include "equalize_controller.hpp"

namespace zlp {
    class EqualizeAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit EqualizeAttach(juce::AudioProcessor& processor,
                                juce::AudioProcessorValueTreeState& parameters,
                                EqualizeController& controller);

        ~EqualizeAttach() override;

    private:
        juce::AudioProcessor& processor_ref_;
        juce::AudioProcessorValueTreeState& parameters_ref_;
        EqualizeController& controller_ref_;

        constexpr static std::array kIDs{
            PSideGain::kID
        };

        constexpr static std::array kDefaultVs{
            PSideGain::kDefaultV
        };

        constexpr static std::array kBandIDs{
            PFilterStatus::kID, PFilterType::kID, POrder::kID,
            PFreq::kID, PGain::kID, PQ::kID
        };

        constexpr static std::array kBandDefaultVs{
            static_cast<float>(PFilterStatus::kDefaultI),
            static_cast<float>(PFilterType::kDefaultI),
            static_cast<float>(POrder::kDefaultI),
            PFreq::kDefaultV, PGain::kDefaultV, PQ::kDefaultV
        };

        void parameterChanged(const juce::String& parameter_ID, float new_value) override;
    };
}
