// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "zlp/zlp.hpp"
#include "state/state.hpp"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor {
public:
    zlstate::DummyProcessor dummy_processor_;
    juce::AudioProcessorValueTreeState parameters_;
    juce::AudioProcessorValueTreeState state_;
    zlstate::Property property_;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sample_rate, int samples_per_block) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    void processBlock(juce::AudioBuffer<double> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;

    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int, const juce::String &) override;

    void getStateInformation(juce::MemoryBlock &dest_data) override;

    void setStateInformation(const void *data, int size_in_bytes) override;

    bool supportsDoublePrecisionProcessing() const override { return true; }

    void setUseExternalSide(const bool use_ext_side) {
        use_ext_side_.store(use_ext_side, std::memory_order::relaxed);
    }

    inline zlp::CompressorController &getController() {
        return compressor_controller_;
    }

private:
    zlp::CompressorController compressor_controller_;
    zlp::CompressAttach compress_attach_;
    juce::AudioBuffer<float> float_buffer_;
    juce::AudioBuffer<double> double_buffer_;
    std::array<float *, 2> main_pointers_, float_side_pointers_;
    std::array<double *, 2> double_side_pointers_;

    enum ChannelLayout {
        kMain1Aux0, kMain1Aux1, kMain1Aux2,
        kMain2Aux0, kMain2Aux1, kMain2Aux2,
        kInvalid
    };

    std::atomic<bool> use_ext_side_{false};
    ChannelLayout channel_layout_{kInvalid};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
