// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "compress_attach.hpp"

namespace zlp {
    CompressAttach::CompressAttach(juce::AudioProcessor &processor,
                                   juce::AudioProcessorValueTreeState &parameters,
                                   CompressorController &controller)
        : processor_ref_(processor),
          parameters_ref_(parameters),
          controller_ref_(controller),
          computer_ref_(controller.getComputer()[0]),
          follower_ref_(controller.getFollower()[0]) {
        juce::ignoreUnused(processor_ref_);
        for (auto &ID: kIDs) {
            parameters_ref_.addParameterListener(ID, this);
        }
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameterChanged(kIDs[i], kDefaultVs[i]);
        }
    }

    CompressAttach::~CompressAttach() {
        for (auto &ID: kIDs) {
            parameters_ref_.removeParameterListener(ID, this);
        }
    }

    void CompressAttach::parameterChanged(const juce::String &parameter_ID, float new_value) {
        if (parameter_ID == PCompStyle::kID) {
            controller_ref_.setCompStyle(static_cast<zldsp::compressor::Style>(new_value));
        } else if (parameter_ID == PThreshold::kID) {
            computer_ref_.setThreshold(new_value);
        } else if (parameter_ID == PRatio::kID) {
            computer_ref_.setRatio(new_value);
        } else if (parameter_ID == PKneeW::kID) {
            computer_ref_.setKneeW(new_value);
        } else if (parameter_ID == PCurve::kID) {
            computer_ref_.setCurve(PCurve::formatV(new_value));
        } else if (parameter_ID == PAttack::kID) {
            follower_ref_.setAttack(new_value);
        } else if (parameter_ID == PRelease::kID) {
            follower_ref_.setRelease(new_value);
        } else if (parameter_ID == PPump::kID) {
            const auto x = PPump::formatV(new_value);
            follower_ref_.setPumpPunch(x);
        } else if (parameter_ID == PSmooth::kID) {
            const auto x = PSmooth::formatV(new_value);
            follower_ref_.setSmooth(x);
        } else if (parameter_ID == PHold::kID) {
            controller_ref_.setHoldLength(new_value);
        } else if (parameter_ID == PRange::kID) {
            controller_ref_.setRange(new_value);
        } else if (parameter_ID == POutGain::kID) {
            controller_ref_.setOutputGain(new_value);
        } else if (parameter_ID == PWet::kID) {
            controller_ref_.setWet(new_value);
        } else if (parameter_ID == PSideStereoMode::kID) {
            controller_ref_.setStereoModeIsMidSide(new_value > .5f);
        } else if (parameter_ID == PSideStereoSwap::kID) {
            controller_ref_.setStereoSwap(new_value > .5f);
        } else if (parameter_ID == PSideStereoLink::kID) {
            controller_ref_.setStereoLink(new_value);
        } else if (parameter_ID == PSideStereoWet1::kID) {
            controller_ref_.setWet1(new_value);
        } else if (parameter_ID == PSideStereoWet2::kID) {
            controller_ref_.setWet2(new_value);
        } else if (parameter_ID == POversample::kID) {
            controller_ref_.setOversampleIdx(static_cast<int>(new_value));
        } else if (parameter_ID == PCompON::kID) {
            controller_ref_.setIsON(new_value > .5f);
        } else if (parameter_ID == PCompDelta::kID) {
            controller_ref_.setIsDelta(new_value > .5f);
        }
    }
} // zlp
