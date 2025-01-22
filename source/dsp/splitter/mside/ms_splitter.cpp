// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "ms_splitter.hpp"
namespace zlSplitter {
    template<typename FloatType>
    void MSSplitter<FloatType>::reset() {
        mBuffer.clear();
        sBuffer.clear();
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        mBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        sBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        split(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::split(juce::dsp::AudioBlock<FloatType> block) {
        mBuffer.setSize(1, static_cast<int>(block.getNumSamples()), true, false, true);
        sBuffer.setSize(1, static_cast<int>(block.getNumSamples()), true, false, true);
        auto lBuffer = block.getChannelPointer(0);
        auto rBuffer = block.getChannelPointer(1);
        auto _mBuffer = mBuffer.getWritePointer(0);
        auto _sBuffer = sBuffer.getWritePointer(0);
        for (size_t i = 0; i < static_cast<size_t>(block.getNumSamples()); ++i) {
            _mBuffer[i] = FloatType(0.5) * (lBuffer[i] + rBuffer[i]);
            _sBuffer[i] = _mBuffer[i] - rBuffer[i];
        }
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::combine(juce::AudioBuffer<FloatType> &buffer) {
        combine(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void MSSplitter<FloatType>::combine(juce::dsp::AudioBlock<FloatType> block) {
        auto lBuffer = block.getChannelPointer(0);
        auto rBuffer =  block.getChannelPointer(1);
        auto _mBuffer = mBuffer.getReadPointer(0);
        auto _sBuffer = sBuffer.getReadPointer(0);
        for (size_t i = 0; i < static_cast<size_t>(block.getNumSamples()); ++i) {
            lBuffer[i] = _mBuffer[i] + _sBuffer[i];
            rBuffer[i] = _mBuffer[i] - _sBuffer[i];
        }
    }

    template
    class MSSplitter<float>;

    template
    class MSSplitter<double>;
}