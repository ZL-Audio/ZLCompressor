// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../fft/zldsp_fft_include.hpp"
#include "../../fft/zldsp_fft_window.hpp"
#include "../../vector/vector.hpp"
#include "../../container/fifo/fifo_base.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    namespace hn = hwy::HWY_NAMESPACE;
    /**
     * a fft analyzer receiver which pulls input samples from FIFOs and runs forward FFT
     * @tparam kNum the number of FFTs
     */
    template <size_t kNum>
    class FFTAnalyzerReceiver {
    public:
        explicit FFTAnalyzerReceiver() = default;

        /**
         *
         * @param order FFT order
         * @param num_channels number of channels
         */
        void prepare(const int order, std::array<size_t, kNum> num_channels) {
            setOrder(order, num_channels);
        }

        /**
         * pull data from FIFO into circular buffer
         * @param range
         * @param sample_fifos
         */
        void pull(const zldsp::container::FIFORange range,
                  std::array<std::vector<std::vector<float>>, kNum>& sample_fifos) {
            const auto num_ready = range.block_size1 + range.block_size2;
            const auto num_replace = static_cast<int>(fft_->get_size()) - num_ready;
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
                        vector::multiply(fft_in_.data(), circular_buffers_[i][chan].data(),
                            window_.data(), window_.size());
                        if (chan == 0) {
                            fft_->forward_sqr_mag(fft_in_.data(), abs_sqr_fft_buffers_[i].data());
                        } else {
                            fft_->forward_sqr_mag(fft_in_.data(), fft_out_.data());
                            vector::add(abs_sqr_fft_buffers_[i].data(), fft_out_.data(), fft_out_.size());
                        }
                    }
                } else {
                    if (stereo_type == StereoType::kLeft) {
                        vector::multiply(fft_in_.data(), circular_buffers_[i][0].data(),
                            window_.data(), fft_->get_size());
                    } else if (stereo_type == StereoType::kRight) {
                        vector::multiply(fft_in_.data(), circular_buffers_[i][1].data(),
                            window_.data(), fft_->get_size());
                    } else if (stereo_type == StereoType::kMid) {
                        static constexpr hn::ScalableTag<float> d;
                        static constexpr size_t lanes = hn::MaxLanes(d);
                        float* __restrict fft_in_ptr = fft_in_.data();
                        const float* __restrict in0_ptr = circular_buffers_[i][0].data();
                        const float* __restrict in1_ptr = circular_buffers_[i][1].data();
                        const float* __restrict window_ptr = window_.data();
                        const auto v_sqrt_over_2 = hn::Set(d, kSqrt2Over2);
                        for (size_t j = 0; j < fft_->get_size(); j += lanes) {
                            const auto v_in0 = hn::LoadU(d, in0_ptr + j);
                            const auto v_in1 = hn::LoadU(d, in1_ptr + j);
                            const auto v_window = hn::LoadU(d, window_ptr + j);
                            const auto v_out = hn::Mul(hn::Add(v_in0, v_in1), v_sqrt_over_2);
                            hn::StoreU(hn::Mul(v_out, v_window), d, fft_in_ptr + j);
                        }
                    } else {
                        static constexpr hn::ScalableTag<float> d;
                        static constexpr size_t lanes = hn::MaxLanes(d);
                        float* __restrict fft_in_ptr = fft_in_.data();
                        const float* __restrict in0_ptr = circular_buffers_[i][0].data();
                        const float* __restrict in1_ptr = circular_buffers_[i][1].data();
                        const float* __restrict window_ptr = window_.data();
                        const auto v_sqrt_over_2 = hn::Set(d, kSqrt2Over2);
                        for (size_t j = 0; j < fft_->get_size(); j += lanes) {
                            const auto v_in0 = hn::LoadU(d, in0_ptr + j);
                            const auto v_in1 = hn::LoadU(d, in1_ptr + j);
                            const auto v_window = hn::LoadU(d, window_ptr + j);
                            const auto v_out = hn::Mul(hn::Sub(v_in0, v_in1), v_sqrt_over_2);
                            hn::StoreU(hn::Mul(v_out, v_window), d, fft_in_ptr + j);
                        }
                    }
                    fft_->forward_sqr_mag(fft_in_.data(), abs_sqr_fft_buffers_[i].data());
                }
            }
        }

        void setON(std::array<bool, kNum> is_on) {
            is_on_ = is_on;
        }

        [[nodiscard]] size_t getFFTSize() const {
            return fft_->get_size();
        }

        /**
         * get absolute square spectrum
         * @return
         */
        std::array<vector::aligned_vector<float>, kNum>& getAbsSqrFFTBuffers() {
            return abs_sqr_fft_buffers_;
        }

    protected:
        std::array<std::vector<vector::aligned_vector<float>>, kNum> circular_buffers_;

        vector::aligned_vector<float> fft_in_;
        vector::aligned_vector<float> fft_out_;
        std::array<vector::aligned_vector<float>, kNum> abs_sqr_fft_buffers_;

        std::unique_ptr<zldsp::fft::RFFT<float>> fft_;
        vector::aligned_vector<float> window_;

        std::array<bool, kNum> is_on_{};

        void setOrder(const int fft_order, std::array<size_t, kNum>& num_channels) {
            fft_ = std::make_unique<zldsp::fft::RFFT<float>>(fft_order);
            const auto fft_size = fft_->get_size();

            window_.resize(fft_size);
            zldsp::fft::create_periodic_hanning(std::span{window_.data(), window_.size()});
            const auto scale = 1.f / static_cast<float>(fft_size);
            vector::multiply(window_.data(), scale, window_.size());
            fft_in_.resize(fft_size);
            fft_out_.resize(fft_size / 2 + 1);
            for (size_t i = 0; i < kNum; ++i) {
                abs_sqr_fft_buffers_[i].resize(fft_size / 2 + 1);
            }
            for (size_t i = 0; i < kNum; ++i) {
                circular_buffers_[i].resize(num_channels[i]);
                for (size_t chan = 0; chan < num_channels[i]; ++chan) {
                    circular_buffers_[i][chan].resize(fft_size);
                    std::fill(circular_buffers_[i][chan].begin(), circular_buffers_[i][chan].end(), 0.f);
                }
            }
        }
    };
}
