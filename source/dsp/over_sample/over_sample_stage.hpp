// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <cstring>
#include <span>
#include <vector>

#include "../vector/vector.hpp"

namespace zldsp::oversample {
    namespace hn = hwy::HWY_NAMESPACE;
    template <typename FloatType>
    class OverSampleStage {
    public:
        explicit OverSampleStage(std::span<const FloatType> up_coeff, std::span<const FloatType> down_coeff) {
            up_coeff_.resize(up_coeff.size() / 2);
            for (size_t i = 1; i < up_coeff.size(); i += 2) {
                up_coeff_[i >> 1] = up_coeff[i] * FloatType(2);
            }
            up_coeff_center_ = up_coeff[up_coeff.size() / 2] * FloatType(2);
            up_coeff_center_pos_ = up_coeff_.size() / 2;

            down_coeff_.resize(down_coeff.size() / 2);
            for (size_t i = 1; i < down_coeff.size(); i += 2) {
                down_coeff_[i >> 1] = down_coeff[i];
            }
            down_coeff_center_ = down_coeff[down_coeff.size() / 2];

            latency_ = (up_coeff.size() + down_coeff.size() - 2) / 4;
        }

        void prepare(const size_t num_channels, const size_t max_num_samples) {
            const size_t up_req_size = up_coeff_.size();
            const size_t down_req_size = down_coeff_.size();

            const size_t up_delay_size = up_req_size + max_num_samples + 1;
            up_delay_lines_.resize(num_channels);
            for (auto& d : up_delay_lines_) {
                d.resize(up_delay_size);
            }

            const size_t down_delay_size = down_req_size + max_num_samples + 1;
            down_delay_lines_.resize(num_channels);
            for (auto& d : down_delay_lines_) {
                d.resize(down_delay_size);
            }

            down_center_delay_lines_.resize(num_channels);
            for (auto& d : down_center_delay_lines_) {
                d.resize(down_coeff_.size() / 2 + max_num_samples);
            }

            os_buffers_.resize(num_channels);
            for (auto& buffer : os_buffers_) {
                buffer.resize(max_num_samples << 1);
            }

            os_pointers_.resize(num_channels);
            for (size_t chan = 0; chan < num_channels; ++chan) {
                os_pointers_[chan] = os_buffers_[chan].data();
            }
            reset();
        }

        void reset() {
            for (auto& d : up_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
            for (auto& d : down_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
            for (auto& d : down_center_delay_lines_) {
                std::fill(d.begin(), d.end(), FloatType(0));
            }
        }

        [[nodiscard]] size_t getLatency() const {
            return latency_;
        }

        template <bool use_simd = false>
        void upsample(std::span<FloatType*> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto delay_line = up_delay_lines_[chan].data();
                auto os_data = os_buffers_[chan].data();
                auto chan_data = buffer[chan];
                std::memcpy(delay_line + up_coeff_.size(), chan_data, num_samples * sizeof(FloatType));

                size_t i = 0;
                if constexpr (use_simd) {
                    static constexpr hn::ScalableTag<FloatType> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    const auto center_coeff = hn::Set(d, up_coeff_center_);
                    for (; i + lanes <= num_samples; i += lanes) {
                        const auto center = hn::Mul(hn::LoadU(d, delay_line + up_coeff_center_pos_ + i), center_coeff);
                        const auto filtered =
                            convolveSymmetric(d, delay_line + i + 1, up_coeff_.data(), up_coeff_.size());
                        hn::StoreInterleaved2(center, filtered, d, os_data + (i << 1));
                    }
                }
                for (; i < num_samples; ++i) {
                    os_data[i << 1] = delay_line[up_coeff_center_pos_ + i] * up_coeff_center_;
                    os_data[(i << 1) + 1] = convolveSymmetric(delay_line + i + 1, up_coeff_.data(), up_coeff_.size());
                }

                std::memmove(delay_line, delay_line + num_samples, up_coeff_.size() * sizeof(FloatType));
            }
        }

        template <bool use_simd = false>
        void downsample(std::span<FloatType*> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto delay_line = down_delay_lines_[chan].data();
                auto center_delay_line = down_center_delay_lines_[chan].data();
                auto os_data = os_buffers_[chan].data();
                auto chan_data = buffer[chan];

                size_t i = 0;
                if constexpr (use_simd) {
                    static constexpr hn::ScalableTag<FloatType> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    for (; i + lanes <= num_samples; i += lanes) {
                        auto center = hn::Zero(d);
                        auto filtered = hn::Zero(d);
                        hn::LoadInterleaved2(d, os_data + (i << 1), center, filtered);
                        hn::StoreU(center, d, center_delay_line + down_coeff_.size() / 2 + i);
                        hn::StoreU(filtered, d, delay_line + down_coeff_.size() + i);
                    }
                }
                for (; i < num_samples; ++i) {
                    center_delay_line[down_coeff_.size() / 2 + i] = os_data[i << 1];
                    delay_line[down_coeff_.size() + i] = os_data[(i << 1) + 1];
                }

                i = 0;
                if constexpr (use_simd) {
                    static constexpr hn::ScalableTag<FloatType> d;
                    static constexpr size_t lanes = hn::MaxLanes(d);
                    const auto center_coeff = hn::Set(d, down_coeff_center_);
                    for (; i + lanes <= num_samples; i += lanes) {
                        const auto center = hn::Mul(hn::LoadU(d, center_delay_line + i), center_coeff);
                        const auto filtered =
                            convolveSymmetric(d, delay_line + i, down_coeff_.data(), down_coeff_.size());
                        hn::StoreU(hn::Add(center, filtered), d, chan_data + i);
                    }
                }
                for (; i < num_samples; ++i) {
                    chan_data[i] = center_delay_line[i] * down_coeff_center_ +
                        convolveSymmetric(delay_line + i, down_coeff_.data(), down_coeff_.size());
                }

                std::memmove(delay_line, delay_line + num_samples, down_coeff_.size() * sizeof(FloatType));
                std::memmove(center_delay_line, center_delay_line + num_samples,
                             down_coeff_.size() / 2 * sizeof(FloatType));
            }
        }

