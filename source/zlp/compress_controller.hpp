// Copyright (C) 2026 - zsliu98
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
#include "../dsp/analyzer/mag_analyzer/mag_analyzer_sender.hpp"
#include "../dsp/over_sample/over_sample.hpp"
#include "../dsp/loudness/lufs_matcher.hpp"
#include "zlp_definitions.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace zlp {
    class CompressController final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;
        static constexpr size_t kAvgAnalyzerPointNum = 120;

        explicit CompressController(juce::AudioProcessor& processor);

        void prepare(double sample_rate, size_t max_num_samples);

        void process(std::array<float*, 2> main_pointers, std::array<float*, 2> side_pointers,
                     size_t num_samples, bool bypass);

        auto& getMagAnalyzerSender() { return mag_analyzer_sender_; }

        auto& getCompressionComputer() { return compression_computer_; }

        auto& getExpansionComputer() { return expansion_computer_; }

        auto& getInflationComputer() { return inflation_computer_; }

        auto& getTracker() { return rms_tracker_; }

        auto& getFollower() { return follower_; }

        auto& getClipper() { return clipper_; }

        void setCompDirection(const PCompDirection::Direction direction) {
            direction_.store(direction, std::memory_order::relaxed);
            to_update_style_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setCompStyle(const zldsp::compressor::Style style) {
            comp_style_.store(style, std::memory_order::relaxed);
            to_update_style_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setAttack(const float attack) {
            attack_.store(attack, std::memory_order::release);
            follower_[0].setAttack(attack);
            rms_follower_[0].setAttack(attack * rms_speed_.load(std::memory_order::acquire));
        }

        void setRelease(const float release) {
            release_.store(release, std::memory_order::release);
            follower_[0].setRelease(release);
            rms_follower_[0].setRelease(release * rms_speed_.load(std::memory_order::acquire));
        }

        void setRMSSpeed(const float speed) {
            rms_speed_.store(speed, std::memory_order::release);
            const auto c_attack = attack_.load(std::memory_order::acquire);
            const auto c_release = release_.load(std::memory_order::acquire);
            rms_follower_[0].setAttack(c_attack * speed);
            rms_follower_[0].setRelease(c_release * speed);
        }

        void setRMSOn(const bool use_rms) {
            use_rms_.store(use_rms, std::memory_order::relaxed);
            to_update_rms_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setRMSLength(const float millisecond) {
            const auto seconds = millisecond * 1e-3f;
            rms_tracker_[0].setMomentarySeconds(seconds);
            rms_tracker_[1].setMomentarySeconds(seconds);
            to_update_rms_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setRMSMix(const float percent) {
            rms_mix_.store(percent * 0.01f, std::memory_order::relaxed);
            to_update_rms_.store(true, std::memory_order::release);
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

        void setLUFSMatcherOn(const bool f) {
            lufs_matcher_on_.store(f, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        float getLUFSMatcherDiff() const {
            return lufs_matcher_.getDiff();
        }

        void setStereoMode(const int mode) {
            stereo_mode_.store(mode, std::memory_order::relaxed);
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
        juce::AudioProcessor& processor_ref_;
        double sample_rate_{48000.0};
        std::array<kfr::univector<float>, 2> pre_buffer_, post_buffer_;
        std::array<float*, 2> pre_pointers_{}, post_pointers_{};
        // global parameter update flag
        std::atomic<bool> to_update_{true};
        // on and delta
        std::atomic<bool> is_on_{true}, is_delta_{false};
        bool c_is_on_{true}, c_is_delta_{false};
        // magnitude analyzer
        std::atomic<bool> mag_analyzer_on_{true};
        bool c_mag_analyzer_on_{true};
        zldsp::analyzer::MagAnalyzerSender<float, 3> mag_analyzer_sender_;
        // lufs matcher
        std::atomic<bool> lufs_matcher_on_{false};
        bool c_lufs_matcher_on_{false};
        zldsp::loudness::LUFSMatcher<float, true> lufs_matcher_;
        // copy pre post flags
        bool c_copy_pre{false}, c_copy_post{false};
        // stereo control
        std::atomic<int> stereo_mode_{0};
        bool c_stereo_mode_is_midside{true}, c_stereo_mode_is_max{false};
        std::atomic<float> stereo_link_{1.};
        float c_stereo_link_{1.}, c_stereo_link_max_{1.f};
        std::atomic<bool> stereo_swap_{false};
        bool c_stereo_swap_{false};
        // compressor style
        std::atomic<PCompDirection::Direction> direction_{PCompDirection::kCompress};
        PCompDirection::Direction c_direction_{PCompDirection::kCompress};
        bool c_is_downward_{true};
        std::atomic<zldsp::compressor::Style> comp_style_{zldsp::compressor::Style::kClean};
        zldsp::compressor::Style c_comp_style_{zldsp::compressor::Style::kClean};
        std::atomic<bool> to_update_style_{false};
        // wet
        std::atomic<bool> to_update_wet_{true};
        std::atomic<float> wet_{1.0}, wet1_{1.0}, wet2_{1.0};
        float c_wet1_{1.0}, c_wet2_{1.0};
        // oversample
        std::atomic<int> oversample_idx_{0};
        int c_oversample_idx_{-1};
        // oversamplers
        zldsp::oversample::OverSampler<float, 1> over_sampler2_;
        zldsp::oversample::OverSampler<float, 2> over_sampler4_;
        zldsp::oversample::OverSampler<float, 3> over_sampler8_;
        zldsp::delay::IntegerDelay<float> oversample_delay_{};
        double oversample_sr_{48000.0};
        // lookahead
        std::atomic<float> lookahead_delay_length_{0.f};
        std::atomic<bool> to_update_lookahead_{false};
        bool is_lookahead_nonzero{false};
        zldsp::delay::IntegerDelay<float> lookahead_delay_{};
        // pdc
        std::atomic<int> pdc_{0};
        // computer, trackers and followers
        zldsp::compressor::CompressionComputer<float, true> compression_computer_{};
        zldsp::compressor::ExpansionComputer<float, true> expansion_computer_{};
        zldsp::compressor::InflationComputer<float, true> inflation_computer_{};
        std::array<zldsp::compressor::RMSTracker<float>, 2> rms_tracker_{};
        std::array<zldsp::compressor::PSFollower<float>, 2> follower_{};
        std::array<zldsp::compressor::PSFollower<float>, 2> rms_follower_{};
        // clean compressors
        std::array<zldsp::compressor::CleanCompressor<float>, 2> clean_comps_ = {
            zldsp::compressor::CleanCompressor<float>{},
            zldsp::compressor::CleanCompressor<float>{}
        };
        // classic compressors
        std::array<zldsp::compressor::ClassicCompressor<float>, 2> classic_comps_ = {
            zldsp::compressor::ClassicCompressor<float>{},
            zldsp::compressor::ClassicCompressor<float>{}
        };
        // optical compressors
        std::array<zldsp::compressor::OpticalCompressor<float>, 2> optical_comps_ = {
            zldsp::compressor::OpticalCompressor<float>{},
            zldsp::compressor::OpticalCompressor<float>{}
        };
        // vocal compressors
        std::array<zldsp::compressor::VocalCompressor<float>, 2> vocal_comps_ = {
            zldsp::compressor::VocalCompressor<float>{},
            zldsp::compressor::VocalCompressor<float>{}
        };
        // rms compressors
        std::atomic<bool> to_update_rms_{true};
        std::atomic<bool> use_rms_{false};
        bool c_use_rms_{false};
        std::atomic<float> rms_mix_{0.f};
        float c_rms_mix_{0.f};
        std::atomic<float> attack_{0.f}, release_{0.f}, rms_speed_{1.f};
        std::vector<float> rms_side_buffer0_, rms_side_buffer1_;
        std::array<zldsp::compressor::CleanCompressor<float>, 2> rms_comps_ = {
            zldsp::compressor::CleanCompressor<float>{},
            zldsp::compressor::CleanCompressor<float>{}
        };
        // hold
        std::atomic<bool> to_update_hold_{true};
        std::atomic<float> hold_length_{0.0};
        std::array<zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMin>, 2> hold_buffer_ = {
            zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMin>{},
            zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMin>{},
        };
        // range
        std::atomic<bool> to_update_range_{true};
        std::atomic<float> range_{80.f};
        float c_range_{80.f};
        // clipper
        zldsp::compressor::TanhClipper<float> clipper_;
        // output gain
        std::atomic<bool> to_update_output_gain_{true};
        std::atomic<float> output_gain_db_{0.f};
        zldsp::gain::Gain<float> output_gain_{};

        void prepareBuffer();

        void processBuffer(float* __restrict main_buffer0, float* __restrict main_buffer1,
                           float* __restrict side_buffer0, float* __restrict side_buffer1,
                           size_t num_samples, bool bypass);

        template <typename C>
        void processSideBuffer(C& c,
                               float* __restrict buffer0, float* __restrict buffer1, size_t num_samples);

        template <typename C, typename Style>
        void dispatchProcess(C& c, Style& comp0, Style& comp1,
                             float* __restrict buffer0, float* __restrict buffer1, size_t num_samples);

        template <typename C>
        void processSideBufferRMS(C& c,
                                  float* __restrict buffer0, float* __restrict buffer1, size_t num_samples);

        void handleAsyncUpdate() override;
    };
}
