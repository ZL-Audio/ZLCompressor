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
          .withInput("Aux", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
      ),
      dummy_processor_(),
      parameters_(*this, nullptr,
                  juce::Identifier("ZLCompressorParameters"),
                  zlp::getParameterLayout()),
      state_(dummy_processor_, nullptr,
             juce::Identifier("ZLCompressorState"),
             zlstate::getStateParameterLayout()),
      property_(state_),
      compressor_controller_(*this),
      compress_attach_(*this, parameters_, compressor_controller_) {
}

PluginProcessor::~PluginProcessor() = default;

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

void PluginProcessor::changeProgramName(int, const juce::String &) {
}

//==============================================================================
void PluginProcessor::prepareToPlay(const double sample_rate, const int samples_per_block) {
    // prepare to play
    const juce::dsp::ProcessSpec spec{
        sample_rate,
        static_cast<juce::uint32>(samples_per_block),
        2
    };
    float_buffer_.setSize(4, samples_per_block);
    float_buffer_.clear();
    float_side_pointers_[0] = float_buffer_.getWritePointer(2);
    float_side_pointers_[1] = float_buffer_.getWritePointer(3);
    double_buffer_.setSize(2, samples_per_block);
    double_side_pointers_[0] = double_buffer_.getWritePointer(0);
    double_side_pointers_[1] = double_buffer_.getWritePointer(1);
    double_buffer_.clear();
    compressor_controller_.prepare(spec);
    // determine current channel layout
    const auto *main_bus = getBus(true, 0);
    const auto *aux_bus = getBus(true, 1);
    channel_layout_ = ChannelLayout::kInvalid;
    if (main_bus == nullptr) {
        return;
    }
    if (main_bus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
        if (aux_bus == nullptr || !aux_bus->isEnabled()) {
            channel_layout_ = ChannelLayout::kMain1Aux0;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
            channel_layout_ = ChannelLayout::kMain1Aux1;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
            channel_layout_ = ChannelLayout::kMain1Aux2;
        }
    } else if (main_bus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
        if (aux_bus == nullptr || !aux_bus->isEnabled()) {
            channel_layout_ = ChannelLayout::kMain2Aux0;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::mono()) {
            channel_layout_ = ChannelLayout::kMain2Aux1;
        } else if (aux_bus->getCurrentLayout() == juce::AudioChannelSet::stereo()) {
            channel_layout_ = ChannelLayout::kMain2Aux2;
        }
    }
}

