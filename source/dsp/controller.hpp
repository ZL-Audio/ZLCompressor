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

namespace zldsp {
    class Controller : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;
        static constexpr size_t kAvgAnalyzerPointNum = 120;

        explicit Controller(juce::AudioProcessor &processor);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<double> &buffer);

        zlMagAnalyzer::MagReductionAnalyzer<double, kAnalyzerPointNum> &getMagAnalyzer() { return mag_analyzer; }

        zlMagAnalyzer::MultipleMagAvgAnalyzer<double, 2, kAvgAnalyzerPointNum> &getMagAvgAnalyzer() {
            return mag_avg_analyzer;
        }

    private:
        juce::AudioProcessor &processor_ref;
        juce::dsp::ProcessSpec main_spec{48000.0, 512, 2};
        juce::AudioBuffer<double> pre_buffer;

        zlMagAnalyzer::MagReductionAnalyzer<double, kAnalyzerPointNum> mag_analyzer;
        zlMagAnalyzer::MultipleMagAvgAnalyzer<double, 2, kAvgAnalyzerPointNum> mag_avg_analyzer;

        std::atomic<bool> ext_side_chain{false};

        std::atomic<int> stereo_mode{0};
        int c_stereo_mode{0};
        zlSplitter::MSSplitter<double> ms_splitter;

        std::atomic<double> link{0.};
        double c_link{1.};

        std::atomic<bool> side_swap{false};

        std::atomic<compressor::style> style{compressor::style::clean};

        std::atomic<double> wet{1.0}, wet1{1.0}, wet2{1.0};
        double c_wet1{1.0}, c_wet2{1.0};

        std::atomic<int> oversample_idx{0};
        int c_oversample_idx{-1};
        size_t c_oversample_stage_idx{0};
        std::atomic<int> oversample_latency{0};
        std::array<juce::dsp::Oversampling<double>, 3> oversample_stages_main{
            juce::dsp::Oversampling<double>(2, 1,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 2,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 3,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
        };
        std::array<juce::dsp::Oversampling<double>, 3> oversample_stages_side{
            juce::dsp::Oversampling<double>(2, 1,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 2,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 3,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
        };

        // computer, trackers and followers
        compressor::KneeComputer<double, true> computer{};
        compressor::RMSTracker<double, true> tracker1{}, tracker2{};
        compressor::PSFollower<double, true, true> follower1{}, follower2{};
        // clean compressors
        std::array<compressor::CleanCompressor<double, true, true, true, true>, 2> cleans = {
            compressor::CleanCompressor<double, true, true, true, true>{computer, tracker1, follower1},
            compressor::CleanCompressor<double, true, true, true, true>{computer, tracker2, follower2}
        };

        void prepareBuffer();

        void processBuffer(double *main_buffer1, double *main_buffer2,
                           double *side_buffer1, double *side_buffer2,
                           size_t num_samples);

        void processSideBufferClean(double *buffer1, double *buffer2, size_t num_samples);

        void processSideBufferClassic(double *buffer1, double *buffer2, size_t num_samples);

        void processSideBufferOptical(double *buffer1, double *buffer2, size_t num_samples);

        void processSideBufferBus(double *buffer1, double *buffer2, size_t num_samples);

        void handleAsyncUpdate() override;
    };
} // zlDSP
