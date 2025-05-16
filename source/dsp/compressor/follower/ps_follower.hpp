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
    enum PPState { kOff, kPunch, kPump };

    /**
     * a punch-smooth follower
     * @tparam FloatType
     * @tparam UseSmooth whether to use smooth
     * @tparam UsePP whether to use pump-punch
     */
    template<typename FloatType, bool UseSmooth = false, bool UsePP = false>
    class PSFollower {
    public:
        PSFollower() = default;

        /**
         * call before processing starts
         * @param sr sampleRate
         */
        void prepare(const double sr) {
            exp_factor_ = -2.0 * std::numbers::pi * 1000.0 / sr;
            to_update_.store(true);
        }

        /**
         * reset the follower
         */
        void reset(const FloatType x) {
            y_ = x;
            state_ = x;
            slope_ = FloatType(0);
        }

        /**
         * update values before processing a buffer
         */
        bool prepareBuffer() {
            if (to_update_.exchange(false)) {
                update();
                return true;
            }
            return false;
        }

        /**
         * update values before processing a buffer by copying parameters from another follower
         */
        void copyFrom(PSFollower &other) {
            attack_ = other.attack_;
            release_ = other.release_;
            if (UseSmooth) {
                smooth_ = other.smooth_;
            }
            if (UsePP) {
                pp_state_ = other.pp_state_;
                pp_ = other.pp_;
            }
        }

        /**
         * process a sample
         * @param x
         * @return
         */
        template<PPState CurrentPPState = PPState::kOff>
        FloatType processSample(const FloatType x) {
            FloatType y0;
            if (UseSmooth) {
                state_ = std::max(x, release_ * (state_ - x) + x);
                const auto y1 = attack_ * (y_ - state_) + state_;
                const auto y2 = x >= y_ ? attack_ * (y_ - x) + x : release_ * (y_ - x) + x;
                y0 = smooth_ * (y1 - y2) + y2;
            } else {
                y0 = x >= y_ ? attack_ * (y_ - x) + x : release_ * (y_ - x) + x;
            }
            if (UsePP) {
                const auto slope0 = y0 - y_;
                switch (CurrentPPState) {
                    case PPState::kOff: {
                        slope_ = slope0;
                        y_ = y0;
                        break;
                    }
                    case PPState::kPump: {
                        if (slope0 < slope_) {
                            slope_ = pp_ * (slope_ - slope0) + slope0;
                        } else {
                            slope_ = slope0;
                        }
                        y_ += slope_;
                        break;
                    }
                    case PPState::kPunch: {
                        if (slope0 > slope_ && slope_ >= FloatType(0)) {
                            slope_ = pp_ * (slope_ - slope0) + slope0;
                        } else {
                            slope_ = slope0;
                        }
                        y_ += slope_;
                        break;
                    }
                }
            } else {
                y_ = y0;
            }
            return y_;
        }

        void setAttack(const FloatType millisecond) {
            attack_time_.store(std::max(FloatType(0), millisecond));
            to_update_.store(true);
        }

        void setRelease(const FloatType millisecond) {
            release_time_.store(std::max(FloatType(0), millisecond));
            to_update_.store(true);
        }

        void setPumpPunch(const FloatType x) {
            pp_portion_.store(std::clamp(x, FloatType(-1), FloatType(1)));
            to_update_.store(true);
        }

        void setSmooth(const FloatType x) {
            smooth_portion_.store(std::clamp(x, FloatType(0), FloatType(1)));
            to_update_.store(true);
        }

        PPState getCurrentPPState() const { return pp_state_; }

    private:
        FloatType y_{}, state_{}, slope_{};
        FloatType attack_{}, release_{};

        PPState pp_state_{PPState::kOff};
        FloatType pp_{}, smooth_{};

        double exp_factor_{-0.1308996938995747};
        std::atomic<FloatType> attack_time_{50}, release_time_{100}, pp_portion_{0}, smooth_portion_{0};
        std::atomic<bool> to_update_{true};

        void update() {
            // cache atomic values
            const auto current_attack_time = static_cast<double>(attack_time_.load());
            const auto current_release_time = static_cast<double>(release_time_.load());
            const auto current_smooth_portion = static_cast<double>(smooth_portion_.load());
            const auto current_pp_portion = static_cast<double>(pp_portion_.load());
            // update attack
            if (current_attack_time < 0.001) {
                attack_ = FloatType(0);
            } else {
                if (UsePP) {
                    attack_ = static_cast<FloatType>(std::exp(
                        exp_factor_ / current_attack_time / (
                            1. - std::pow(std::abs(current_pp_portion), 2.) * 0.125)));
                } else {
                    attack_ = static_cast<FloatType>(std::exp(exp_factor_ / current_attack_time));
                }
            }
            // update release
            if (current_release_time < 0.001) {
                release_ = FloatType(0);
            } else {
                release_ = static_cast<FloatType>(std::exp(exp_factor_ / current_release_time));
            }
            if (UseSmooth) {
                // update smooth
                smooth_ = static_cast<FloatType>(current_smooth_portion);
            }
            if (UsePP) {
                // update pump-punch
                if (current_attack_time < 0.001 || std::abs(current_pp_portion) < 0.001) {
                    pp_ = FloatType(0);
                    pp_state_ = PPState::kOff;
                } else {
                    pp_state_ = current_pp_portion > 0 ? PPState::kPump : PPState::kPunch;
                    pp_ = static_cast<FloatType>(std::exp(
                        exp_factor_ / current_attack_time / std::abs(current_pp_portion)));
                }
            }
        }
    };
}
