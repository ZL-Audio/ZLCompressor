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
    Controller::Controller(juce::AudioProcessor &processor)
        : processorRef(processor) {

    }

    void Controller::prepare(const juce::dsp::ProcessSpec &spec) {
        mainSpec = spec;
        magAnalyzer.prepare(spec);
        mainLRSplitter.prepare(spec);
        mainMSSplitter.prepare(spec);
        sideLRSplitter.prepare(spec);
        sideMSSplitter.prepare(spec);
        compressor.prepare(spec);
        compressor.setThreshold(-18.0);
        compressor.setRatio(2.0);
        compressor.setAttack(50.0);
        compressor.setRelease(100.0);

        preBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
    }

    void Controller::prepareBuffer() {
        {
            currentStereoMode = stereoMode.load();
        }
        {
            if (oversampleIdx.load() != currentOversampleIdx) {
                currentOversampleIdx = oversampleIdx.load();
                if (currentOversampleIdx > 0) {
                    oversampleLatency.store(
                        static_cast<int>(
                            oversampleStages[static_cast<size_t>(currentOversampleIdx - 1)].getLatencyInSamples()));
                } else {
                    oversampleLatency.store(0);
                }
                // TODO: prepare updated new sample-rates
            }
        }

    }

    void Controller::process(juce::AudioBuffer<double> &buffer) {
        prepareBuffer();
        juce::AudioBuffer<double> mainBuffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<double> sideBuffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        // stereo split the main/side buffer
        if (currentStereoMode == 0) {
            mainLRSplitter.split(mainBuffer);
            sideLRSplitter.split(sideBuffer);
        } else {
            mainMSSplitter.split(mainBuffer);
            sideMSSplitter.split(sideBuffer);
        }
        // up-sample side-buffer
        if (currentOversampleIdx == 0) {
            processSideBuffer(sideBuffer);
        } else {
            // over-sample the side-buffer
        }

        preBuffer.makeCopyOf(mainBuffer, true);
        juce::dsp::AudioBlock<double> block(mainBuffer);
        compressor.process(juce::dsp::ProcessContextReplacing<double>(block));
        magAnalyzer.process({preBuffer, mainBuffer});
        magAvgAnalyzer.process({preBuffer, mainBuffer});
    }

    void Controller::processSideBuffer(juce::AudioBuffer<double> &buffer) {
        // TODO: process (over-sampled) side buffer here
    }

    void Controller::handleAsyncUpdate() {
        processorRef.setLatencySamples(oversampleLatency.load());
    }
} // zlDSP
