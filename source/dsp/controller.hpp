// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "compressor/compressor.hpp"
#include "gain/gain.hpp"
#include "splitter/splitter.hpp"
#include "mag_analyzer/mag_analyzer.hpp"

namespace zlDSP {
    class Controller : private juce::AsyncUpdater {
    public:
        static constexpr size_t analyzerPointNum = 251;
        static constexpr size_t avgAnalyzerPointNum = 120;

        explicit Controller(juce::AudioProcessor &processor);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<double> &buffer);

        zlMagAnalyzer::MagReductionAnalyzer<double, analyzerPointNum> &getMagAnalyzer() { return magAnalyzer; }

        zlMagAnalyzer::MultipleMagAvgAnalyzer<double, 2, avgAnalyzerPointNum> &getMagAvgAnalyzer() { return magAvgAnalyzer; }

    private:
        juce::AudioProcessor &processorRef;
        juce::dsp::ProcessSpec mainSpec;
        juce::AudioBuffer<double> preBuffer;

        zlMagAnalyzer::MagReductionAnalyzer<double, analyzerPointNum> magAnalyzer;
        zlMagAnalyzer::MultipleMagAvgAnalyzer<double, 2, avgAnalyzerPointNum> magAvgAnalyzer;

        std::atomic<int> stereoMode{0};
        int currentStereoMode{0};
        zlSplitter::LRSplitter<double> mainLRSplitter, sideLRSplitter;
        zlSplitter::MSSplitter<double> mainMSSplitter, sideMSSplitter;

        std::atomic<int> oversampleIdx{0};
        int currentOversampleIdx{0};
        std::atomic<int> oversampleLatency{0};
        std::array<juce::dsp::Oversampling<double>, 3> oversampleStages{
            juce::dsp::Oversampling<double>(2, 1,
                juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 2,
                juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 3,
                juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
        };

        juce::dsp::Compressor<double> compressor;

        void prepareBuffer();

        void processSideBuffer(juce::AudioBuffer<double> &buffer);

        void handleAsyncUpdate() override;
    };
} // zlDSP
