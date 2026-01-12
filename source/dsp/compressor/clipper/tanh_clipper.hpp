// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <atomic>

#include "../../chore/decibels.hpp"

namespace zldsp::compressor {
    template <typename FloatType>
    class TanhClipper {
    public:
        TanhClipper() = default;

        void setReductionAtUnit(FloatType db) {
            reduction_db_at_unit_ = static_cast<double>(db);
            to_update_k_.store(true, std::memory_order::release);
        }

        void prepareBuffer() {
            if (to_update_k_.exchange(false, std::memory_order::acquire)) {
                c_wet_ = wet_.load(std::memory_order::relaxed) / 100.0;
                is_on_ = c_wet_ > 1e-5;
                if (is_on_) {
                    updateActualK();
                }
            }
        }

        void process(FloatType* buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto v = static_cast<double>(buffer[i]);
                buffer[i] = static_cast<FloatType>(std::tanh(v * k1_) * k2_);
            }
        }

        FloatType processSample(FloatType x) const {
            return static_cast<FloatType>(std::tanh(static_cast<double>(x) * k1_) * k2_);
        }

        void setWet(const FloatType wet) {
            wet_.store(std::clamp(static_cast<double>(wet), 0.0, 100.0), std::memory_order::relaxed);
            to_update_k_.store(true, std::memory_order::release);
        }

        bool getIsON() const {
            return is_on_;
        }

    private:
        static constexpr double kLowThres = 0.999, kHighThres = 1.001;

        double reduction_db_at_unit_{-1.0};
        std::atomic<double> wet_{0.0};
        std::atomic<bool> to_update_k_{true};
        double c_wet_{0.0};
        bool is_on_{false};
        double k1_{1.0}, k2_{1.0};

        void updateActualK() {
            const auto final_reduction = std::clamp(reduction_db_at_unit_, -80.0, 0.0) * c_wet_;
            const auto final_reduction_gain = chore::decibelsToGain(final_reduction);
            if (final_reduction > -2.895296501106843e-08) {
                k1_ = 1e-4;
            }
            else if (final_reduction > -2.895295870818236e-06) {
                k1_ = getK(final_reduction_gain, 1e-4, 1e-3);
            }
            else if (final_reduction > -0.0002895228991000713) {
                k1_ = getK(final_reduction_gain, 1e-3, 1e-2);
            }
            else if (final_reduction > -0.02888559791501009) {
                k1_ = getK(final_reduction_gain, 1e-2, 1e-1);
            }
            else if (final_reduction > -2.3655279428366893) {
                k1_ = getK(final_reduction_gain, 1e-1, 1.0);
            }
            else if (final_reduction > -20.000000035805904) {
                k1_ = getK(final_reduction_gain, 1.0, 10.0);
            }
            else if (final_reduction > -40.0) {
                k1_ = getK(final_reduction_gain, 10.0, 100.0);
            }
            else {
                k1_ = 1.0 / final_reduction_gain;
            }
            k2_ = 1.0 / k1_;
        }

        static double evalP(const double y, const double k) {
            return std::tanh(k) / k / y;
        }

        static double getK(const double y, double k_left, double k_right) {
            if (evalP(y, k_right) >= kLowThres) {
                return k_right;
            }
            else if (evalP(y, k_left) <= kHighThres) {
                return k_left;
            }
            size_t i = 0;
            while (i < 100) {
                i += 1;
                const auto k = std::sqrt(k_left * k_right);
                const auto p = evalP(y, k);
                if (p > kHighThres) {
                    k_left = k;
                }
                else if (p < kLowThres) {
                    k_right = k;
                }
                else {
                    return k;
                }
            }
            return std::sqrt(k_left * k_right);
        }
    };
}
