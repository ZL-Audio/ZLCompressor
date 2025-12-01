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
    CompressAttach::CompressAttach(juce::AudioProcessor& processor,
                                   juce::AudioProcessorValueTreeState& parameters,
                                   CompressController& controller) :
        processor_ref_(processor),
        parameters_ref_(parameters),
        controller_ref_(controller),
        computer_ref_(controller.getComputer()[0]),
        follower_ref_(controller.getFollower()[0]) {
        juce::ignoreUnused(processor_ref_);
        for (size_t i = 0; i < kIDs.size(); ++i) {
            parameters_ref_.addParameterListener(kIDs[i], this);
            parameterChanged(kIDs[i], parameters.getRawParameterValue(kIDs[i])->load(std::memory_order::relaxed));
        }
    }

    CompressAttach::~CompressAttach() {
        for (auto& ID : kIDs) {
            parameters_ref_.removeParameterListener(ID, this);
        }
    }

    void CompressAttach::parameterChanged(const juce::String& parameter_ID, float value) {
        if (parameter_ID == PCompStyle::kID) {
            controller_ref_.setCompStyle(static_cast<zldsp::compressor::Style>(value));
        } else if (parameter_ID == PCompDirection::kID) {
            controller_ref_.setCompDownward(value < .5f);
        } else if (parameter_ID == PThreshold::kID) {
            computer_ref_.setThreshold(value);
        } else if (parameter_ID == PRatio::kID) {
            computer_ref_.setRatio(value);
        } else if (parameter_ID == PKneeW::kID) {
            computer_ref_.setKneeW(value);
        } else if (parameter_ID == PCurve::kID) {
            computer_ref_.setCurve(PCurve::formatV(value));
        } else if (parameter_ID == PAttack::kID) {
            controller_ref_.setAttack(value);
        } else if (parameter_ID == PRelease::kID) {
            controller_ref_.setRelease(value);
        } else if (parameter_ID == PPump::kID) {
            follower_ref_.setPumpPunch(PPump::formatV(value));
        } else if (parameter_ID == PSmooth::kID) {
            follower_ref_.setSmooth(PSmooth::formatV(value));
        } else if (parameter_ID == PHold::kID) {
            controller_ref_.setHoldLength(value);
        } else if (parameter_ID == PRange::kID) {
            controller_ref_.setRange(value);
        } else if (parameter_ID == POutGain::kID) {
            controller_ref_.setOutputGain(value);
        } else if (parameter_ID == PWet::kID) {
            controller_ref_.setWet(value);
        } else if (parameter_ID == PSideStereoMode::kID) {
            controller_ref_.setStereoMode(static_cast<int>(std::round(value)));
        } else if (parameter_ID == PSideStereoSwap::kID) {
            controller_ref_.setStereoSwap(value > .5f);
        } else if (parameter_ID == PSideStereoLink::kID) {
            controller_ref_.setStereoLink(value);
        } else if (parameter_ID == PSideStereoWet1::kID) {
            controller_ref_.setWet1(value);
        } else if (parameter_ID == PSideStereoWet2::kID) {
            controller_ref_.setWet2(value);
        } else if (parameter_ID == PClipperDrive::kID) {
            controller_ref_.getClipper().setWet(value);
        } else if (parameter_ID == POversample::kID) {
            controller_ref_.setOversampleIdx(static_cast<int>(value));
        } else if (parameter_ID == PLookAhead::kID) {
            controller_ref_.setLookahead(value);
        } else if (parameter_ID == PCompON::kID) {
            controller_ref_.setIsON(value > .5f);
        } else if (parameter_ID == PCompDelta::kID) {
            controller_ref_.setIsDelta(value > .5f);
        } else if (parameter_ID == PRMSON::kID) {
            controller_ref_.setRMSOn(value > .5f);
        } else if (parameter_ID == PRMSLength::kID) {
            controller_ref_.setRMSLength(value);
        } else if (parameter_ID == PRMSSpeed::kID) {
            controller_ref_.setRMSSpeed(value);
        } else if (parameter_ID == PRMSMix::kID) {
            controller_ref_.setRMSMix(value);
        }
    }
}
