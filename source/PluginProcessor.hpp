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

#include "dsp/dsp.hpp"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor {
public:
    // zlState::DummyProcessor dummyProcessor;
    juce::AudioProcessorValueTreeState parameters;//, state;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

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

    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;

    void setStateInformation(const void *data, int sizeInBytes) override;

    bool supportsDoublePrecisionProcessing() const override { return true; }

    zldsp::Controller &getController() { return controller; }

private:
    zldsp::Controller controller;

    juce::AudioBuffer<double> doubleBuffer;

    enum ChannelLayout {
        main1aux0, main1aux1, main1aux2,
        main2aux0, main2aux1, main2aux2,
        invalid
    };
    ChannelLayout channelLayout{invalid};

    void doubleBufferCopyFrom(int destChan, const juce::AudioBuffer<float> &buffer, int srcChan);

    void doubleBufferCopyTo(int srcChan, juce::AudioBuffer<float> &buffer, int destChan) const;

    void doubleBufferCopyFrom(int destChan, const juce::AudioBuffer<double> &buffer, int srcChan);

    void doubleBufferCopyTo(int srcChan, juce::AudioBuffer<double> &buffer, int destChan) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
