// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "compressor_controller.hpp"

namespace zlp {
    CompressorController::CompressorController(juce::AudioProcessor &processor)
        : processor_ref_(processor) {
    }

    void CompressorController::prepare(const juce::dsp::ProcessSpec &spec) {
        main_spec_ = spec;
        mag_analyzer_.prepare(spec.sampleRate);
        mag_avg_analyzer_.prepare(spec.sampleRate);
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
        for (auto &t: tracker_) {
            t.setMaximumMomentarySeconds(0.04f * 8.01f);
            t.prepare(spec.sampleRate);
            t.setMaximumMomentarySeconds(0.04f);
        }
        oversampled_side_buffer_.setSize(2, static_cast<int>(spec.maximumBlockSize) * 8);
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
    }

    void CompressorController::prepareBuffer() {
        bool to_update_pdc = false;
        c_mag_analyzer_on_ = mag_analyzer_on_.load(std::memory_order::relaxed);
        c_spec_analyzer_on_ = spec_analyzer_on_.load(std::memory_order::relaxed);
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
            const auto oversample_sr = main_spec_.sampleRate * static_cast<double>(oversample_mul);
            for (auto &t: tracker_) {
                t.prepare(oversample_sr);
            }
            for (auto &f: follower_) {
                f.prepare(oversample_sr);
            }
            // prepare the hold buffer with the multiplied samplerate
            for (auto &h: hold_buffer_) {
                h.setCapacity(static_cast<size_t>(oversample_sr));
            }
            to_update_hold_.store(true);
        }
        if (to_update_lookahead_.exchange(false, std::memory_order::acquire)) {
            to_update_pdc = true;
            lookahead_delay_.setDelay(lookahead_delay_length_.load(std::memory_order::relaxed));
            is_lookahead_nonzero = (lookahead_delay_.getDelayInSamples() != 0);
        }
        // load stereo mode
        c_stereo_mode_ = stereo_mode_.load(std::memory_order::relaxed);
        // load compressor style, if they are different, reset the internal state
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
                }
                case zldsp::compressor::Style::kOptical: {
                    optical_comps_[0].reset();
                    optical_comps_[1].reset();
                }
                default: break;
            }
        }
        // load stereo link
        c_stereo_link_ = stereo_link_.load(std::memory_order::relaxed);
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
        if (to_update_pdc) {
            pdc_.store(lookahead_delay_.getDelayInSamples() + oversample_delay_.getDelayInSamples());
            triggerAsyncUpdate();
        }
    }

    void CompressorController::process(std::array<float *, 2> main_pointers,
                                       std::array<float *, 2> side_pointers,
                                       const size_t num_samples) {
        prepareBuffer();
        if (is_lookahead_nonzero) {
            lookahead_delay_.process(main_pointers, num_samples);
        }

        if (c_mag_analyzer_on_) {
            zldsp::vector::copy<float>(pre_pointers_, main_pointers, num_samples);
        }

        // stereo split the main/side buffer
        if (c_stereo_mode_ == 0) {
            zldsp::splitter::MSSplitter<float>::split(main_pointers[0], main_pointers[1], num_samples);
            zldsp::splitter::MSSplitter<float>::split(side_pointers[0], side_pointers[1], num_samples);
        }
        // upsample side buffer
        std::array<float *, 4> pointers{main_pointers[0], main_pointers[1], side_pointers[0], side_pointers[1]};
        switch (c_oversample_idx_) {
            case 0: {
                processBuffer(main_pointers[0], main_pointers[1], side_pointers[0], side_pointers[1], num_samples);
                break;
            }
            case 1: {
                over_sampler2_.upsample(pointers, num_samples);
                auto &os_pointers = over_sampler2_.getOSPointer();
                processBuffer(os_pointers[0], os_pointers[1], os_pointers[2], os_pointers[3], num_samples << 1);
                over_sampler2_.downsample(pointers, num_samples);
                oversample_delay_.process(pre_pointers_, num_samples);
                break;
            }
            case 2: {
                over_sampler4_.upsample(pointers, num_samples);
                auto &os_pointers = over_sampler4_.getOSPointer();
                processBuffer(os_pointers[0], os_pointers[1], os_pointers[2], os_pointers[3], num_samples << 2);
                over_sampler4_.downsample(pointers, num_samples);
                oversample_delay_.process(pre_pointers_, num_samples);
                break;
            }
            case 3: {
                over_sampler8_.upsample(pointers, num_samples);
                auto &os_pointers = over_sampler8_.getOSPointer();
                processBuffer(os_pointers[0], os_pointers[1], os_pointers[2], os_pointers[3], num_samples << 3);
                over_sampler8_.downsample(pointers, num_samples);
                oversample_delay_.process(pre_pointers_, num_samples);
                break;
            }
            default: ;
        }

        // stereo combine the main buffer
        if (c_stereo_mode_ == 0) {
            zldsp::splitter::MSSplitter<float>::combine(main_pointers[0], main_pointers[1], num_samples);
        }

        // copy to post buffer
        if (c_mag_analyzer_on_) {
            zldsp::vector::copy<float>(post_pointers_, main_pointers, num_samples);
        }
        // makeup gain
        output_gain_.process(main_pointers, num_samples);

        if (c_mag_analyzer_on_) {
            mag_analyzer_.process({pre_pointers_, post_pointers_, main_pointers}, num_samples);
            mag_avg_analyzer_.process({pre_pointers_, main_pointers}, num_samples);
        }

        if (is_delta_.load(std::memory_order::relaxed)) {
            for (size_t chan = 0; chan < 2; ++chan) {
                auto pre_vector = kfr::make_univector(pre_pointers_[chan], num_samples);
                auto post_vector = kfr::make_univector(post_pointers_[chan], num_samples);
                auto main_vector = kfr::make_univector(main_pointers[chan], num_samples);
                main_vector = pre_vector - post_vector;
            }
        }
    }

    void CompressorController::processBuffer(float * __restrict main_buffer0, float * __restrict main_buffer1,
                                             float * __restrict side_buffer0, float * __restrict side_buffer1,
                                             const size_t num_samples) {
        // prepare computer, trackers and followers
        if (computer_[0].prepareBuffer()) { computer_[1].copyFrom(computer_[0]); }
        tracker_[0].prepareBuffer();
        tracker_[1].prepareBuffer();
        if (follower_[0].prepareBuffer()) { follower_[1].copyFrom(follower_[0]); }

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
            default: return;
        }
        // apply the hold
        if (hold_buffer_[0].getSize() > 0) {
            for (size_t i = 0; i < num_samples; ++i) {
                side_buffer0[i] = hold_buffer_[0].push(side_buffer0[i]);
                side_buffer1[i] = hold_buffer_[1].push(side_buffer1[i]);
            }
        }
        // apply the stereo link
        for (size_t i = 0; i < num_samples; ++i) {
            const auto x = side_buffer0[i];
            const auto y = side_buffer1[i];
            const auto xy = c_stereo_link_ * (x - y);
            side_buffer0[i] = y + xy;
            side_buffer1[i] = x - xy;
        }
        // process wet values and convert decibel to gain
        auto side_v0 = kfr::make_univector(side_buffer0, num_samples);
        auto side_v1 = kfr::make_univector(side_buffer1, num_samples);
        side_v0 = kfr::exp10(kfr::max(side_v0, -c_range_) * c_wet1_);
        side_v1 = kfr::exp10(kfr::max(side_v1, -c_range_) * c_wet2_);
        // apply gain on the main buffer
        auto main_v0 = kfr::make_univector(main_buffer0, num_samples);
        auto main_v1 = kfr::make_univector(main_buffer1, num_samples);
        if (is_on_.load(std::memory_order::relaxed)) {
            if (stereo_swap_.load(std::memory_order::relaxed)) {
                main_v0 = main_v0 * side_v1;
                main_v1 = main_v1 * side_v0;
            } else {
                main_v0 = main_v0 * side_v0;
                main_v1 = main_v1 * side_v1;
            }
        }
    }

    void CompressorController::processSideBufferClean(float * __restrict buffer0, float * __restrict buffer1,
                                                      const size_t num_samples) {
        clean_comps_[0].process(buffer0, num_samples);
        clean_comps_[1].process(buffer1, num_samples);
    }

    void CompressorController::processSideBufferClassic(float * __restrict buffer0, float * __restrict buffer1,
                                                        const size_t num_samples) {
        classic_comps_[0].process(buffer0, num_samples);
        classic_comps_[1].process(buffer1, num_samples);
    }

    void CompressorController::processSideBufferOptical(float * __restrict buffer0, float * __restrict buffer1,
                                                        const size_t num_samples) {
        optical_comps_[0].process(buffer0, num_samples);
        optical_comps_[1].process(buffer1, num_samples);
    }

    void CompressorController::handleAsyncUpdate() {
        processor_ref_.setLatencySamples(pdc_.load(std::memory_order::relaxed));
    }
} // zlDSP
