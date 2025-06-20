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
#include "../dsp/over_sample/over_sample.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlp {
    class CompressController final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;
        static constexpr size_t kAvgAnalyzerPointNum = 120;

        explicit CompressController(juce::AudioProcessor &processor);

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(std::array<float *, 2>main_pointers, std::array<float *, 2> side_pointers, size_t num_samples);

        auto &getMagAnalyzer() { return mag_analyzer_; }

        auto &getMagAvgAnalyzer() { return mag_avg_analyzer_; }

        auto &getComputer() { return computer_; }

        auto &getTracker() { return tracker_; }

        auto &getFollower() { return follower_; }

        void setCompStyle(const zldsp::compressor::Style style) {
            comp_style_.store(style, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setHoldLength(const float millisecond) {
            hold_length_.store(millisecond * 1e-3f, std::memory_order::relaxed);
            to_update_hold_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setRange(const float db) {
            range_.store(db, std::memory_order::relaxed);
            to_update_range_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setOutputGain(const float db) {
            output_gain_db_.store(db, std::memory_order::relaxed);
            to_update_output_gain_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setWet(const float percent) {
            wet_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setWet1(const float percent) {
            wet1_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setWet2(const float percent) {
            wet2_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_wet_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setMagAnalyzerOn(const bool f) {
            mag_analyzer_on_.store(f, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setSpecAnalyzerOn(const bool f) {
            spec_analyzer_on_.store(f, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setStereoModeIsMidSide(const bool f) {
            stereo_mode_.store(static_cast<int>(f), std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setStereoSwap(const bool f) {
            stereo_swap_.store(f, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setStereoLink(const float percent) {
            stereo_link_.store(1.f - percent * 0.005f, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setIsON(const bool is_on) {
            is_on_.store(is_on, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setIsDelta(const bool is_delta) {
            is_delta_.store(is_delta, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setOversampleIdx(const int idx) {
            oversample_idx_.store(idx, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setLookahead(const float x) {
            lookahead_delay_length_.store(x * 0.001f, std::memory_order::relaxed);
            to_update_lookahead_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

    private:
        juce::AudioProcessor &processor_ref_;
        juce::dsp::ProcessSpec main_spec_{48000.0, 512, 2};
        std::array<kfr::univector<float>, 2> pre_buffer_, post_buffer_;
        std::array<float *, 2> pre_pointers_{}, post_pointers_{};
        // global parameter update flag
        std::atomic<bool> to_update_{true};
        // on and delta
        std::atomic<bool> is_on_{true}, is_delta_{false};
        bool c_is_on_{true}, c_is_delta_{false};
        // magnitude analyzer
        std::atomic<bool> mag_analyzer_on_{true}, spec_analyzer_on_{false};
        bool c_mag_analyzer_on_{true}, c_spec_analyzer_on_{false};
        zldsp::analyzer::MagReductionAnalyzer<float, kAnalyzerPointNum> mag_analyzer_;
        zldsp::analyzer::MultipleMagAvgAnalyzer<float, 2, kAvgAnalyzerPointNum> mag_avg_analyzer_;
        // stereo control
        std::atomic<int> stereo_mode_{0};
        int c_stereo_mode_{0};
        std::atomic<float> stereo_link_{1.};
        float c_stereo_link_{1.};
        std::atomic<bool> stereo_swap_{false};
        bool c_stereo_swap_{false};
        // compressor style
        std::atomic<zldsp::compressor::Style> comp_style_{zldsp::compressor::Style::kClean};
        zldsp::compressor::Style c_comp_style_{zldsp::compressor::Style::kClean};
        // wet
        std::atomic<bool> to_update_wet_{true};
        std::atomic<float> wet_{1.0}, wet1_{1.0}, wet2_{1.0};
        float c_wet1_{1.0}, c_wet2_{1.0};
        // oversample
        std::atomic<int> oversample_idx_{0};
        int c_oversample_idx_{-1};
        juce::AudioBuffer<float> oversampled_side_buffer_{};

        zldsp::oversample::OverSampler<float, 1> over_sampler2_;
        zldsp::oversample::OverSampler<float, 2> over_sampler4_;
        zldsp::oversample::OverSampler<float, 3> over_sampler8_;

        zldsp::delay::IntegerDelay<float> oversample_delay_{};
        // lookahead
        std::atomic<float> lookahead_delay_length_{0.f};
        std::atomic<bool> to_update_lookahead_{false};
        bool is_lookahead_nonzero{false};
        zldsp::delay::IntegerDelay<float> lookahead_delay_{};
        // pdc
        std::atomic<int> pdc_{0};
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
        // vocal compressors
        std::array<zldsp::compressor::VocalCompressor<float>, 2> vocal_comps_ = {
            zldsp::compressor::VocalCompressor<float>{computer_[0], tracker_[0], follower_[0]},
            zldsp::compressor::VocalCompressor<float>{computer_[1], tracker_[1], follower_[1]}
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

        void prepareBuffer();

        void processBuffer(float * __restrict main_buffer0, float * __restrict main_buffer1,
                           float * __restrict side_buffer0, float * __restrict side_buffer1,
                           size_t num_samples);

        void processSideBufferClean(float * __restrict buffer0, float * __restrict buffer1, size_t num_samples);

        void processSideBufferClassic(float * __restrict buffer0, float * __restrict buffer1, size_t num_samples);

        void processSideBufferOptical(float * __restrict buffer0, float * __restrict buffer1, size_t num_samples);

        void processSideBufferVocal(float * __restrict buffer0, float * __restrict buffer1, size_t num_samples);

        void handleAsyncUpdate() override;
    };
} // zlDSP
