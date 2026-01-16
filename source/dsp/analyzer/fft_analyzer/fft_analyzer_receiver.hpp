// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../container/abstract_fifo.hpp"
#include "../../fft/kfr_engine.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    /**
     * a fft analyzer receiver which pulls input samples from FIFOs and runs forward FFT
     * @tparam FloatType the float type of input audio buffers
     * @tparam kNum the number of FFTs
     */
    template <typename FloatType, size_t kNum>
    class MultipleFFTBaseReceiver {
    public:
        explicit MultipleFFTBaseReceiver() = default;

        /**
         *
         * @param sample_rate sample rate
         * @param order FFT order
         * @param num_channels number of channels
         */
        void prepare(const double sample_rate, const int order, std::array<size_t, kNum> num_channels) {
            sample_rate_ = sample_rate;
            setOrder(order, num_channels);
            updateTilt();
        }

        /**
         * pull data from FIFO into circular buffer
         * @param range
         * @param sample_fifos
         */
        void pull(const zldsp::container::AbstractFIFO::Range range,
                  std::array<std::vector<std::vector<float>>, kNum>& sample_fifos) {
            const auto num_replace = static_cast<int>(fft_.getSize()) - (range.block_size1 + range.block_size2);
            for (size_t i = 0; i < kNum; ++i) {
                if (!is_on_[i]) { continue; }
                for (size_t chan = 0; chan < circular_buffers_[i].size(); ++chan) {
                    auto& circular_buffer{circular_buffers_[i][chan]};
                    auto& sample_fifo{sample_fifos[i][chan]};
                    std::memmove(circular_buffer.data(),
                                 circular_buffer.data() + static_cast<std::ptrdiff_t>(num_ready),
                                 sizeof(float) * static_cast<size_t>(num_replace));
                    if (range.block_size1 > 0) {
                        std::copy(sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index1),
                                  sample_fifo.begin() + static_cast<std::ptrdiff_t>(
                                      range.start_index1 + range.block_size1),
                                  circular_buffer.begin() + static_cast<std::ptrdiff_t>(num_replace));
                    }
                    if (range.block_size2 > 0) {
                        std::copy(sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index2),
                                  sample_fifo.begin() + static_cast<std::ptrdiff_t>(
                                      range.start_index2 + range.block_size2),
                                  circular_buffer.begin() + static_cast<std::ptrdiff_t>(
                                      num_replace + range.block_size1));
                    }
                }
            }
        }

        /**
         * run forward FFT to get the absolute square spectrum
         * @param stereo_type
         */
        void forward(const StereoType stereo_type) {
            // run forward FFT & apply tilt
            for (size_t i = 0; i < kNum; ++i) {
                if (!is_on_[i]) { continue; }
                if (circular_buffers_[i].size() != 2 || stereo_type == StereoType::kStereo) {
                    for (size_t chan = 0; chan < circular_buffers_[i].size(); ++chan) {
                        fft_in_ = circular_buffers_[i][chan] * window_;
                        fft_.forward(fft_in_, fft_out_);
                        if (chan == 0) {
                            abs_sqr_fft_buffers_[i] = kfr::cabssqr(fft_out_);
                        } else {
                            abs_sqr_fft_buffers_[i] = abs_sqr_fft_buffers_[i] + kfr::cabssqr(fft_out_);
                        }
                    }
                    abs_sqr_fft_buffers_[i] = abs_sqr_fft_buffers_[i] * tilt_shift_;
                } else {
                    if (stereo_type == StereoType::kLeft) {
                        fft_in_ = circular_buffers_[i][0] * window_;
                    } else if (stereo_type == StereoType::kRight) {
                        fft_in_ = circular_buffers_[i][1] * window_;
                    } else if (stereo_type == StereoType::kMid) {
                        fft_in_ = kSqrt2Over2 * (circular_buffers_[i][0] + circular_buffers_[i][1]) * window_;
                    } else {
                        fft_in_ = kSqrt2Over2 * (circular_buffers_[i][0] - circular_buffers_[i][1]) * window_;
                    }
                    fft_.forward(fft_in_, fft_out_);
                    abs_sqr_fft_buffers_[i] = kfr::cabssqr(fft_out_) * tilt_shift_;
                }
            }
        }

        void setON(std::array<bool, kNum>& is_on) {
            is_on_ = is_on;
        }

        void setTiltSlope(const float x) {
            tilt_slope_ = x;
            updateTilt();
        }

        /**
         * get absolute square spectrum
         * @return
         */
        std::array<kfr::univector<float>, kNum>& getAbsSqrFFTBuffers() {
            return abs_sqr_fft_buffers_;
        }

    protected:
        std::array<std::vector<kfr::univector<float>>, kNum> circular_buffers_;

        kfr::univector<float> fft_in_;
        kfr::univector<std::complex<float>> fft_out_;
        std::array<kfr::univector<float>, kNum> abs_sqr_fft_buffers_;

        zldsp::fft::KFREngine<float> fft_;
        kfr::univector<float> window_;

        std::array<bool, kNum> is_on_{};

        double sample_rate_{48000.0};
        float tilt_slope_{4.5f};
        kfr::univector<float> tilt_shift_{};

        void setOrder(const int fft_order, std::array<size_t, kNum>& num_channels) {
            fft_.setOrder(static_cast<size_t>(fft_order));
            const auto fft_size = fft_.getSize();

            window_.resize(fft_size);
            zldsp::fft::fillCycleHanningWindow(window_, static_cast<size_t>(fft_size));
            const auto scale = 1.f / static_cast<float>(fft_size);
            window_ = window_ * scale;

            fft_in_.resize(fft_size);
            fft_out_.resize(fft_size / 2 + 1);
            for (size_t i = 0; i < kNum; ++i) {
                abs_sqr_fft_buffers_[i].resize(fft_size / 2 + 1);
            }
            tilt_shift_.resize(fft_size / 2 + 1);

            for (size_t i = 0; i < kNum; ++i) {
                circular_buffers_[i].resize(num_channels[i]);
                for (size_t chan = 0; chan < num_channels[i]; ++chan) {
                    circular_buffers_[i][chan].resize(fft_size);
                    std::fill(circular_buffers_[i][chan].begin(), circular_buffers_[i][chan].end(), 0.f);
                }
            }
        }

        void updateTilt() {
            const auto delta = sample_rate_ / static_cast<double>(fft_.getSize());
            const auto exponent = (tilt_slope_ * 0.1) / std::log10(2.0);
            const auto base_scalar = std::pow(delta * 0.001, exponent);
            for (size_t i = 1; i < tilt_shift_.size(); ++i) {
                tilt_shift_[i] = static_cast<float>(base_scalar * std::pow(i, exponent));
            }
            tilt_shift_[0] = tilt_shift_[1];
        }
    };
}
