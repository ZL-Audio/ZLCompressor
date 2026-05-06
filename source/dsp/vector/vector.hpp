// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <vector>
#include <algorithm>

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma GCC diagnostic ignored "-Wshadow-field-in-constructor"
#endif
#include <hwy/highway.h>
#include <hwy/aligned_allocator.h>
#include <hwy/contrib/math/math-inl.h>
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace zldsp::vector {
    namespace hn = hwy::HWY_NAMESPACE;

    template <typename F>
    using aligned_vector = std::vector<F, hwy::AlignedAllocator<F>>;

    template <typename F>
    HWY_INLINE void copy(F* __restrict out, const F* __restrict in, const size_t size) {
        std::memcpy(out, in, sizeof(F) * size);
    }

    template <typename F>
    HWY_INLINE void copy(std::span<F*> outs, std::span<F*> ins, const size_t size) {
        for (size_t i = 0; i < std::min(outs.size(), ins.size()); ++i) {
            copy<F>(outs[i], ins[i], size);
        }
    }

    template <typename F1, typename F2>
    HWY_INLINE void copy(F1* __restrict out, const F2* __restrict in, const size_t size) {
        for (size_t i = 0; i < size; ++i) {
            out[i] = static_cast<F1>(in[i]);
        }
    }

    template <typename F1, typename F2>
    HWY_INLINE void copy(std::span<F1*> outs, std::span<F2*> ins, const size_t size) {
        for (size_t i = 0; i < std::min(outs.size(), ins.size()); ++i) {
            copy<F1, F2>(outs[i], ins[i], size);
        }
    }

    template <typename F>
    HWY_INLINE void add(F* HWY_RESTRICT in, const F to_add, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_add = hn::Set(d, to_add);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            const auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::Add(v_in, v_add), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] += to_add;
        }
    }

    template <typename F>
    HWY_INLINE void add(F* HWY_RESTRICT in, const F* HWY_RESTRICT to_add, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_add = hn::LoadU(d, to_add + i);
            hn::StoreU(hn::Add(v_in, v_add), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] += to_add[i];
        }
    }

    template <typename F>
    HWY_INLINE void sub(F* HWY_RESTRICT in, const F* HWY_RESTRICT to_sub, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_add = hn::LoadU(d, to_sub + i);
            hn::StoreU(hn::Sub(v_in, v_add), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] -= to_sub[i];
        }
    }

    template <typename F>
    HWY_INLINE void sub(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const F* HWY_RESTRICT to_sub, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_add = hn::LoadU(d, to_sub + i);
            hn::StoreU(hn::Sub(v_in, v_add), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = in[i] - to_sub[i];
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT in, const F to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_m = hn::Set(d, to_mul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::Mul(v_in, v_m), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] *= to_mul;
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const F to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_m = hn::Set(d, to_mul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            hn::StoreU(hn::Mul(v_in, v_m), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = in[i] * to_mul;
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT in, const F* HWY_RESTRICT to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_mul = hn::LoadU(d, to_mul + i);
            hn::StoreU(hn::Mul(v_in, v_mul), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] *= to_mul[i];
        }
    }

    template <typename F>
    HWY_INLINE void multiply(F* HWY_RESTRICT out, const F* HWY_RESTRICT in, const F* HWY_RESTRICT to_mul, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_mul = hn::LoadU(d, to_mul + i);
            hn::StoreU(hn::Mul(v_in, v_mul), d, out + i);
        }
        for (; i < size; ++i) {
            out[i] = in[i] * to_mul[i];
        }
    }

    template <typename F>
    HWY_INLINE void clamp(F* HWY_RESTRICT in, const F lo, const F hi, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        const auto v_lo = hn::Set(d, lo);
        const auto v_hi = hn::Set(d, hi);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_clamped = hn::Max(v_lo, hn::Min(v_in, v_hi));
            hn::StoreU(hn::Clamp(v_in, v_lo, v_hi), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = std::clamp(in[i], lo, hi);
        }
    }

    template <typename F>
    HWY_INLINE F sum(const F* HWY_RESTRICT in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr size_t block = lanes << 2;

        size_t i = 0;
        hn::Vec<decltype(d)> single_sum;
        if (size >= block) {
            auto sum0 = hn::Zero(d);
            auto sum1 = hn::Zero(d);
            auto sum2 = hn::Zero(d);
            auto sum3 = hn::Zero(d);
            for (; i + block <= size; i += block) {
                sum0 = hn::Add(sum0, hn::LoadU(d, in + i));
                sum1 = hn::Add(sum1, hn::LoadU(d, in + i + lanes));
                sum2 = hn::Add(sum2, hn::LoadU(d, in + i + lanes * 2));
                sum3 = hn::Add(sum3, hn::LoadU(d, in + i + lanes * 3));
            }
            single_sum = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
        } else {
            single_sum = hn::Zero(d);
        }
        for (; i + lanes <= size; i += lanes) {
            auto va = hn::LoadU(d, in + i);
            single_sum = hn::Add(va, single_sum);
        }
        F scalar_sum = hn::ReduceSum(d, single_sum);
        for (; i < size; ++i) {
            scalar_sum += in[i];
        }
        return scalar_sum;
    }

    template <typename F>
    HWY_INLINE F sum_sqr(const F* HWY_RESTRICT in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr size_t block = lanes << 2;

        size_t i = 0;
        hn::Vec<decltype(d)> single_sum;
        if (size >= block) {
            auto sum0 = hn::Zero(d);
            auto sum1 = hn::Zero(d);
            auto sum2 = hn::Zero(d);
            auto sum3 = hn::Zero(d);
            for (; i + block <= size; i += block) {
                {
                    auto va = hn::LoadU(d, in + i);
                    sum0 = hn::MulAdd(va, va, sum0);
                }
                {
                    auto va = hn::LoadU(d, in + i + lanes);
                    sum1 = hn::MulAdd(va, va, sum1);
                }
                {
                    auto va = hn::LoadU(d, in + i + lanes * 2);
                    sum2 = hn::MulAdd(va, va, sum2);
                }
                {
                    auto va = hn::LoadU(d, in + i + lanes * 3);
                    sum3 = hn::MulAdd(va, va, sum3);
                }
            }
            single_sum = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
        } else {
            single_sum = hn::Zero(d);
        }
        for (; i + lanes <= size; i += lanes) {
            auto va = hn::LoadU(d, in + i);
            single_sum = hn::MulAdd(va, va, single_sum);
        }
        F scalar_sum = hn::ReduceSum(d, single_sum);
        for (; i < size; ++i) {
            scalar_sum += in[i] * in[i];
        }
        return scalar_sum;
    }

    template <typename F>
    HWY_INLINE F dot_product(const F* HWY_RESTRICT in0, const F* HWY_RESTRICT in1,
                             const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr size_t block = lanes << 2;

        size_t i = 0;
        hn::Vec<decltype(d)> single_sum;
        if (size >= block) {
            auto sum0 = hn::Zero(d);
            auto sum1 = hn::Zero(d);
            auto sum2 = hn::Zero(d);
            auto sum3 = hn::Zero(d);
            for (; i + block <= size; i += block) {
                {
                    auto va = hn::LoadU(d, in0 + i);
                    auto vb = hn::LoadU(d, in1 + i);
                    sum0 = hn::MulAdd(va, vb, sum0);
                }
                {
                    auto va = hn::LoadU(d, in0 + i + lanes);
                    auto vb = hn::LoadU(d, in1 + i + lanes);
                    sum1 = hn::MulAdd(va, vb, sum1);
                }
                {
                    auto va = hn::LoadU(d, in0 + i + lanes * 2);
                    auto vb = hn::LoadU(d, in1 + i + lanes * 2);
                    sum2 = hn::MulAdd(va, vb, sum2);
                }
                {
                    auto va = hn::LoadU(d, in0 + i + lanes * 3);
                    auto vb = hn::LoadU(d, in1 + i + lanes * 3);
                    sum3 = hn::MulAdd(va, vb, sum3);
                }
            }
            single_sum = hn::Add(hn::Add(sum0, sum1), hn::Add(sum2, sum3));
        } else {
            single_sum = hn::Zero(d);
        }
        for (; i + lanes <= size; i += lanes) {
            auto va = hn::LoadU(d, in0 + i);
            auto vb = hn::LoadU(d, in1 + i);
            single_sum = hn::MulAdd(va, vb, single_sum);
        }
        F scalar_sum = hn::ReduceSum(d, single_sum);
        for (; i < size; ++i) {
            scalar_sum += in0[i] * in1[i];
        }
        return scalar_sum;
    }

    template <typename F>
    HWY_INLINE void sqr_mag_to_db(F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr auto kLogSqrMin = static_cast<F>(1e-24);
        static constexpr auto kLogSqrMul = static_cast<F>(4.3429448190325182765);
        const auto v_min = hn::Set(d, kLogSqrMin);
        const auto v_multiplier = hn::Set(d, kLogSqrMul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_log = hn::CallLog(d, hn::Max(v_in, v_min));
            hn::StoreU(hn::Mul(v_log, v_multiplier), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = kLogSqrMul * std::log(std::max(in[i], kLogSqrMin));
        }
    }

    template <typename F>
    HWY_INLINE void mag_to_db(F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        static constexpr auto kLogMin = static_cast<F>(1e-12);
        static constexpr auto kLogMul = static_cast<F>(8.6858896380650365530);
        const auto v_min = hn::Set(d, kLogMin);
        const auto v_multiplier = hn::Set(d, kLogMul);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_log = hn::CallLog(d, hn::Max(v_in, v_min));
            hn::StoreU(hn::Mul(v_log, v_multiplier), d, in + i);
        }
        for (; i < size; ++i) {
            in[i] = kLogMul * std::log(std::max(in[i], kLogMin));
        }
    }

    template <typename F>
    HWY_INLINE F max_of(const F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        auto v_max = hn::Set(d, std::numeric_limits<F>::lowest());
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            v_max = hn::Max(v_max, v_in);
        }
        F scalar_max = hn::ReduceMax(d, v_max);
        for (; i < size; ++i) {
            scalar_max = std::max(scalar_max, in[i]);
        }
        return scalar_max;
    }

    template <typename F>
    HWY_INLINE F max_abs_of(const F* __restrict in, const size_t size) {
        static constexpr hn::ScalableTag<F> d;
        static constexpr size_t lanes = hn::MaxLanes(d);
        auto v_max_abs = hn::Zero(d);
        size_t i = 0;
        for (; i + lanes <= size; i += lanes) {
            auto v_in = hn::LoadU(d, in + i);
            auto v_abs = hn::Abs(v_in);
            v_max_abs = hn::Max(v_max_abs, v_abs);
        }
        F scalar_max_abs = hn::ReduceMax(d, v_max_abs);
        for (; i < size; ++i) {
            scalar_max_abs = std::max(scalar_max_abs, std::abs(in[i]));
        }
        return scalar_max_abs;
    }
}
