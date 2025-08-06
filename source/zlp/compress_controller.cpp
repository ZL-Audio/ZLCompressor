// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "compress_controller.hpp"

namespace zlp {
    CompressController::CompressController(juce::AudioProcessor &processor)
        : processor_ref_(processor) {
    }

    void CompressController::prepare(const juce::dsp::ProcessSpec &spec) {
        main_spec_ = spec;
        mag_analyzer_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize));
        mag_avg_analyzer_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize));
        lufs_matcher_.prepare(spec.sampleRate, 2);
        output_gain_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize), 0.1);

        pre_buffer_[0].resize(static_cast<size_t>(spec.maximumBlockSize));
        pre_buffer_[1].resize(static_cast<size_t>(spec.maximumBlockSize));
        pre_pointers_[0] = pre_buffer_[0].data();
        pre_pointers_[1] = pre_buffer_[1].data();
        post_buffer_[0].resize(static_cast<size_t>(spec.maximumBlockSize));
        post_buffer_[1].resize(static_cast<size_t>(spec.maximumBlockSize));
        post_pointers_[0] = post_buffer_[0].data();
        post_pointers_[1] = post_buffer_[1].data();
        // allocate memories for up to 8x oversampling
        for (auto &t: rms_tracker_) {
            t.setMaximumMomentarySeconds(0.05f * 8.f);
            t.prepare(spec.sampleRate);
            t.setMaximumMomentarySeconds(0.05f);
        }
        rms_side_buffer0_.resize(static_cast<size_t>(spec.maximumBlockSize) * 8);
        rms_side_buffer1_.resize(rms_side_buffer0_.size());
        // init oversamplers
        over_sampler2_.prepare(4, static_cast<size_t>(spec.maximumBlockSize));
        over_sampler4_.prepare(4, static_cast<size_t>(spec.maximumBlockSize));
        over_sampler8_.prepare(4, static_cast<size_t>(spec.maximumBlockSize));

        oversample_delay_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize), 2,
                                  static_cast<float>(over_sampler8_.getLatency()) / static_cast<float>(spec.
                                      sampleRate));
        oversample_delay_.setDelayInSamples(0);
        c_oversample_idx_ = -1;
        // init lookahead delay
        lookahead_delay_.prepare(spec.sampleRate, static_cast<size_t>(spec.maximumBlockSize), 2, 0.02f);
        lookahead_delay_.setDelayInSamples(0);
        to_update_lookahead_.store(true, std::memory_order::release);
        // init hold buffers
        for (auto &h: hold_buffer_) {
            h.setCapacity(static_cast<size_t>(8.0 * spec.sampleRate));
        }
        to_update_.store(true, std::memory_order::release);
    }

    void CompressController::prepareBuffer() {
        bool to_update_pdc = false;
        c_is_on_ = is_on_.load(std::memory_order::relaxed);
        c_is_delta_ = is_delta_.load(std::memory_order::relaxed);
        c_mag_analyzer_on_ = mag_analyzer_on_.load(std::memory_order::relaxed);

        const auto new_lufs_matcher_on_ = lufs_matcher_on_.load(std::memory_order::relaxed);
        if (new_lufs_matcher_on_ != c_lufs_matcher_on_) {
            c_lufs_matcher_on_ = new_lufs_matcher_on_;
            if (c_lufs_matcher_on_) {
                lufs_matcher_.reset();
            }
        }

        c_copy_pre = c_mag_analyzer_on_ || c_is_delta_;
        c_copy_post = c_mag_analyzer_on_ || c_is_delta_;
        // load oversampling idx, set up trackers/followers and update latency
        const auto new_oversample_idx = oversample_idx_.load(std::memory_order::relaxed);
        if (new_oversample_idx != c_oversample_idx_) {
            to_update_pdc = true;
            c_oversample_idx_ = new_oversample_idx;
            switch (c_oversample_idx_) {
                case 0: {
                    oversample_delay_.setDelayInSamples(0);
                    break;
                }
                case 1: {
                    over_sampler2_.reset();
                    oversample_delay_.setDelayInSamples(static_cast<int>(over_sampler2_.getLatency()));
                    break;
                }
                case 2: {
                    over_sampler4_.reset();
                    oversample_delay_.setDelayInSamples(static_cast<int>(over_sampler4_.getLatency()));
                    break;
                }
                case 3: {
                    over_sampler8_.reset();
                    oversample_delay_.setDelayInSamples(static_cast<int>(over_sampler8_.getLatency()));
                    break;
                }
                default: ;
            }
            const auto oversample_mul = 1 << c_oversample_idx_;
            // prepare tracker and followers with the multiplied samplerate
            oversample_sr_ = main_spec_.sampleRate * static_cast<double>(oversample_mul);
            to_update_rms_.store(true, std::memory_order::release);
            for (auto &f: follower_) {
                f.prepare(oversample_sr_);
            }
            for (auto &f: rms_follower_) {
                f.prepare(oversample_sr_);
            }
            for (auto &t: rms_tracker_) {
                t.prepare(oversample_sr_);
            }
            // prepare the hold buffer with the multiplied samplerate
            for (auto &h: hold_buffer_) {
                h.setCapacity(static_cast<size_t>(oversample_sr_));
            }
            to_update_hold_.store(true);
        }
        if (to_update_lookahead_.exchange(false, std::memory_order::acquire)) {
            to_update_pdc = true;
            lookahead_delay_.setDelay(lookahead_delay_length_.load(std::memory_order::relaxed));
            is_lookahead_nonzero = (lookahead_delay_.getDelayInSamples() != 0);
        }
        // load stereo mode
        const auto stereo_mode = stereo_mode_.load(std::memory_order::relaxed);
        c_stereo_mode_is_midside = (stereo_mode == 0) || (stereo_mode == 2);
        c_stereo_mode_is_max = (stereo_mode == 2) || (stereo_mode == 3);
        c_stereo_swap_ = stereo_swap_.load(std::memory_order::relaxed);
        // load compressor style, reset the internal state if different
        const auto new_comp_style = comp_style_.load(std::memory_order::relaxed);
        if (c_comp_style_ != new_comp_style) {
            c_comp_style_ = new_comp_style;
            switch (c_comp_style_) {
                case zldsp::compressor::Style::kClean: {
                    clean_comps_[0].reset();
                    clean_comps_[1].reset();
                    break;
                }
                case zldsp::compressor::Style::kClassic: {
                    classic_comps_[0].reset();
                    classic_comps_[1].reset();
                    break;
                }
                case zldsp::compressor::Style::kOptical: {
                    optical_comps_[0].reset();
                    optical_comps_[1].reset();
                    break;
                }
                case zldsp::compressor::Style::kVocal: {
                    vocal_comps_[0].reset();
                    vocal_comps_[1].reset();
                    break;
                }
                default: break;
            }
        }
        // load stereo link
        c_stereo_link_ = stereo_link_.load(std::memory_order::relaxed);
        c_stereo_link_max_ = 1.f - 2.f * (1.f - c_stereo_link_);
        // load hold values
        if (to_update_hold_.exchange(false, std::memory_order::acquire)) {
            const auto oversample_mul = 1 << c_oversample_idx_;
            const auto hold_size = static_cast<size_t>(
                                       main_spec_.sampleRate * hold_length_.load(std::memory_order::relaxed)
                                   ) * static_cast<size_t>(oversample_mul);
            hold_buffer_[0].setSize(hold_size);
            hold_buffer_[1].setSize(hold_size);
        }
        // load wet values
        if (to_update_wet_.exchange(false, std::memory_order::acquire)) {
            const auto c_wet = wet_.load(std::memory_order::relaxed);
            c_wet1_ = wet1_.load(std::memory_order::relaxed) * c_wet * 0.05f;
            // 0.05 accounts for the db to gain transformation
            c_wet2_ = wet2_.load(std::memory_order::relaxed) * c_wet * 0.05f;
            to_update_range_.store(true);
            to_update_output_gain_.store(true);
        }
        if (to_update_range_.exchange(false, std::memory_order::acquire)) {
            c_range_ = range_.load(
                           std::memory_order::relaxed) * wet_.load(std::memory_order::relaxed);
        }
        if (to_update_output_gain_.exchange(false, std::memory_order::acquire)) {
            output_gain_.setGainDecibels(
                output_gain_db_.load(std::memory_order::relaxed) * wet_.load(std::memory_order::relaxed));
        }
        if (to_update_rms_.exchange(false, std::memory_order::acquire)) {
            c_use_rms_ = use_rms_.load(std::memory_order::relaxed);
            if (c_use_rms_) {
                c_rms_mix_ = rms_mix_.load(std::memory_order::relaxed);
                rms_tracker_[0].prepareBuffer();
                rms_tracker_[1].prepareBuffer();
            } else {
                rms_comps_[0].reset();
                rms_comps_[1].reset();
            }
        }
        if (to_update_pdc) {
            pdc_.store(lookahead_delay_.getDelayInSamples() + oversample_delay_.getDelayInSamples());
            triggerAsyncUpdate();
        }
    }

    void CompressController::process(std::array<float *, 2> main_pointers,
                                     std::array<float *, 2> side_pointers,
                                     const size_t num_samples, bool bypass) {
        if (to_update_.exchange(false, std::memory_order::acquire)) {
            prepareBuffer();
        }
        if (is_lookahead_nonzero) {
            lookahead_delay_.process(main_pointers, num_samples);
        }
        // process the pre lufs matcher
        if (c_lufs_matcher_on_) {
            lufs_matcher_.processPre(main_pointers, num_samples);
        }
        // stereo split the main/side buffer
        if (c_stereo_mode_is_midside) {
            zldsp::splitter::MSSplitter<float>::split(main_pointers[0], main_pointers[1], num_samples);
            zldsp::splitter::MSSplitter<float>::split(side_pointers[0], side_pointers[1], num_samples);
        }
        // copy pre buffer
        if (c_copy_pre) {
            zldsp::vector::copy<float>(pre_pointers_, main_pointers, num_samples);
        }
        // upsample side buffer
        std::array<float *, 4> pointers{main_pointers[0], main_pointers[1], side_pointers[0], side_pointers[1]};
        switch (c_oversample_idx_) {
            case 0: {
                processBuffer(main_pointers[0], main_pointers[1], side_pointers[0], side_pointers[1], num_samples, bypass);
                break;
            }
            case 1: {
                over_sampler2_.upsample(pointers, num_samples);
                auto &os_pointers = over_sampler2_.getOSPointer();
                processBuffer(os_pointers[0], os_pointers[1], os_pointers[2], os_pointers[3], num_samples << 1, bypass);
                over_sampler2_.downsample(pointers, num_samples);
                oversample_delay_.process(pre_pointers_, num_samples);
                break;
            }
            case 2: {
                over_sampler4_.upsample(pointers, num_samples);
                auto &os_pointers = over_sampler4_.getOSPointer();
                processBuffer(os_pointers[0], os_pointers[1], os_pointers[2], os_pointers[3], num_samples << 2, bypass);
                over_sampler4_.downsample(pointers, num_samples);
                oversample_delay_.process(pre_pointers_, num_samples);
                break;
            }
            case 3: {
                over_sampler8_.upsample(pointers, num_samples);
                auto &os_pointers = over_sampler8_.getOSPointer();
                processBuffer(os_pointers[0], os_pointers[1], os_pointers[2], os_pointers[3], num_samples << 3, bypass);
                over_sampler8_.downsample(pointers, num_samples);
                oversample_delay_.process(pre_pointers_, num_samples);
                break;
            }
            default: ;
        }
        // copy post buffer
        if (c_copy_post) {
            zldsp::vector::copy<float>(post_pointers_, main_pointers, num_samples);
        }
        // makeup gain
        if (c_is_on_ && !bypass) {
            output_gain_.template process<false>(main_pointers, num_samples);
        } else {
            output_gain_.template process<true>(main_pointers, num_samples);
        }
        // mag analyzer
        if (c_mag_analyzer_on_) {
            mag_analyzer_.process({pre_pointers_, post_pointers_, main_pointers}, num_samples);
            mag_avg_analyzer_.process({pre_pointers_, main_pointers}, num_samples);
        }
        // delta
        if (c_is_delta_) {
            for (size_t chan = 0; chan < 2; ++chan) {
                auto pre_vector = kfr::make_univector(pre_pointers_[chan], num_samples);
                auto post_vector = kfr::make_univector(post_pointers_[chan], num_samples);
                auto main_vector = kfr::make_univector(main_pointers[chan], num_samples);
                main_vector = pre_vector - post_vector;
            }
        }
        // stereo combine the main buffer
        if (c_stereo_mode_is_midside) {
            zldsp::splitter::MSSplitter<float>::combine(main_pointers[0], main_pointers[1], num_samples);
        }
        // process the post lufs matcher
        if (c_lufs_matcher_on_) {
            lufs_matcher_.processPost(main_pointers, num_samples);
        }
    }

    void CompressController::processBuffer(float * __restrict main_buffer0, float * __restrict main_buffer1,
                                           float * __restrict side_buffer0, float * __restrict side_buffer1,
                                           const size_t num_samples, bool bypass) {
        // create univector refs
        auto side_v0 = kfr::make_univector(side_buffer0, num_samples);
        auto side_v1 = kfr::make_univector(side_buffer1, num_samples);
        auto main_v0 = kfr::make_univector(main_buffer0, num_samples);
        auto main_v1 = kfr::make_univector(main_buffer1, num_samples);
        // prepare computer, trackers and followers
        if (computer_[0].prepareBuffer()) {
            computer_[1].copyFrom(computer_[0]);
            if (clipper_on_) {
                updateClipper();
            }
        }
        if (follower_[0].prepareBuffer()) {
            follower_[1].copyFrom(follower_[0]);
        }
        // prepare rms compressors
        if (c_use_rms_) {
            if (rms_follower_[0].prepareBuffer()) {
                rms_follower_[1].copyFrom(rms_follower_[0]);
            }
            auto rms_side_v0 = kfr::make_univector(rms_side_buffer0_.data(), num_samples);
            auto rms_side_v1 = kfr::make_univector(rms_side_buffer1_.data(), num_samples);
            rms_side_v0 = side_v0;
            rms_side_v1 = side_v1;
        }
        // process compress style
        switch (c_comp_style_) {
            case zldsp::compressor::Style::kClean: {
                processSideBufferClean(side_buffer0, side_buffer1, num_samples);
                break;
            }
            case zldsp::compressor::Style::kClassic: {
                processSideBufferClassic(side_buffer0, side_buffer1, num_samples);
                break;
            }
            case zldsp::compressor::Style::kOptical: {
                processSideBufferOptical(side_buffer0, side_buffer1, num_samples);
                break;
            }
            case zldsp::compressor::Style::kVocal: {
                processSideBufferVocal(side_buffer0, side_buffer1, num_samples);
                break;
            }
            default: return;
        }
        // process rms compressors and mix rms
        if (c_use_rms_) {
            processSideBufferRMS(rms_side_buffer0_.data(), rms_side_buffer1_.data(), num_samples);
            auto rms_side_v0 = kfr::make_univector(rms_side_buffer0_.data(), num_samples);
            auto rms_side_v1 = kfr::make_univector(rms_side_buffer1_.data(), num_samples);
            side_v0 = side_v0 * (1.f - c_rms_mix_) + rms_side_v0 * c_rms_mix_;
            side_v1 = side_v1 * (1.f - c_rms_mix_) + rms_side_v1 * c_rms_mix_;
        }
        // apply the hold
        if (hold_buffer_[0].getSize() > 0) {
            for (size_t i = 0; i < num_samples; ++i) {
                side_buffer0[i] = hold_buffer_[0].push(side_buffer0[i]);
                side_buffer1[i] = hold_buffer_[1].push(side_buffer1[i]);
            }
        }
        // if bypassed, skip reduction calculation
        if (!c_is_on_ || bypass) {
            return;
        }
        // apply the stereo link
        if (c_stereo_mode_is_max) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto x = side_buffer0[i];
                const auto y = side_buffer1[i];
                if (x < y) {
                    side_buffer1[i] = (y - x) * c_stereo_link_max_ + x;
                } else {
                    side_buffer0[i] = (x - y) * c_stereo_link_max_ + y;
                }
            }
        } else {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto x = side_buffer0[i];
                const auto y = side_buffer1[i];
                const auto xy = c_stereo_link_ * (x - y);
                side_buffer0[i] = y + xy;
                side_buffer1[i] = x - xy;
            }
        }
        // process wet values and convert decibel to gain
        side_v0 = kfr::exp10(kfr::max(side_v0, -c_range_) * c_wet1_);
        side_v1 = kfr::exp10(kfr::max(side_v1, -c_range_) * c_wet2_);
        // apply stereo swap
        if (c_stereo_swap_) {
            main_v0 = main_v0 * side_v1;
            main_v1 = main_v1 * side_v0;
        } else {
            main_v0 = main_v0 * side_v0;
            main_v1 = main_v1 * side_v1;
        }
        // apply clipper
        clipper_.prepareBuffer();
        if (clipper_on_ != clipper_.getIsON()) {
            clipper_on_ = clipper_.getIsON();
            if (clipper_on_) {
                updateClipper();
            }
        }
        if (clipper_on_) {
            clipper_.process(main_buffer0, num_samples);
            clipper_.process(main_buffer1, num_samples);
        }
    }

    void CompressController::processSideBufferClean(float * __restrict buffer0, float * __restrict buffer1,
                                                    const size_t num_samples) {
        clean_comps_[0].process(buffer0, num_samples);
        clean_comps_[1].process(buffer1, num_samples);
    }

    void CompressController::processSideBufferClassic(float * __restrict buffer0, float * __restrict buffer1,
                                                      const size_t num_samples) {
        classic_comps_[0].process(buffer0, num_samples);
        classic_comps_[1].process(buffer1, num_samples);
    }

    void CompressController::processSideBufferOptical(float * __restrict buffer0, float * __restrict buffer1,
                                                      const size_t num_samples) {
        optical_comps_[0].process(buffer0, num_samples);
        optical_comps_[1].process(buffer1, num_samples);
    }

    void CompressController::processSideBufferVocal(float * __restrict buffer0, float * __restrict buffer1,
                                                    const size_t num_samples) {
        vocal_comps_[0].process(buffer0, num_samples);
        vocal_comps_[1].process(buffer1, num_samples);
    }

    void CompressController::processSideBufferRMS(float * __restrict buffer0, float * __restrict buffer1,
                                                  const size_t num_samples) {
        rms_comps_[0].process<true>(buffer0, num_samples);
        rms_comps_[1].process<true>(buffer1, num_samples);
    }

    void CompressController::handleAsyncUpdate() {
        processor_ref_.setLatencySamples(pdc_.load(std::memory_order::relaxed));
    }

    void CompressController::updateClipper() {
        const auto threshold = computer_[0].getThreshold();
        const auto x1 = zldsp::chore::decibelsToGain(threshold);
        const auto y1 = zldsp::chore::decibelsToGain(threshold + computer_[0].eval(threshold));
        const auto y2 = zldsp::chore::decibelsToGain(computer_[0].eval(0.f));
        clipper_.update(x1, y1, y2);
    }
} // zlDSP
