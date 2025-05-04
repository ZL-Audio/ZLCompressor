// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_dsp/juce_dsp.h>

#include "../../container/container.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    /**
     * a tracker that tracks the momentary RMS loudness of the audio signal
     * @tparam FloatType
     * @tparam IsPeakMix
     */
    template<typename FloatType, bool IsPeakMix = false>
    class RMSTracker {
    public:
        inline static FloatType kMinusInfinityDB = FloatType(-240);

        RMSTracker() = default;

        /**
         * call before prepare to play
         * set the maximum time length of the tracker
         * @param second the maximum time length of the tracker
         */
        void setMaximumMomentarySeconds(const FloatType second) {
            maximum_time_length = second;
        }

        void reset() {
            square_sum = FloatType(0);
            square_buffer.clear();
        }

        /**
         * call before processing starts
         * @param sr sample_rate
         */
        void prepare(const double sr) {
            sample_rate.store(sr);
            setMaximumMomentarySize(
                static_cast<size_t>(static_cast<double>(maximum_time_length) * sr));
            reset();
            setMomentarySeconds(time_length.load());
        }

        /**
         * update values before processing a buffer
         */
        void prepareBuffer() {
            if (to_update.exchange(false)) {
                currentBuffer_size = buffer_size.load();
                while (square_buffer.size() > currentBuffer_size) {
                    square_sum -= square_buffer.pop_front();
                }
                if (IsPeakMix) {
                    current_peak_mix = peak_mix.load();
                    current_peak_mix_c = FloatType(1) - current_peak_mix;
                }
            }
        }

        void processSample(const FloatType x) {
            const FloatType square = IsPeakMix
                                         ? std::abs(x) * (current_peak_mix + std::abs(x) * current_peak_mix_c)
                                         : x * x;
            if (square_buffer.size() == currentBuffer_size) {
                square_sum -= square_buffer.pop_front();
            }
            square_buffer.push_back(square);
            square_sum += square;
        }

        void processBufferRMS(juce::AudioBuffer<FloatType> &buffer) {
            FloatType _ms = 0;
            for (auto channel = 0; channel < buffer.getNumChannels(); channel++) {
                auto data = buffer.getReadPointer(channel);
                _ms += zlVector::sumsqr(buffer.getReadPointer(channel),
                                        static_cast<size_t>(buffer.getNumSamples()));
            }
            _ms = _ms / static_cast<FloatType>(buffer.getNumSamples());

            if (square_buffer.size() == currentBuffer_size) {
                square_sum -= square_buffer.pop_front();
            }
            square_buffer.push_back(_ms);
            square_sum += _ms;
        }

        /**
         * thread-safe, lock-free
         * set the time length of the tracker
         * @param second the time length of the tracker
         */
        void setMomentarySeconds(const FloatType second) {
            time_length.store(second);
            setMomentarySize(static_cast<size_t>(static_cast<double>(second) * sample_rate.load()));
            to_update.store(true);
        }

        /**
         * thread-safe, lock-free
         * get the time length of the tracker
         */
        inline size_t getMomentarySize() const {
            return buffer_size.load();
        }

        /**
         * thread-safe, lock-free
         * set the peak-mix portion
         * @param x the peak-mix portion
         */
        void setPeakMix(const FloatType x) {
            peak_mix.store(x);
            to_update.store(true);
        }

        size_t getCurrentBuffer_size() const { return currentBuffer_size; }

        FloatType getMomentarySquare() {
            return square_sum;
        }

        FloatType getMomentaryLoudness() {
            FloatType meanSquare = square_sum / static_cast<FloatType>(currentBuffer_size);
            return juce::Decibels::gainToDecibels(meanSquare, kMinusInfinityDB) * FloatType(0.5);
        }

    private:
        FloatType square_sum{0};
        zlContainer::CircularBuffer<FloatType> square_buffer{1};

        std::atomic<double> sample_rate{48000.0};
        std::atomic<FloatType> time_length{0};
        FloatType maximum_time_length{0};
        size_t currentBuffer_size{1};
        std::atomic<size_t> buffer_size{1};
        FloatType current_peak_mix{0}, current_peak_mix_c{1};
        std::atomic<FloatType> peak_mix{0};
        std::atomic<bool> to_update{true};

        void setMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            buffer_size.store(mSize);
        }

        void setMaximumMomentarySize(size_t mSize) {
            mSize = std::max(static_cast<size_t>(1), mSize);
            square_buffer.set_capacity(mSize);
        }
    };
}