void PluginProcessor::releaseResources() {
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() &&
        (layouts.getChannelSet(true, 1).isDisabled() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::mono() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::stereo())) {
        return true;
    }
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono() &&
        (layouts.getChannelSet(true, 1).isDisabled() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::mono() ||
         layouts.getChannelSet(true, 1) == juce::AudioChannelSet::stereo())) {
        return true;
    }
    return false;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &) {
    juce::ScopedNoDenormals no_denormals;
    if (buffer.getNumSamples() == 0) return; // ignore empty blocks
    const auto c_use_ext_side = use_ext_side_.load(std::memory_order::relaxed);
    const auto buffer_size = static_cast<size_t>(buffer.getNumSamples());
    switch (channel_layout_) {
        case ChannelLayout::kMain1Aux0: {
            main_pointers_[0] = buffer.getWritePointer(0);
            main_pointers_[1] = float_buffer_.getWritePointer(1);
            zldsp::vector::copy(main_pointers_[1], main_pointers_[0], buffer_size);

            zldsp::vector::copy<double, float>(double_side_pointers_, main_pointers_, buffer_size);
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            break;
        }
        case ChannelLayout::kMain1Aux1: {
            main_pointers_[0] = buffer.getWritePointer(0);
            main_pointers_[1] = float_buffer_.getWritePointer(1);
            zldsp::vector::copy(main_pointers_[1], main_pointers_[0], buffer_size);

            if (c_use_ext_side) {
                zldsp::vector::copy(double_side_pointers_[0], buffer.getReadPointer(1), buffer_size);
                zldsp::vector::copy(double_side_pointers_[1], double_side_pointers_[0], buffer_size);
            } else {
                zldsp::vector::copy<double, float>(double_side_pointers_, main_pointers_, buffer_size);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            break;
        }
        case ChannelLayout::kMain1Aux2: {
            main_pointers_[0] = buffer.getWritePointer(0);
            main_pointers_[1] = float_buffer_.getWritePointer(1);
            zldsp::vector::copy(main_pointers_[1], main_pointers_[0], buffer_size);

            if (c_use_ext_side) {
                zldsp::vector::copy(double_side_pointers_[0], buffer.getReadPointer(1), buffer_size);
                zldsp::vector::copy(double_side_pointers_[1], buffer.getReadPointer(2), buffer_size);
            } else {
                zldsp::vector::copy<double, float>(double_side_pointers_, main_pointers_, buffer_size);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            break;
        }
        case ChannelLayout::kMain2Aux0: {
            main_pointers_[0] = buffer.getWritePointer(0);
            main_pointers_[1] = buffer.getWritePointer(1);

            zldsp::vector::copy<double, float>(double_side_pointers_, main_pointers_, buffer_size);
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            break;
        }
        case ChannelLayout::kMain2Aux1: {
            main_pointers_[0] = buffer.getWritePointer(0);
            main_pointers_[1] = buffer.getWritePointer(1);

            if (c_use_ext_side) {
                zldsp::vector::copy(double_side_pointers_[0], buffer.getWritePointer(2), buffer_size);
                zldsp::vector::copy(double_side_pointers_[1], double_side_pointers_[0], buffer_size);
            } else {
                zldsp::vector::copy<double, float>(double_side_pointers_, main_pointers_, buffer_size);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            break;
        }
        case ChannelLayout::kMain2Aux2: {
            main_pointers_[0] = buffer.getWritePointer(0);
            main_pointers_[1] = buffer.getWritePointer(1);

            if (c_use_ext_side) {
                zldsp::vector::copy(double_side_pointers_[0], buffer.getWritePointer(2), buffer_size);
                zldsp::vector::copy(double_side_pointers_[1], buffer.getWritePointer(3), buffer_size);
            } else {
                zldsp::vector::copy<double, float>(double_side_pointers_, main_pointers_, buffer_size);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            break;
        }
        case ChannelLayout::kInvalid: {
        }
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<double> &buffer, juce::MidiBuffer &) {
    juce::ScopedNoDenormals no_denormals;
    if (buffer.getNumSamples() == 0) return; // ignore empty blocks
    const auto c_use_ext_side = use_ext_side_.load(std::memory_order::relaxed);
    const auto buffer_size = static_cast<size_t>(buffer.getNumSamples());
    switch (channel_layout_) {
        case ChannelLayout::kMain1Aux0: {
            zldsp::vector::copy(main_pointers_[0], buffer.getWritePointer(0), buffer_size);
            zldsp::vector::copy(main_pointers_[1], main_pointers_[0], buffer_size);

            double_side_pointers_[0] = buffer.getWritePointer(0);
            zldsp::vector::copy(double_side_pointers_[1], double_side_pointers_[0], buffer_size);
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], buffer_size);
            break;
        }
        case ChannelLayout::kMain1Aux1: {
            zldsp::vector::copy(main_pointers_[0], buffer.getWritePointer(0), buffer_size);
            zldsp::vector::copy(main_pointers_[1], main_pointers_[0], buffer_size);

            double_side_pointers_[0] = c_use_ext_side ? buffer.getWritePointer(1) : buffer.getWritePointer(0);
            zldsp::vector::copy(double_side_pointers_[1], double_side_pointers_[0], buffer_size);
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], buffer_size);
            break;
        }
        case ChannelLayout::kMain1Aux2: {
            zldsp::vector::copy(main_pointers_[0], buffer.getWritePointer(0), buffer_size);
            zldsp::vector::copy(main_pointers_[1], main_pointers_[0], buffer_size);

            if (c_use_ext_side) {
                double_side_pointers_[0] = buffer.getWritePointer(1);
                double_side_pointers_[1] = buffer.getWritePointer(2);
            } else {
                double_side_pointers_[0] = buffer.getWritePointer(0);
                zldsp::vector::copy(double_side_pointers_[1], double_side_pointers_[0], buffer_size);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], buffer_size);
            break;
        }
        case ChannelLayout::kMain2Aux0: {
            zldsp::vector::copy(main_pointers_[0], buffer.getWritePointer(0), buffer_size);
            zldsp::vector::copy(main_pointers_[1], buffer.getWritePointer(1), buffer_size);

            double_side_pointers_[0] = buffer.getWritePointer(0);
            double_side_pointers_[1] = buffer.getWritePointer(1);
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(1), main_pointers_[1], buffer_size);
            break;
        }
        case ChannelLayout::kMain2Aux1: {
            zldsp::vector::copy(main_pointers_[0], buffer.getWritePointer(0), buffer_size);
            zldsp::vector::copy(main_pointers_[1], buffer.getWritePointer(1), buffer_size);

            if (c_use_ext_side) {
                double_side_pointers_[0] = buffer.getWritePointer(2);
                zldsp::vector::copy(double_side_pointers_[1], double_side_pointers_[0], buffer_size);
            } else {
                double_side_pointers_[0] = buffer.getWritePointer(0);
                double_side_pointers_[1] = buffer.getWritePointer(1);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(1), main_pointers_[1], buffer_size);
            break;
        }
        case ChannelLayout::kMain2Aux2: {
            zldsp::vector::copy(main_pointers_[0], buffer.getWritePointer(0), buffer_size);
            zldsp::vector::copy(main_pointers_[1], buffer.getWritePointer(1), buffer_size);

            if (c_use_ext_side) {
                double_side_pointers_[0] = buffer.getWritePointer(2);
                double_side_pointers_[1] = buffer.getWritePointer(3);
            } else {
                double_side_pointers_[0] = buffer.getWritePointer(0);
                double_side_pointers_[1] = buffer.getWritePointer(1);
            }
            // process side eq
            zldsp::vector::copy<float, double>(float_side_pointers_, double_side_pointers_, buffer_size);

            compressor_controller_.process(main_pointers_, float_side_pointers_, buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(0), main_pointers_[0], buffer_size);
            zldsp::vector::copy(buffer.getWritePointer(1), main_pointers_[1], buffer_size);
            break;
        }
        case ChannelLayout::kInvalid: {
        }
    }
}

bool PluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    // return new juce::GenericAudioProcessorEditor(*this);
    return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &dest_data) {
    auto temp_tree = juce::ValueTree("ZLEqualizerParaState");
    temp_tree.appendChild(parameters_.copyState(), nullptr);
    // temp_tree.appendChild(parameters_NA_.copyState(), nullptr);
    const std::unique_ptr<juce::XmlElement> xml(temp_tree.createXml());
    copyXmlToBinary(*xml, dest_data);
}

void PluginProcessor::setStateInformation(const void *data, int size_in_bytes) {
    std::unique_ptr<juce::XmlElement> xml_state(getXmlFromBinary(data, size_in_bytes));
    if (xml_state != nullptr && xml_state->hasTagName("ZLEqualizerParaState")) {
        const auto temp_tree = juce::ValueTree::fromXml(*xml_state);
        parameters_.replaceState(temp_tree.getChildWithName(parameters_.state.getType()));
        // parameters_NA_.replaceState(temp_tree.getChildWithName(parameters_NA_.state.getType()));
    }
}

juce::AudioProcessor *JUCE_CALLTYPE

createPluginFilter() {
    return new PluginProcessor();
}
