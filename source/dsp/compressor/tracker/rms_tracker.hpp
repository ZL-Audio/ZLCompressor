// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_COMPRESSOR_RMS_TRACKER_HPP
#define ZL_COMPRESSOR_RMS_TRACKER_HPP

#include <juce_dsp/juce_dsp.h>

#include "../../container/container.hpp"

namespace zlCompressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     */
    template<typename FloatType, bool isPeakMix = false>
    class RMSTracker {
    public:
        inline static FloatType minusInfinityDB = FloatType(-240);

        RMSTracker() = default;

        /**
         * call before prepare to play
         * set the maximum time length of the tracker
         * @param x the maximum time length of the tracker
         */
        void setMaximumMomentarySeconds(FloatType x) {
            maximumTimeLength = x;
        }

        /**
         * call before processing starts
         * @param spec
         */
        void prepare(const juce::dsp::ProcessSpec &spec) {
            sampleRate.store(spec.sampleRate);
            setMaximumMomentarySize(static_cast<size_t>(static_cast<double>(maximumTimeLength) * spec.sampleRate));
            reset();
            setMomentarySeconds(timeLength.load());
        }

        /**
         * cache values before processing a buffer
         */
        void prepareBuffer() {
            if (toUpdate.exchange(false)) {
                currentBufferSize = bufferSize.load();
                while (loudnessBuffer.size() > currentBufferSize) {
                    mLoudness -= loudnessBuffer.pop_front();
                }
                if (isPeakMix) {
                    currentPeakMix = peakMix.load();
                    currentPeakMixC = FloatType(1) - currentPeakMix;
                }
            }
        }

        void process(FloatType peak) {
            const FloatType square = isPeakMix
                                         ? std::abs(peak) * (currentPeakMix + std::abs(peak) * currentPeakMixC)
                                         : peak * peak;
            if (loudnessBuffer.size() == currentBufferSize) {
                mLoudness -= loudnessBuffer.pop_front();
            }
            loudnessBuffer.push_back(square);
            mLoudness += square;
        }

        /**
         * thread-safe, lock-free
         * set the time length of the tracker
         * @param x the time length of the tracker
         */
        void setMomentarySeconds(FloatType x) {
            timeLength.store(x);
            setMomentarySize(static_cast<size_t>(x * static_cast<FloatType>(sampleRate.load())));
            toUpdate.store(true);
        }

        /**
         * thread-safe, lock-free
         * get the time length of the tracker
         */
        inline size_t getMomentarySize() const {
            return bufferSize.load();
        }

        /**
         * thread-safe, lock-free
         * set the peak-mix portion
         * @param x the peak-mix portion
         */
        void setPeakMix(const FloatType x) {
            peakMix.store(x);
            toUpdate.store(true);
        }

        FloatType getMomentaryLoudness() {
            FloatType meanSquare = mLoudness / static_cast<FloatType>(currentBufferSize);
            return juce::Decibels::gainToDecibels(meanSquare, minusInfinityDB) * FloatType(0.5);
        }

    private:
        FloatType mLoudness{0};
        zlContainer::CircularBuffer<FloatType> loudnessBuffer{1};

        std::atomic<double> sampleRate{44100.0};
        std::atomic<FloatType> timeLength{0};
        FloatType maximumTimeLength{0};
        size_t currentBufferSize{1};
        std::atomic<size_t> bufferSize{1};
        FloatType currentPeakMix{0}, currentPeakMixC{1};
        std::atomic<FloatType> peakMix{0};
        std::atomic<bool> toUpdate{true};

        void reset() {
            mLoudness = FloatType(0);
            loudnessBuffer.clear();
        }

        void setMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            bufferSize.store(mSize);
        }

        void setMaximumMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            loudnessBuffer.set_capacity(mSize);
        }
    };
} // zldetector

#endif //ZL_COMPRESSOR_RMS_TRACKER_HPP
