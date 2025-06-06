// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <atomic>

#include "../vector/kfr_import.hpp"
#include "../chore/decibels.hpp"
#include "../container/abstract_fifo.hpp"

namespace zldsp::analyzer {
    enum MagType {
        kPeak, kRMS
    };

    template<typename FloatType, size_t MagNum, size_t PointNum>
    class MultipleMagBase {
    public:
        explicit MultipleMagBase() = default;

        virtual ~MultipleMagBase() = default;

        virtual void prepare(double sample_rate) = 0;

        void process(std::array<std::span<FloatType *>, MagNum> buffers,
                     const size_t num_samples) {
            switch (mag_type_.load(std::memory_order::relaxed)) {
                case MagType::kPeak: {
                    processBuffer<MagType::kPeak>(buffers, static_cast<int>(num_samples));
                    break;
                }
                case MagType::kRMS: {
                    processBuffer<MagType::kRMS>(buffers, static_cast<int>(num_samples));
                    break;
                }
                default: {
                }
            }
        }

        void setTimeLength(const float x) {
            time_length_.store(x, std::memory_order::relaxed);
            to_update_time_length_.store(true, std::memory_order::release);
        }

        void setToReset() { to_reset_.store(true, std::memory_order::release); }

        void setMagType(const MagType x) { mag_type_.store(x, std::memory_order::relaxed); }

    protected:
        std::atomic<double> sample_rate_{48000.0};
        std::array<std::array<float, PointNum>, MagNum> mag_fifos_{};
        zldsp::container::AbstractFIFO abstract_fifo_{PointNum};
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
        void processBuffer(std::array<std::span<FloatType *>, MagNum> &buffers, int num_samples) {
            int start_idx{0}, end_idx{0};
            if (num_samples == 0) { return; }
            if (to_update_time_length_.exchange(false, std::memory_order::acquire)) {
                max_pos_ = sample_rate_.load(std::memory_order::relaxed) * static_cast<double>(
                               time_length_.load(std::memory_order::relaxed)) / static_cast<double>(PointNum - 1);
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
                    if (abstract_fifo_.getNumFree() > 0) {
                        const auto range = abstract_fifo_.prepareToWrite(1);
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
                                    mag_fifos_[i][static_cast<size_t>(write_idx)] = 0.5f * zldsp::chore::gainToDecibels(
                                            static_cast<float>(
                                                current_mags_[i] / static_cast<FloatType>(current_num_samples_)));
                                }
                                current_num_samples_ = 0;
                                break;
                            }
                        }
                        abstract_fifo_.finishWrite(1);

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
        void updateMags(std::array<std::span<FloatType *>, MagNum> &buffers,
                        const int start_idx, const int num_samples) {
            for (size_t i = 0; i < MagNum; ++i) {
                auto &buffer = buffers[i];
                for (size_t chan = 0; chan < buffer.size(); ++chan) {
                    auto v = kfr::make_univector(
                        buffer[chan] + static_cast<size_t>(start_idx),
                        static_cast<size_t>(num_samples));
                    switch (CurrentMagType) {
                        case MagType::kPeak: {
                            current_mags_[i] = std::max(current_mags_[i], kfr::absmaxof(v));
                            break;
                        }
                        case MagType::kRMS: {
                            current_mags_[i] += kfr::sumsqr(v);
                            current_num_samples_ += num_samples;
                            break;
                        }
                    }
                }
            }
        }
    };
}