        std::vector<std::vector<FloatType>>& getOSBuffer() {
            return os_buffers_;
        }

        std::vector<FloatType*>& getOSPointer() {
            return os_pointers_;
        }

    private:
        static HWY_INLINE FloatType convolveSymmetric(const FloatType* HWY_RESTRICT input,
                                                      const FloatType* HWY_RESTRICT coeff, const size_t size) {
            FloatType output{FloatType(0)};
            const auto symmetric_size = size >> 1;
            const auto symmetric_shift = size - 1;
            for (size_t i = 0; i < symmetric_size; ++i) {
                output += (input[i] + input[symmetric_shift - i]) * coeff[i];
            }
            return output;
        }

        template <typename D>
        static HWY_INLINE auto convolveSymmetric(const D d, const FloatType* HWY_RESTRICT input,
                                                 const FloatType* HWY_RESTRICT coeff, const size_t size) {
            auto sum0 = hn::Zero(d);
            auto sum1 = hn::Zero(d);
            auto sum2 = hn::Zero(d);
            auto sum3 = hn::Zero(d);
            const auto symmetric_size = size >> 1;
            const auto symmetric_shift = size - 1;

            size_t i = 0;
            for (; i + 4 <= symmetric_size; i += 4) {
                const auto samples0 = hn::Add(hn::LoadU(d, input + i), hn::LoadU(d, input + symmetric_shift - i));
                const auto samples1 =
                    hn::Add(hn::LoadU(d, input + i + 1), hn::LoadU(d, input + symmetric_shift - i - 1));
                const auto samples2 =
                    hn::Add(hn::LoadU(d, input + i + 2), hn::LoadU(d, input + symmetric_shift - i - 2));
                const auto samples3 =
                    hn::Add(hn::LoadU(d, input + i + 3), hn::LoadU(d, input + symmetric_shift - i - 3));
                sum0 = hn::MulAdd(samples0, hn::Set(d, coeff[i]), sum0);
                sum1 = hn::MulAdd(samples1, hn::Set(d, coeff[i + 1]), sum1);
                sum2 = hn::MulAdd(samples2, hn::Set(d, coeff[i + 2]), sum2);
                sum3 = hn::MulAdd(samples3, hn::Set(d, coeff[i + 3]), sum3);
            }

            auto output = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
            for (; i < symmetric_size; ++i) {
                const auto samples = hn::Add(hn::LoadU(d, input + i), hn::LoadU(d, input + symmetric_shift - i));
                output = hn::MulAdd(samples, hn::Set(d, coeff[i]), output);
            }
            return output;
        }

        vector::aligned_vector<FloatType> up_coeff_{};
        FloatType up_coeff_center_{FloatType(0)};
        size_t up_coeff_center_pos_{0};
        std::vector<vector::aligned_vector<FloatType>> up_delay_lines_{};

        vector::aligned_vector<FloatType> down_coeff_{};
        FloatType down_coeff_center_{FloatType(0)};
        std::vector<vector::aligned_vector<FloatType>> down_delay_lines_{};
        std::vector<vector::aligned_vector<FloatType>> down_center_delay_lines_{};

        size_t latency_{0};

        std::vector<std::vector<FloatType>> os_buffers_{};
        std::vector<FloatType*> os_pointers_{};
    };
}
