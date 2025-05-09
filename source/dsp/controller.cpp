// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zldsp {
    Controller::Controller(juce::AudioProcessor &processor)
        : processor_ref(processor) {
    }

    void Controller::prepare(const juce::dsp::ProcessSpec &spec) {
        main_spec = spec;
        mag_analyzer.prepare(spec);

        pre_buffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
        // allocate memories for up to 8x oversampling
        for (auto &t: {&tracker1, &tracker2}) {
            t->setMaximumMomentarySeconds(0.04 * 8.01);
            t->prepare(spec.sampleRate);
            t->setMaximumMomentarySeconds(0.04);
        }
        oversampled_side_buffer.setSize(2, static_cast<int>(spec.maximumBlockSize) * 8);
        // init oversamplers
        for (auto &os: oversample_stages_main) {
            os.initProcessing(static_cast<size_t>(spec.maximumBlockSize));
        }
        for (auto &os: oversample_stages_side) {
            os.initProcessing(static_cast<size_t>(spec.maximumBlockSize));
        }
    }


    void Controller::prepareBuffer() {
        // load external side-chain
        c_ext_side_chain = ext_side_chain.load();
        // load stereo mode
        c_stereo_mode = stereo_mode.load();
        // load stereo link
        c_link = 1.0 - std::clamp(link.load(), 0.0, 0.5);
        // load wet values
        if (to_update_wet.exchange(false)) {
            const auto c_wet = wet.load();
            c_wet1 = wet1.load() * c_wet * 0.05; // 0.05 accounts for the db to gain transformation
            c_wet2 = wet2.load() * c_wet * 0.05;
        }
        // load oversampling idx, set up trackers/followers and update latency
        if (oversample_idx.load() != c_oversample_idx) {
            c_oversample_idx = oversample_idx.load();
            if (c_oversample_idx > 0) {
                c_oversample_stage_idx = static_cast<size_t>(c_oversample_idx - 1);
                oversample_latency.store(
                    static_cast<int>(oversample_stages_main[c_oversample_stage_idx].getLatencyInSamples()));
            } else {
                oversample_latency.store(0);
            }
            const auto oversample_mul = 1 << c_oversample_idx;
            // prepare tracker and followers with the multiplied samplerate
            const auto oversample_sr = main_spec.sampleRate * static_cast<double>(oversample_mul);
            for (auto &t: {&tracker1, &tracker2}) {
                t->prepare(oversample_sr);
            }
            for (auto &f: {&follower1, &follower2}) {
                f->prepare(oversample_sr);
            }
            // prepare oversampled side-buffer
            oversampled_side_buffer.setSize(2, static_cast<int>(main_spec.maximumBlockSize) * oversample_mul);
            triggerAsyncUpdate();
        }
    }

    void Controller::process(juce::AudioBuffer<double> &buffer) {
        prepareBuffer();
        juce::AudioBuffer<double> main_buffer{buffer.getArrayOfWritePointers() + 0, 2, buffer.getNumSamples()};
        juce::AudioBuffer<double> side_buffer{buffer.getArrayOfWritePointers() + 2, 2, buffer.getNumSamples()};
        pre_buffer.makeCopyOf(main_buffer, true);

        if (!ext_side_chain.load()) {
            side_buffer.makeCopyOf(main_buffer, true);
        }

        // stereo split the main/side buffer
        if (c_stereo_mode == 1) {
            ms_splitter.split(main_buffer);
            if (c_ext_side_chain) ms_splitter.split(side_buffer);
        }
        // up-sample side buffer
        if (c_oversample_idx == 0) {
            if (!c_ext_side_chain) side_buffer.makeCopyOf(main_buffer, true);
            processBuffer(buffer.getWritePointer(0), buffer.getWritePointer(1),
                          buffer.getWritePointer(2), buffer.getWritePointer(3),
                          static_cast<size_t>(buffer.getNumSamples()));
        } else {
            juce::dsp::AudioBlock<double> main_block(main_buffer);
            // upsample the main buffer
            auto &main_oversampler = oversample_stages_main[c_oversample_stage_idx];
            auto os_main_block = main_oversampler.processSamplesUp(main_block);
            if (c_ext_side_chain) {
                // upsample the side buffer
                juce::dsp::AudioBlock<double> side_block(side_buffer);
                auto &side_oversampler = oversample_stages_side[c_oversample_stage_idx];
                auto os_side_block = side_oversampler.processSamplesUp(side_block);
                // process the oversampled buffers
                processBuffer(os_main_block.getChannelPointer(0), os_main_block.getChannelPointer(1),
                              os_side_block.getChannelPointer(0), os_side_block.getChannelPointer(1),
                              os_main_block.getNumSamples());
            } else {
                // copy the oversampled main buffer to the oversampled side buffer
                oversampled_side_buffer.copyFrom(0, 0, os_main_block.getChannelPointer(0),
                                                 static_cast<int>(os_main_block.getNumSamples()));
                oversampled_side_buffer.copyFrom(1, 0, os_main_block.getChannelPointer(1),
                                                 static_cast<int>(os_main_block.getNumSamples()));
                // process the oversampled buffers
                processBuffer(os_main_block.getChannelPointer(0), os_main_block.getChannelPointer(1),
                              oversampled_side_buffer.getWritePointer(0), oversampled_side_buffer.getWritePointer(1),
                              static_cast<size_t>(os_main_block.getNumSamples()));
            }
            // downsample the main buffer
            main_oversampler.processSamplesDown(main_block);
        }

        juce::dsp::AudioBlock<double> block(main_buffer);
        mag_analyzer.process({pre_buffer, main_buffer});
        mag_avg_analyzer.process({pre_buffer, main_buffer});
    }

    void Controller::processBuffer(double *main_buffer1, double *main_buffer2,
                                   double *side_buffer1, double *side_buffer2,
                                   const size_t num_samples) {
        // prepare computer, trackers and followers
        computer.prepareBuffer();
        tracker1.prepareBuffer();
        tracker2.prepareBuffer();
        follower1.prepareBuffer();
        follower2.prepareBuffer();
        // process compress style
        const auto c_style = style.load();
        switch (c_style) {
            case compressor::style::clean: {
                processSideBufferClean(side_buffer1, side_buffer2, num_samples);
                break;
            }
            case compressor::style::classic: {
                processSideBufferClassic(side_buffer1, side_buffer2, num_samples);
                break;
            }
            case compressor::style::optical: {
                processSideBufferOptical(side_buffer1, side_buffer2, num_samples);
                break;
            }
            case compressor::style::bus: {
                processSideBufferBus(side_buffer1, side_buffer2, num_samples);
                break;
            }
        }
        // apply the stereo link
        for (size_t i = 0; i < num_samples; ++i) {
            const auto x = side_buffer1[i];
            const auto y = side_buffer2[i];
            const auto xy = c_link * (x - y);
            side_buffer1[i] = y + xy;
            side_buffer2[i] = x - xy;
        }
        // process wet values and convert decibel to gain
        auto side_v1 = kfr::make_univector(side_buffer1, num_samples);
        auto side_v2 = kfr::make_univector(side_buffer2, num_samples);
        side_v1 = kfr::exp10(side_v1 * c_wet1);
        side_v2 = kfr::exp10(side_v2 * c_wet2);
        // apply gain on the main buffer
        auto main_v1 = kfr::make_univector(main_buffer1, num_samples);
        auto main_v2 = kfr::make_univector(main_buffer2, num_samples);
        if (side_swap.load()) {
            main_v1 = main_v1 * side_v2;
            main_v2 = main_v2 * side_v1;
        } else {
            main_v1 = main_v1 * side_v1;
            main_v2 = main_v2 * side_v2;
        }
    }


    void Controller::processSideBufferClean(double *buffer1, double *buffer2, const size_t num_samples) {
        cleans[0].process(buffer1, num_samples);
        cleans[1].process(buffer2, num_samples);
    }

    void Controller::processSideBufferClassic(double *buffer1, double *buffer2, const size_t num_samples) {
        juce::ignoreUnused(buffer1, buffer2, num_samples);
    }

    void Controller::processSideBufferOptical(double *buffer1, double *buffer2, const size_t num_samples) {
        juce::ignoreUnused(buffer1, buffer2, num_samples);
    }

    void Controller::processSideBufferBus(double *buffer1, double *buffer2, const size_t num_samples) {
        juce::ignoreUnused(buffer1, buffer2, num_samples);
    }

    void Controller::handleAsyncUpdate() {
        processor_ref.setLatencySamples(oversample_latency.load());
    }
} // zlDSP
