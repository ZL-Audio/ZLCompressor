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
#include "../dsp/delay/delay.hpp"
#include "../dsp/mag_analyzer/mag_analyzer.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlp {
    class CompressorController final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;
        static constexpr size_t kAvgAnalyzerPointNum = 120;

        explicit CompressorController(juce::AudioProcessor &processor);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(std::array<float *, 2>main_pointers, std::array<float *, 2> side_pointers, size_t num_samples);

        auto &getMagAnalyzer() { return mag_analyzer_; }

        auto &getMagAvgAnalyzer() { return mag_avg_analyzer_; }

        auto &getComputer() { return computer_; }

        auto &getTracker() { return tracker_; }

        auto &getFollower() { return follower_; }

        void setCompStyle(const zldsp::compressor::Style style) {
            comp_style_.store(style, std::memory_order::relaxed);
        }

        void setOversampleIdx(const int idx) {
            oversample_idx_.store(idx, std::memory_order::relaxed);
        }

        void setHoldLength(const float millisecond) {
            hold_length_.store(millisecond * 1e-3f, std::memory_order::relaxed);
            to_update_hold_.store(true, std::memory_order::release);
        }

        void setRange(const float db) {
            range_.store(db, std::memory_order::relaxed);
            to_update_range_.store(true, std::memory_order::release);
        }

        void setOutputGain(const float db) {
            output_gain_db_.store(db, std::memory_order::relaxed);
            to_update_output_gain_.store(true, std::memory_order::release);
        }

        void setWet(const float percent) {
            wet_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
        }

        void setWet1(const float percent) {
            wet1_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
        }

        void setWet2(const float percent) {
            wet2_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
        }

        void setMagAnalyzerOn(const bool f) {
            mag_analyzer_on_.store(f, std::memory_order::relaxed);
        }

        void setSpecAnalyzerOn(const bool f) {
            spec_analyzer_on_.store(f, std::memory_order::relaxed);
        }

        void setStereoModeIsMidSide(const bool f) {
            stereo_mode_.store(static_cast<int>(f), std::memory_order::relaxed);
        }

        void setStereoSwap(const bool f) {
            stereo_swap_.store(f, std::memory_order::relaxed);
        }

        void setStereoLink(const float percent) {
            stereo_link_.store(1.f - percent * 0.005f, std::memory_order::relaxed);
        }

        void setIsON(const bool is_on) {
            is_on_.store(is_on, std::memory_order::relaxed);
        }

        void setIsDelta(const bool is_delta) {
            is_delta_.store(is_delta, std::memory_order::relaxed);
        }

    private:
        juce::AudioProcessor &processor_ref_;
        std::atomic<bool> is_on_{true}, is_delta_{false};

        std::atomic<bool> mag_analyzer_on_{true}, spec_analyzer_on_{false};
        bool c_mag_analyzer_on_{true}, c_spec_analyzer_on_{false};

        juce::dsp::ProcessSpec main_spec_{48000.0, 512, 2};
        std::array<kfr::univector<float>, 2> pre_buffer_, post_buffer_;
        std::array<float *, 2> pre_pointers_{}, post_pointers_{};

        zldsp::analyzer::MagReductionAnalyzer<float, kAnalyzerPointNum> mag_analyzer_;
        zldsp::analyzer::MultipleMagAvgAnalyzer<float, 2, kAvgAnalyzerPointNum> mag_avg_analyzer_;

        std::atomic<int> stereo_mode_{0};
        int c_stereo_mode_{0};

        std::atomic<float> stereo_link_{1.};
        float c_stereo_link_{1.};

        std::atomic<bool> stereo_swap_{false};

        std::atomic<zldsp::compressor::Style> comp_style_{zldsp::compressor::Style::kClean};
        zldsp::compressor::Style c_comp_style_{zldsp::compressor::Style::kClean};

        std::atomic<bool> to_update_wet_{true};
        std::atomic<float> wet_{1.0}, wet1_{1.0}, wet2_{1.0};
        float c_wet1_{1.0}, c_wet2_{1.0};

        std::atomic<int> oversample_idx_{0};
        int c_oversample_idx_{-1};
        size_t c_oversample_stage_idx_{0};
        std::atomic<int> oversample_latency_{0};
        juce::AudioBuffer<float> oversampled_side_buffer_{};
        std::array<juce::dsp::Oversampling<float>, 3> oversample_stages_main_{
            juce::dsp::Oversampling<float>(2, 1,
                                           juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<float>(2, 2,
                                           juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<float>(2, 3,
                                           juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true),
        };
        std::array<juce::dsp::Oversampling<float>, 3> oversample_stages_side_{
            juce::dsp::Oversampling<float>(2, 1,
                                           juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<float>(2, 2,
                                           juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true),
            juce::dsp::Oversampling<float>(2, 3,
                                           juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true),
        };

        // computer, trackers and followers
        std::array<zldsp::compressor::KneeComputer<float, true>, 2> computer_{};
        std::array<zldsp::compressor::RMSTracker<float>, 2> tracker_{};
        std::array<zldsp::compressor::PSFollower<float, true, true>, 2> follower_{};
        // clean compressors
        std::array<zldsp::compressor::CleanCompressor<float>, 2> clean_comps_ = {
            zldsp::compressor::CleanCompressor<float>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::CleanCompressor<float>{computer_[1], tracker_[1], follower_[1]}
        };
        // classic compressors
        std::array<zldsp::compressor::ClassicCompressor<float>, 2> classic_comps_ = {
            zldsp::compressor::ClassicCompressor<float>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::ClassicCompressor<float>{computer_[1], tracker_[1], follower_[1]}
        };
        // optical compressors
        std::array<zldsp::compressor::OpticalCompressor<float>, 2> optical_comps_ = {
            zldsp::compressor::OpticalCompressor<float>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::OpticalCompressor<float>{computer_[1], tracker_[1], follower_[1]}
        };

        std::atomic<bool> to_update_hold_{true};
        std::atomic<float> hold_length_{0.0};
        std::array<zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMin>, 2> hold_buffer_ = {
            zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMin>{},
            zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMin>{},
        };

        std::atomic<bool> to_update_range_{true};
        std::atomic<float> range_{80.f};
        float c_range_{80.f};

        std::atomic<bool> to_update_output_gain_{true};
        std::atomic<float> output_gain_db_{0.f};
        zldsp::gain::Gain<float> output_gain_{};

        zldsp::delay::IntegerDelay<float> oversample_delay_{};

        void prepareBuffer();

        void processBuffer(float *main_buffer0, float *main_buffer1,
                           float *side_buffer0, float *side_buffer1,
                           size_t num_samples);

        void processSideBufferClean(float *buffer0, float *buffer1, size_t num_samples);

        void processSideBufferClassic(float *buffer0, float *buffer1, size_t num_samples);

        void processSideBufferOptical(float *buffer0, float *buffer1, size_t num_samples);

        void handleAsyncUpdate() override;
    };
} // zlDSP
