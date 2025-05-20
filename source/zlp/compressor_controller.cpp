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

        pre_buffer_.setSize(2, static_cast<int>(spec.maximumBlockSize));
        // allocate memories for up to 8x oversampling
        for (auto &t: tracker_) {
            t.setMaximumMomentarySeconds(0.04f * 8.01f);
            t.prepare(spec.sampleRate);
            t.setMaximumMomentarySeconds(0.04f);
        }
        oversampled_side_buffer_.setSize(2, static_cast<int>(spec.maximumBlockSize) * 8);
        // init oversamplers
        for (auto &os: oversample_stages_main_) {
            os.initProcessing(static_cast<size_t>(spec.maximumBlockSize));
        }
        for (auto &os: oversample_stages_side_) {
            os.initProcessing(static_cast<size_t>(spec.maximumBlockSize));
        }
    }


    void CompressorController::prepareBuffer() {
        // load external side-chain
        c_ext_side_chain_ = ext_side_chain_.load();
        // load stereo mode
        c_stereo_mode_ = stereo_mode_.load();
        // load compressor style, if they are different, reset the internal state
        if (c_comp_style_ != comp_style_.load()) {
            c_comp_style_ = comp_style_.load();
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
        c_stereo_link_ = 1.f - std::clamp(stereo_link_.load(), 0.f, .5f);
        // load wet values
        if (to_update_wet_.exchange(false)) {
            const auto c_wet = wet_.load();
            c_wet1_ = wet1_.load() * c_wet * 0.05f; // 0.05 accounts for the db to gain transformation
            c_wet2_ = wet2_.load() * c_wet * 0.05f;
        }
        // load oversampling idx, set up trackers/followers and update latency
        if (oversample_idx_.load() != c_oversample_idx_) {
            c_oversample_idx_ = oversample_idx_.load();
            if (c_oversample_idx_ > 0) {
                c_oversample_stage_idx_ = static_cast<size_t>(c_oversample_idx_ - 1);
                oversample_latency_.store(
                    static_cast<int>(oversample_stages_main_[c_oversample_stage_idx_].getLatencyInSamples()));
            } else {
                oversample_latency_.store(0);
            }
            const auto oversample_mul = 1 << c_oversample_idx_;
            // prepare tracker and followers with the multiplied samplerate
            const auto oversample_sr = main_spec_.sampleRate * static_cast<float>(oversample_mul);
            for (auto &t: tracker_) {
                t.prepare(oversample_sr);
            }
            for (auto &f: follower_) {
                f.prepare(oversample_sr);
            }
            // prepare oversampled side-buffer
            oversampled_side_buffer_.setSize(2, static_cast<int>(main_spec_.maximumBlockSize) * oversample_mul);
            triggerAsyncUpdate();
        }
    }

    void CompressorController::process(juce::AudioBuffer<float> &buffer) {
        prepareBuffer();
        juce::AudioBuffer<float> main_buffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<float> side_buffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        pre_pointers_[0] = pre_buffer_.getWritePointer(0);
        pre_pointers_[1] = pre_buffer_.getWritePointer(1);
        main_pointers_[0] = main_buffer.getWritePointer(0);
        main_pointers_[1] = main_buffer.getWritePointer(1);

        pre_buffer_.makeCopyOf(main_buffer, true);

        if (!ext_side_chain_.load()) {
            side_buffer.makeCopyOf(main_buffer, true);
        }

        // stereo split the main/side buffer
        if (c_stereo_mode_ == 1) {
            zldsp::splitter::MSSplitter<float>::split(main_buffer.getWritePointer(0),
                                                       main_buffer.getWritePointer(1),
                                                       static_cast<size_t>(main_buffer.getNumSamples()));
            if (c_ext_side_chain_) {
                zldsp::splitter::MSSplitter<float>::split(side_buffer.getWritePointer(0),
                                                           side_buffer.getWritePointer(1),
                                                           static_cast<size_t>(side_buffer.getNumSamples()));
            }
        }
        // up-sample side buffer
        if (c_oversample_idx_ == 0) {
            if (!c_ext_side_chain_) side_buffer.makeCopyOf(main_buffer, true);
            processBuffer(buffer.getWritePointer(0), buffer.getWritePointer(1),
                          buffer.getWritePointer(2), buffer.getWritePointer(3),
                          static_cast<size_t>(buffer.getNumSamples()));
        } else {
            juce::dsp::AudioBlock<float> main_block(main_buffer);
            // upsample the main buffer
            auto &main_oversampler = oversample_stages_main_[c_oversample_stage_idx_];
            auto os_main_block = main_oversampler.processSamplesUp(main_block);
            if (c_ext_side_chain_) {
                // upsample the side buffer
                juce::dsp::AudioBlock<float> side_block(side_buffer);
                auto &side_oversampler = oversample_stages_side_[c_oversample_stage_idx_];
                auto os_side_block = side_oversampler.processSamplesUp(side_block);
                // process the oversampled buffers
                processBuffer(os_main_block.getChannelPointer(0), os_main_block.getChannelPointer(1),
                              os_side_block.getChannelPointer(0), os_side_block.getChannelPointer(1),
                              os_main_block.getNumSamples());
            } else {
                // copy the oversampled main buffer to the oversampled side buffer
                oversampled_side_buffer_.copyFrom(0, 0, os_main_block.getChannelPointer(0),
                                                  static_cast<int>(os_main_block.getNumSamples()));
                oversampled_side_buffer_.copyFrom(1, 0, os_main_block.getChannelPointer(1),
                                                  static_cast<int>(os_main_block.getNumSamples()));
                // process the oversampled buffers
                processBuffer(os_main_block.getChannelPointer(0), os_main_block.getChannelPointer(1),
                              oversampled_side_buffer_.getWritePointer(0), oversampled_side_buffer_.getWritePointer(1),
                              static_cast<size_t>(os_main_block.getNumSamples()));
            }
            // downsample the main buffer
            main_oversampler.processSamplesDown(main_block);
        }

        mag_analyzer_.process({std::span(pre_pointers_), std::span{main_pointers_}},
                              static_cast<size_t>(buffer.getNumSamples()));
        mag_avg_analyzer_.process({std::span(pre_pointers_), std::span{main_pointers_}},
                                  static_cast<size_t>(buffer.getNumSamples()));


        output_gain_.process(std::span(main_pointers_), static_cast<size_t>(main_buffer.getNumSamples()));
    }

    void CompressorController::processBuffer(float *main_buffer1, float *main_buffer2,
                                   float *side_buffer1, float *side_buffer2,
                                   const size_t num_samples) {
        // prepare computer, trackers and followers
        if (computer_[0].prepareBuffer()) { computer_[1].copyFrom(computer_[0]); }
        tracker_[0].prepareBuffer();
        tracker_[1].prepareBuffer();
        if (follower_[0].prepareBuffer()) { follower_[1].copyFrom(follower_[0]); }

        // process compress style
        switch (c_comp_style_) {
            case zldsp::compressor::Style::kClean: {
                processSideBufferClean(side_buffer1, side_buffer2, num_samples);
                break;
            }
            case zldsp::compressor::Style::kClassic: {
                processSideBufferClassic(side_buffer1, side_buffer2, num_samples);
                break;
            }
            case zldsp::compressor::Style::kOptical: {
                processSideBufferOptical(side_buffer1, side_buffer2, num_samples);
                break;
            }
            default: return;
        }
        // apply the stereo link
        for (size_t i = 0; i < num_samples; ++i) {
            const auto x = side_buffer1[i];
            const auto y = side_buffer2[i];
            const auto xy = c_stereo_link_ * (x - y);
            side_buffer1[i] = y + xy;
            side_buffer2[i] = x - xy;
        }
        // process wet values and convert decibel to gain
        auto side_v1 = kfr::make_univector(side_buffer1, num_samples);
        auto side_v2 = kfr::make_univector(side_buffer2, num_samples);
        side_v1 = kfr::exp10(side_v1 * c_wet1_);
        side_v2 = kfr::exp10(side_v2 * c_wet2_);
        // apply gain on the main buffer
        auto main_v1 = kfr::make_univector(main_buffer1, num_samples);
        auto main_v2 = kfr::make_univector(main_buffer2, num_samples);
        if (side_swap_.load()) {
            main_v1 = main_v1 * side_v2;
            main_v2 = main_v2 * side_v1;
        } else {
            main_v1 = main_v1 * side_v1;
            main_v2 = main_v2 * side_v2;
        }
    }


    void CompressorController::processSideBufferClean(float *buffer1, float *buffer2, const size_t num_samples) {
        clean_comps_[0].process(buffer1, num_samples);
        clean_comps_[1].process(buffer2, num_samples);
    }

    void CompressorController::processSideBufferClassic(float *buffer1, float *buffer2, const size_t num_samples) {
        classic_comps_[0].process(buffer1, num_samples);
        classic_comps_[1].process(buffer2, num_samples);
    }

    void CompressorController::processSideBufferOptical(float *buffer1, float *buffer2, const size_t num_samples) {
        optical_comps_[0].process(buffer1, num_samples);
        optical_comps_[1].process(buffer2, num_samples);
    }

    void CompressorController::handleAsyncUpdate() {
        processor_ref_.setLatencySamples(oversample_latency_.load());
    }
} // zlDSP
