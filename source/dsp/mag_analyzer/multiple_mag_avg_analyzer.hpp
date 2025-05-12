// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "../chore/decibels.hpp"
#include "../vector/vector.hpp"
#include "../container/abstract_fifo.hpp"

namespace zldsp::analyzer {
    template<typename FloatType, size_t MagNum, size_t BinNum>
    class MultipleMagAvgAnalyzer {
    public:
        static constexpr double kRmsLength = 0.01;

        enum MagType {
            kPeak, kRMS
        };

        explicit MultipleMagAvgAnalyzer() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            sample_rate_.store(spec.sampleRate);
            max_pos_ = sample_rate_.load() * kRmsLength;
            current_pos_ = 0.;
            current_num_samples_ = 0;
        }

        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> buffers) {
            switch (mag_type_.load()) {
                case MagType::kPeak: {
                    processBuffer<MagType::kPeak>(buffers);
                    break;
                }
                case MagType::kRMS: {
                    processBuffer<MagType::kRMS>(buffers);
                    break;
                }
                default: {
                }
            }
        }

        void setToReset() { to_reset_.store(true); }

        void setMagType(const MagType x) { mag_type_.store(x); }

        void run() {
            juce::ScopedNoDenormals no_denormals;
            if (to_reset_.exchange(false)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    auto &cumulative_count{cumulative_counts_[i]};
                    std::fill(cumulative_count.begin(), cumulative_count.end(), 0.);
                }
            }

            const int num_ready = abstract_fifo_.getNumReady();
            zldsp::container::AbstractFIFO::Range range;
            abstract_fifo_.prepareToRead(num_ready, range);
            for (size_t i = 0; i < MagNum; ++i) {
                auto &mag_fifo{mag_fifos_[i]};
                auto &cumulative_count{cumulative_counts_[i]};
                for (auto idx = range.start_index1; idx < range.start_index1 + range.block_size1; ++idx) {
                    updateHist(cumulative_count, mag_fifo[static_cast<size_t>(idx)]);
                }
                for (auto idx = range.start_index2; idx < range.start_index2 + range.block_size2; ++idx) {
                    updateHist(cumulative_count, mag_fifo[static_cast<size_t>(idx)]);
                }
            }
            abstract_fifo_.finishedRead(num_ready);

            std::array<double, MagNum> maximum_counts{};
            for (size_t i = 0; i < MagNum; ++i) {
                maximum_counts[i] = *std::max_element(cumulative_counts_[i].begin(), cumulative_counts_[i].end());
            }
            const auto maximum_count = std::max(10. / kRmsLength,
                                                *std::max_element(maximum_counts.begin(), maximum_counts.end()));
            const auto maximum_count_r = 1.0 / maximum_count;
            for (size_t i = 0; i < MagNum; ++i) {
                auto &cumulative_count{cumulative_counts_[i]};
                auto &avg_count{avg_counts_[i]};
                zldsp::vector::multiply(avg_count.data(), cumulative_count.data(), maximum_count_r, avg_count.size());
            }
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, MagNum> paths,
                        const std::array<bool, MagNum> is_close_path,
                        const juce::Rectangle<float> bound, size_t end_idx) {
            end_idx = std::min(end_idx, BinNum);
            const auto delta_y = bound.getHeight() / static_cast<float>(end_idx - 1);
            for (size_t i = 0; i < MagNum; ++i) {
                const auto y = bound.getY();
                const auto &avg_count{avg_counts_[i]};
                constexpr size_t idx = 0;
                const auto x = bound.getX() + static_cast<float>(avg_count[idx]) * bound.getWidth();
                if (is_close_path[i]) {
                    paths[i].get().startNewSubPath(bound.getTopLeft());
                    paths[i].get().lineTo(x, y);
                } else {
                    paths[i].get().startNewSubPath(x, y);
                }
            }

            for (size_t i = 0; i < MagNum; ++i) {
                auto y = delta_y;
                const auto &avg_count{avg_counts_[i]};
                for (size_t idx = 1; idx < end_idx; ++idx) {
                    const auto x = bound.getX() + static_cast<float>(avg_count[idx]) * bound.getWidth();
                    paths[i].get().lineTo(x, y);
                    y += delta_y;
                }
            }
            for (size_t i = 0; i < MagNum; ++i) {
                if (is_close_path[i]) {
                    paths[i].get().lineTo(bound.getBottomLeft());
                    paths[i].get().closeSubPath();
                }
            }
        }

    protected:
        std::atomic<double> sample_rate_{48000.0};
        std::array<std::array<float, 1000>, MagNum> mag_fifos_{};
        zldsp::container::AbstractFIFO abstract_fifo_{1000};

        std::atomic<bool> to_reset_{false};
        std::atomic<MagType> mag_type_{MagType::kRMS};

        double current_pos_{0.}, max_pos_{1.};
        int current_num_samples_{0};
        std::array<FloatType, MagNum> current_mags_{};

        std::array<std::array<double, BinNum>, MagNum> cumulative_counts_{};
        std::array<std::array<double, BinNum>, MagNum> avg_counts_{};

        template<MagType CurrentMagType>
        void processBuffer(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> &buffers) {
            int start_idx{0}, end_idx{0};
            int num_samples = buffers[0].get().getNumSamples();
            if (num_samples == 0) { return; }
            while (true) {
                const auto remain_num = static_cast<int>(std::round(max_pos_ - current_pos_));
                if (num_samples >= remain_num) {
                    start_idx = end_idx;
                    end_idx = end_idx + remain_num;
                    num_samples -= remain_num;
                    updateMags<CurrentMagType>(buffers, start_idx, remain_num);
                    current_pos_ = current_pos_ + static_cast<double>(remain_num) - max_pos_;
                    if (abstract_fifo_.getNumFree() > 0) {

                        zldsp::container::AbstractFIFO::Range range;
                        abstract_fifo_.prepareToWrite(1, range);
                        const auto write_idx = range.block_size1 > 0 ? range.start_index1 : range.start_index2;
                        switch (CurrentMagType) {
                            case MagType::kPeak: {
                                for (size_t i = 0; i < MagNum; ++i) {
                                    mag_fifos_[i][static_cast<size_t>(write_idx)] = zldsp::chore::gainToDecibels(
                                        static_cast<float>(current_mags_[i]));
                                }
                                break;
                            }
                            case MagType::kRMS: {
                                for (size_t i = 0; i < MagNum; ++i) {
                                    mag_fifos_[i][static_cast<size_t>(write_idx)] =
                                            0.5f * zldsp::chore::gainToDecibels(
                                                static_cast<float>(
                                                    current_mags_[i] / static_cast<FloatType>(current_num_samples_)));
                                }
                                current_num_samples_ = 0;
                                break;
                            }
                        }
                        abstract_fifo_.finishedWrite(1);

                        std::fill(current_mags_.begin(), current_mags_.end(), FloatType(0));
                    }
                } else {
                    updateMags<CurrentMagType>(buffers, start_idx, num_samples);
                    current_pos_ += static_cast<double>(num_samples);
                    break;
                }
            }
        }

        template<MagType CurrentMagType>
        void updateMags(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> buffers,
                        const int start_idx, const int num_samples) {
            for (size_t i = 0; i < MagNum; ++i) {
                auto &buffer = buffers[i];
                switch (CurrentMagType) {
                    case MagType::kPeak: {
                        const auto current_magnitude = buffer.get().getMagnitude(start_idx, num_samples);
                        current_mags_[i] = std::max(current_mags_[i], current_magnitude);
                    }
                    case MagType::kRMS: {
                        FloatType current_s{FloatType(0)};
                        for (int j = 0; j < buffer.get().getNumChannels(); ++j) {
                            auto *data = buffer.get().getReadPointer(j, start_idx);
                            for (auto idx = 0; idx < num_samples; ++idx) {
                                current_s += data[static_cast<size_t>(idx)] * data[static_cast<size_t>(idx)];
                            }
                        }
                        current_mags_[i] += current_s;
                        current_num_samples_ += num_samples;
                    }
                }
            }
        }

        static inline void updateHist(std::array<double, BinNum> &hist, const double x) {
            const auto idx = static_cast<size_t>(std::max(0., std::round(-x)));
            if (idx < BinNum) {
                zldsp::vector::multiply(hist.data(),0.99999, hist.size());
                hist[idx] += 1.;
            }
        }
    };
}
