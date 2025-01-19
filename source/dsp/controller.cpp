// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlDSP {
    void Controller::prepare(const juce::dsp::ProcessSpec &spec) {
        magAnalyzer.prepare(spec);
        compressor.prepare(spec);
        compressor.setThreshold(-20.0);
        compressor.setRatio(2.0);
        compressor.setAttack(50.0);
        compressor.setRelease(100.0);
    }

    void Controller::process(juce::AudioBuffer<double> &buffer) {
        juce::AudioBuffer<double> mainBuffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<double> sideBuffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        juce::ignoreUnused(sideBuffer);
        magAnalyzer.pushInBuffer(mainBuffer);
        juce::dsp::AudioBlock<double> block(mainBuffer);
        compressor.process(juce::dsp::ProcessContextReplacing<double>(block));
        magAnalyzer.pushOutBuffer(mainBuffer);
    }
} // zlDSP
