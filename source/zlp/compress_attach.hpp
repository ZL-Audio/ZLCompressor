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
#include "compressor_controller.hpp"

namespace zlp {
    class CompressAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit CompressAttach(juce::AudioProcessor &processor,
                                juce::AudioProcessorValueTreeState &parameters,
                                CompressorController &controller);

        ~CompressAttach() override;

    private:
        juce::AudioProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_;
        CompressorController &controller_ref_;

        zldsp::compressor::KneeComputer<float, true> &computer_ref_;
        zldsp::compressor::PSFollower<float, true, true> &follower_ref_;

        constexpr static std::array kIDs{
            PCompStyle::kID,
            PThreshold::kID, PRatio::kID, PKneeW::kID, PCurve::kID,
            PAttack::kID, PRelease::kID, PPump::kID, PSmooth::kID,
            PHold::kID, PRange::kID, POutGain::kID, PWet::kID,
            PSideStereoMode::kID, PSideStereoSwap::kID,
            PSideStereoLink::kID,
            PSideStereoWet1::kID, PSideStereoWet2::kID,
            POversample::kID, PLookAhead::kID,
            PCompON::kID, PCompDelta::kID
        };

        constexpr static std::array kDefaultVs{
            static_cast<float>(PCompStyle::kDefaultI),
            PThreshold::kDefaultV, PRatio::kDefaultV, PKneeW::kDefaultV, PCurve::kDefaultV,
            PAttack::kDefaultV, PRelease::kDefaultV, PPump::kDefaultV, PSmooth::kDefaultV,
            PHold::kDefaultV, PRange::kDefaultV, POutGain::kDefaultV, PWet::kDefaultV,
            static_cast<float>(PSideStereoMode::kDefaultI), static_cast<float>(PSideStereoSwap::kDefaultI),
            PSideStereoLink::kDefaultV,
            PSideStereoWet1::kDefaultV, PSideStereoWet2::kDefaultV,
            static_cast<float>(POversample::kDefaultI), PLookAhead::kDefaultV,
            static_cast<float>(PCompON::kDefaultI), static_cast<float>(PCompDelta::kDefaultI)
        };

        void parameterChanged(const juce::String &parameter_ID, float new_value) override;
    };
} // zlp
