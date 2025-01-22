// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_COMPRESSOR_BALLISTICS_FILTER_HPP
#define ZL_COMPRESSOR_BALLISTICS_FILTER_HPP

#include <juce_dsp/juce_dsp.h>
#include <numbers>

namespace zl::Compressor {
    template<typename FloatType, bool isHold = false>
    class BallisticsFilter {
    public:
        enum Style {
            decoupled,
            branching,
            decoupledSmooth,
            branchingSmooth
        };

        BallisticsFilter() = default;

        /**
         * call before processing starts
         * @param spec
         */
        void prepare(const juce::dsp::ProcessSpec &spec) {
            sampleRate = spec.sampleRate;
            expFactor = -2.0 * std::numbers::pi * 1000.0 / sampleRate;
            state = FloatType(0);
            y = FloatType(0);
            toUpdate.store(true);
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (toUpdate.exchange(false)) {
                update();
            }
            if (currentStyle != style.load()) {
                currentStyle = style.load();
                state = FloatType(0);
                y = FloatType(0);
            }
        }

        FloatType processSample(const FloatType x) {
            switch (currentStyle) {
                case decoupled: {
                    state = std::max(x, state * release);
                    y = attack * y + attackC * state;
                }
                case branching: {
                    y = x >= y ? attack * y + attackC * x : release * y;
                }
                case decoupledSmooth: {
                    state = std::max(x, release * state + releaseC * x);
                    y = attack * y + attackC * state;
                }
                case branchingSmooth: {
                    y = x >= y ? attack * y + attackC * x : release * y + releaseC * x;
                }
            }
            return y;
        }

    private:
        std::atomic<bool> toUpdate{true};
        double sampleRate{44100.0}, expFactor{-0.142};
        std::atomic<FloatType> attackTime{1}, releaseTime{1}, holdTime{1};
        FloatType attack{0}, attackC{1}, release{0}, releaseC{0};
        FloatType state{0}, y{0};
        int holdSize{0}, holdPos{0};
        std::atomic<Style> style{branchingSmooth};
        Style currentStyle{branchingSmooth};

        void update() {
            const auto currentAttackTime = attackTime.load();
            attack = currentAttackTime < FloatType(1e-3)
                         ? FloatType(0)
                         : static_cast<FloatType>(std::exp(static_cast<double>(currentAttackTime) * expFactor));
            attackC = FloatType(1) - attack;
            const auto currentReleaseTime = releaseTime.load();
            release = currentReleaseTime < FloatType(1e-3)
                          ? FloatType(0)
                          : static_cast<FloatType>(std::exp(static_cast<double>(currentReleaseTime) * expFactor));
            releaseC = FloatType(1) - release;
            if (isHold) {
                holdSize = static_cast<int>(std::round(holdTime.load() * sampleRate));
            }
        }
    };
}

#endif //ZL_COMPRESSOR_BALLISTICS_FILTER_HPP
