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

namespace zldsp::analyzer {
    template<typename FloatType, size_t MagNum, size_t PointNum>
    class MultipleMagAnalyzer {
    public:
        enum MagType {
            kPeak, kRMS
        };

        explicit MultipleMagAnalyzer() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            sample_rate_.store(spec.sampleRate);
            setTimeLength(time_length_.load());
            std::fill(current_mags_.begin(), current_mags_.end(), FloatType(-999));
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

        int run(const int num_to_read = PointNum) {
            juce::ScopedNoDenormals no_denormals;
            // calculate the number of points put into circular buffers
            const int fifo_num_ready = abstract_fifo_.getNumReady();
            if (to_reset_.exchange(false)) {
                for (size_t i = 0; i < MagNum; ++i) {
                    std::fill(circular_mags_[i].begin(), circular_mags_[i].end(), -240.f);
                }
                const auto scope = abstract_fifo_.read(fifo_num_ready);
                return 0;
            }
            const int num_ready = fifo_num_ready >= static_cast<int>(PointNum / 2)
                                      ? fifo_num_ready
                                      : std::min(fifo_num_ready, num_to_read);
            if (num_ready <= 0) return 0;
            const auto num_ready_shift = static_cast<size_t>(num_ready);
            // shift circular buffers
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circular_peak{circular_mags_[i]};
                std::rotate(circular_peak.begin(),
                            circular_peak.begin() + num_ready_shift,
                            circular_peak.end());
            }
            // read from FIFOs
            const auto scope = abstract_fifo_.read(num_ready);
            for (size_t i = 0; i < MagNum; ++i) {
                auto &circular_peak{circular_mags_[i]};
                auto &peak_fifo{mag_fifos_[i]};
                size_t j = circular_peak.size() - static_cast<size_t>(num_ready);
                if (scope.blockSize1 > 0) {
                    std::copy(&peak_fifo[static_cast<size_t>(scope.startIndex1)],
                              &peak_fifo[static_cast<size_t>(scope.startIndex1 + scope.blockSize1)],
                              &circular_peak[j]);
                    j += static_cast<size_t>(scope.blockSize1);
                }
                if (scope.blockSize2 > 0) {
                    std::copy(&peak_fifo[static_cast<size_t>(scope.startIndex2)],
                              &peak_fifo[static_cast<size_t>(scope.startIndex2 + scope.blockSize2)],
                              &circular_peak[j]);
                }
            }
            return num_ready;
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, MagNum> paths,
                        const juce::Rectangle<float> bound,
                        const float shift = 0.f,
                        const float min_db = -72.f, const float max_db = 0.f) {
            const auto delta_x = bound.getWidth() / static_cast<float>(PointNum - 1);
            const auto x0 = bound.getX(), y0 = bound.getY(), height = bound.getHeight();
            float x = x0 - shift * delta_x;
            for (size_t idx = 0; idx < PointNum; ++idx) {
                for (size_t i = 0; i < MagNum; ++i) {
                    const auto y = magToY(circular_mags_[i][idx], y0, height, min_db, max_db);
                    paths[i].get().lineTo(x, y);
                }
                x += delta_x;
            }
        }

        void setTimeLength(const float x) {
            time_length_.store(x);
            to_update_time_length_.store(true);
        }

        void setToReset() { to_reset_.store(true); }

        void setMagType(const MagType x) { mag_type_.store(x); }

    protected:
        std::atomic<double> sample_rate_{48000.0};
        std::array<std::array<float, PointNum>, MagNum> mag_fifos_{};
        juce::AbstractFifo abstract_fifo_{PointNum};
        std::array<std::array<float, PointNum>, MagNum> circular_mags_{};
        size_t circular_idx_{0};

        std::atomic<float> time_length_{7.f};
        double current_pos_{0.}, max_pos_{1.};
        int current_num_samples_{0};
        std::atomic<bool> to_update_time_length_{true};
        std::array<FloatType, MagNum> current_mags_{};

        std::atomic<bool> to_reset_{false};
        std::atomic<MagType> mag_type_{MagType::kRMS};

        template<MagType CurrentMagType>
        void processBuffer(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, MagNum> &buffers) {
            int start_idx{0}, end_idx{0};
            int num_samples = buffers[0].get().getNumSamples();
            if (num_samples == 0) { return; }
            if (to_update_time_length_.exchange(false)) {
                max_pos_ = sample_rate_.load() * static_cast<double>(time_length_.load()) / static_cast<double>(
                               PointNum - 1);
                current_pos_ = 0;
                current_num_samples_ = 0;
            }
            while (true) {
                const auto remain_num = static_cast<int>(std::round(max_pos_ - current_pos_));
                if (num_samples >= remain_num) {
                    start_idx = end_idx;
                    end_idx = end_idx + remain_num;
                    num_samples -= remain_num;
                    updateMags<CurrentMagType>(buffers, start_idx, remain_num);
                    current_pos_ = current_pos_ + static_cast<double>(remain_num) - max_pos_;
                    if (abstract_fifo_.getFreeSpace() > 0) {
                        const auto scope = abstract_fifo_.write(1);
                        const auto write_idx = scope.blockSize1 > 0 ? scope.startIndex1 : scope.startIndex2;
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
                                    mag_fifos_[i][static_cast<size_t>(write_idx)] = 0.5f * zldsp::chore::gainToDecibels(
                                        static_cast<float>(
                                            current_mags_[i] / static_cast<FloatType>(current_num_samples_)));
                                }
                                current_num_samples_ = 0;
                                break;
                            }
                        }
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

        static float magToY(const float mag, const float y0, const float height,
                            const float min_db, const float max_db) {
            return y0 + height * (max_db - mag) / (max_db - min_db);
        }
    };
}
