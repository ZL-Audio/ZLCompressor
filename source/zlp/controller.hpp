// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../dsp/compressor/compressor.hpp"
#include "../dsp/gain/gain.hpp"
#include "../dsp/splitter/splitter.hpp"
#include "../dsp/mag_analyzer/mag_analyzer.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlp {
    class Controller final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;
        static constexpr size_t kAvgAnalyzerPointNum = 120;

        explicit Controller(juce::AudioProcessor &processor);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<double> &buffer);

        auto &getMagAnalyzer() { return mag_analyzer_; }

        auto &getMagAvgAnalyzer() { return mag_avg_analyzer_; }

        auto &getComputer() { return computer_; }

        auto &getTracker() { return tracker_; }

        auto &getFollower() { return follower_; }

        void setCompStyle(const zldsp::compressor::Style style) {
            comp_style_.store(style);
        }

    private:
        juce::AudioProcessor &processor_ref_;
        juce::dsp::ProcessSpec main_spec_{48000.0, 512, 2};
        juce::AudioBuffer<double> pre_buffer_;
        std::array<double *, 2> pre_pointers_{}, main_pointers_{};

        zldsp::analyzer::MagReductionAnalyzer<double, kAnalyzerPointNum> mag_analyzer_;
        zldsp::analyzer::MultipleMagAvgAnalyzer<double, 2, kAvgAnalyzerPointNum> mag_avg_analyzer_;

        std::atomic<bool> ext_side_chain_{false};
        bool c_ext_side_chain_{false};

        std::atomic<int> stereo_mode_{0};
        int c_stereo_mode_{0};

        std::atomic<double> stereo_link_{0.};
        double c_stereo_link_{1.};

        std::atomic<bool> side_swap_{false};

        std::atomic<zldsp::compressor::Style> comp_style_{zldsp::compressor::Style::kClean};
        zldsp::compressor::Style c_comp_style_{zldsp::compressor::Style::kClean};

        std::atomic<bool> to_update_wet_{true};
        std::atomic<double> wet_{1.0}, wet1_{1.0}, wet2_{1.0};
        double c_wet1_{1.0}, c_wet2_{1.0};

        std::atomic<int> oversample_idx_{0};
        int c_oversample_idx_{-1};
        size_t c_oversample_stage_idx_{0};
        std::atomic<int> oversample_latency_{0};
        juce::AudioBuffer<double> oversampled_side_buffer_{};
        std::array<juce::dsp::Oversampling<double>, 3> oversample_stages_main_{
            juce::dsp::Oversampling<double>(2, 1,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 2,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 3,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
        };
        std::array<juce::dsp::Oversampling<double>, 3> oversample_stages_side_{
            juce::dsp::Oversampling<double>(2, 1,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 2,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<double>(2, 3,
                                            juce::dsp::Oversampling<double>::filterHalfBandFIREquiripple, true, true),
        };

        // computer, trackers and followers
        std::array<zldsp::compressor::KneeComputer<double, true>, 2> computer_{};
        std::array<zldsp::compressor::RMSTracker<double>, 2> tracker_{};
        std::array<zldsp::compressor::PSFollower<double, true, true>, 2> follower_{};
        // clean compressors
        std::array<zldsp::compressor::CleanCompressor<double>, 2> clean_comps_ = {
            zldsp::compressor::CleanCompressor<double>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::CleanCompressor<double>{computer_[1], tracker_[1], follower_[1]}
        };
        // classic compressors
        std::array<zldsp::compressor::ClassicCompressor<double>, 2> classic_comps_ = {
            zldsp::compressor::ClassicCompressor<double>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::ClassicCompressor<double>{computer_[1], tracker_[1], follower_[1]}
        };
        // optical compressors
        std::array<zldsp::compressor::OpticalCompressor<double>, 2> optical_comps_ = {
            zldsp::compressor::OpticalCompressor<double>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::OpticalCompressor<double>{computer_[1], tracker_[1], follower_[1]}
        };

        zldsp::gain::Gain<double> output_gain_{};

        void prepareBuffer();

        void processBuffer(double *main_buffer1, double *main_buffer2,
                           double *side_buffer1, double *side_buffer2,
                           size_t num_samples);

        void processSideBufferClean(double *buffer1, double *buffer2, size_t num_samples);

        void processSideBufferClassic(double *buffer1, double *buffer2, size_t num_samples);

        void processSideBufferOptical(double *buffer1, double *buffer2, size_t num_samples);

        void handleAsyncUpdate() override;
    };
} // zlDSP
