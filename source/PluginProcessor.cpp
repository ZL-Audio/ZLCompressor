// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output 1", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output 2", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr,
                 juce::Identifier("Parameters"),
                 zlDSP::getParameterLayout()) {
}

PluginProcessor::~PluginProcessor() {
}

//==============================================================================
const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void PluginProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    const juce::dsp::ProcessSpec spec{
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };
    doubleBuffer.setSize(4, samplesPerBlock);
    controller.prepare(spec);
}

void PluginProcessor::releaseResources() {
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }
    const auto auxOut = layouts.getChannelSet(false, 1);
    if (!auxOut.isDisabled() && auxOut != juce::AudioChannelSet::stereo()) {
        return false;
    }
    return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    doubleBuffer.setSize(juce::jmin(4, buffer.getNumChannels()),
                         buffer.getNumSamples(),
                         false, false, true);
    for (int chan = 0; chan < doubleBuffer.getNumChannels(); ++chan) {
        auto *dest = doubleBuffer.getWritePointer(chan);
        auto *src = buffer.getReadPointer(chan);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            dest[i] = static_cast<double>(src[i]);
        }
    }
    controller.process(doubleBuffer);
    for (int chan = 0; chan < doubleBuffer.getNumChannels(); ++chan) {
        auto *dest = buffer.getWritePointer(chan);
        auto *src = doubleBuffer.getReadPointer(chan);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            dest[i] = static_cast<float>(src[i]);
        }
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<double> &buffer,
                                   juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    controller.process(buffer);
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    return new PluginEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    auto tempTree = juce::ValueTree("ZLSplitterParaState");
    tempTree.appendChild(parameters.copyState(), nullptr);
    // tempTree.appendChild(parametersNA.copyState(), nullptr);
    const std::unique_ptr<juce::XmlElement> xml(tempTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName("ZLSplitterParaState")) {
        auto tempTree = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(tempTree.getChildWithName(parameters.state.getType()));
        // parametersNA.replaceState(tempTree.getChildWithName(parametersNA.state.getType()));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new PluginProcessor();
}
