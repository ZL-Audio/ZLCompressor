// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "zlp_definitions.hpp"
#include "compress_controller.hpp"

namespace zlp {
    class CompressAttach final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit CompressAttach(juce::AudioProcessor& processor,
                                juce::AudioProcessorValueTreeState& parameters,
                                CompressController& controller);

        ~CompressAttach() override;

    private:
        juce::AudioProcessor& processor_ref_;
        juce::AudioProcessorValueTreeState& parameters_ref_;
        CompressController& controller_ref_;

        zldsp::compressor::CompressionComputer<float, true>& compression_computer_ref_;
        zldsp::compressor::ExpansionComputer<float, true>& expansion_computer_ref_;
        zldsp::compressor::InflationComputer<float, true>& inflation_computer_ref_;
        zldsp::compressor::PSFollower<float>& follower_ref_;

        constexpr static std::array kIDs{
            PCompStyle::kID, PCompDirection::kID,
            PThreshold::kID, PRatio::kID, PKneeW::kID, PCurve::kID, PFloor::kID,
            PAttack::kID, PRelease::kID, PPump::kID, PSmooth::kID,
            PHold::kID, PRange::kID, POutGain::kID, PWet::kID,
            PSideStereoMode::kID, PSideStereoSwap::kID,
            PSideStereoLink::kID,
            PSideStereoWet1::kID, PSideStereoWet2::kID,
            PClipperDrive::kID,
            POversample::kID, PLookAhead::kID,
            PCompON::kID, PCompDelta::kID,
            PRMSON::kID, PRMSLength::kID, PRMSSpeed::kID, PRMSMix::kID,
            PRangeINF::kID
        };

        void parameterChanged(const juce::String& parameter_ID, float value) override;
    };
}
