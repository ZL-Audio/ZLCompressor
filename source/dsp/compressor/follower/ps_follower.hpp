// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <numbers>
#include <cmath>
#include <algorithm>

namespace zldsp::compressor {
    /**
     * a punch-smooth follower
     * @tparam FloatType
     * @tparam UseSmooth whether to use smooth
     * @tparam UsePunch whether to use punch
     */
    template<typename FloatType, bool UseSmooth = false, bool UsePunch = false>
    class PSFollower {
    public:
        PSFollower() = default;

        /**
         * call before processing starts
         * @param sr sampleRate
         */
        void prepare(const double sr) {
            exp_factor = -2.0 * std::numbers::pi * 1000.0 / sr;
            to_update.store(true);
        }

        /**
         * reset the follower
         */
        void reset(const FloatType x) {
            y = x;
            state = x;
            slope = FloatType(0);
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (to_update.exchange(false)) {
                update();
            }
        }

        /**
         * process a sample
         * @param x
         * @return
         */
        FloatType processSample(const FloatType x) {
            FloatType y0;
            if (UseSmooth) {
                state = std::max(x, release * state + release_c * x);
                const auto y1 = attack * y + attack_c * state;
                const auto y2 = x >= y ? attack * y + attack_c * x : release * y + release_c * x;
                y0 = smooth * y1 + smooth_c * y2;
            } else {
                y0 = x >= y ? attack * y + attack_c * x : release * y + release_c * x;
            }
            if (UsePunch) {
                const auto slope0 = y0 - y;
                if (punch >= FloatType(0)) {
                    if (slope0 < slope) {
                        slope = punch * slope + punch_c * slope0;
                    } else {
                        slope = slope0;
                    }
                } else {
                    if (slope0 > slope && slope >= FloatType(0)) {
                        slope = punch * slope + punch_c * slope0;
                    } else {
                        slope = slope0;
                    }
                }
                y += slope;
            } else {
                y = y0;
            }
            return y;
        }

        void setAttack(const FloatType millisecond) {
            attack_time.store(std::max(FloatType(0), millisecond));
            to_update.store(true);
        }

        void setRelease(const FloatType millisecond) {
            release_time.store(std::max(FloatType(0), millisecond));
            to_update.store(true);
        }

        void setPunch(const FloatType x) {
            punch_portion.store(std::clamp(x, FloatType(-1), FloatType(1)));
            to_update.store(true);
        }

        void setSmooth(const FloatType x) {
            smooth_portion.store(std::clamp(x, FloatType(0), FloatType(1)));
            to_update.store(true);
        }

    private:
        FloatType y{}, state{}, slope{};
        FloatType attack{}, attack_c{}, release{}, release_c{};
        FloatType punch{}, punch_c{}, smooth{}, smooth_c{};
        double exp_factor{-0.1308996938995747};
        std::atomic<FloatType> attack_time{50}, release_time{100}, smooth_portion{0}, punch_portion{0};
        std::atomic<bool> to_update{true};

        void update() {
            // cache atomic values
            const auto currentAttackTime = static_cast<double>(attack_time.load());
            const auto currentReleaseTime = static_cast<double>(release_time.load());
            const auto currentSmoothPortion = static_cast<double>(smooth_portion.load());
            const auto currentPunchPortion = static_cast<double>(punch_portion.load());
            // update attack
            if (currentAttackTime < 0.001) {
                attack = FloatType(0);
            } else {
                if (UsePunch) {
                    attack = static_cast<FloatType>(std::exp(
                        exp_factor / currentAttackTime / (1. - std::pow(std::abs(currentPunchPortion), 2.) * 0.125)));
                } else {
                    attack = static_cast<FloatType>(std::exp(exp_factor / currentAttackTime));
                }
            }
            attack_c = FloatType(1) - attack;
            // update release
            if (currentReleaseTime < 0.001) {
                release = FloatType(0);
            } else {
                release = static_cast<FloatType>(std::exp(exp_factor / currentReleaseTime));
            }
            release_c = FloatType(1) - release;
            if (UseSmooth) {
                // update smooth
                smooth = static_cast<FloatType>(currentSmoothPortion);
                smooth_c = FloatType(1) - smooth;
            }
            if (UsePunch) {
                // update punch
                if (currentAttackTime < 0.001 || std::abs(currentPunchPortion) < 0.001) {
                    punch = FloatType(0);
                } else {
                    punch = static_cast<FloatType>(std::exp(
                        exp_factor / currentAttackTime / std::abs(currentPunchPortion)));
                }
                punch_c = FloatType(1) - punch;
            }
        }
    };
}
