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
#include "controller.hpp"

namespace zlp {
    class CompressAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit CompressAttach(juce::AudioProcessor &processor,
                                juce::AudioProcessorValueTreeState &parameters,
                                Controller &controller);

        ~CompressAttach() override;

    private:
        juce::AudioProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_;
        Controller &controller_ref_;

        constexpr static std::array kIDs{
            PCompStyle::kID,
            PThreshold::kID, PRatio::kID, PKneeW::kID, PCurve::kID,
            PAttack::kID, PRelease::kID, PPunch::kID, PSmooth::kID
        };

        constexpr static std::array kDefaultVs{
            static_cast<float>(PCompStyle::kDefaultI),
            PThreshold::kDefaultV, PRatio::kDefaultV, PKneeW::kDefaultV, PCurve::kDefaultV,
            PAttack::kDefaultV, PRelease::kDefaultV, PPunch::kDefaultV, PSmooth::kDefaultV
        };

        void parameterChanged(const juce::String &parameter_ID, float new_value) override;
    };
} // zlp
