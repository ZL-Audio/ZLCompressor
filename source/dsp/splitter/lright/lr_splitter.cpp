// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "lr_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    void LRSplitter<FloatType>::reset() {
        lBuffer.clear();
        rBuffer.clear();
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        lBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        rBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        split(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::split(juce::dsp::AudioBlock<FloatType> block) {
        lBuffer.setSize(1, static_cast<int>(block.getNumSamples()), true, false, true);
        rBuffer.setSize(1, static_cast<int>(block.getNumSamples()), true, false, true);
        const auto lBlock = block.getSingleChannelBlock(0);
        const auto rBlock = block.getSingleChannelBlock(1);
        lBlock.copyTo(lBuffer, 0, 0, block.getNumSamples());
        rBlock.copyTo(rBuffer, 0, 0, block.getNumSamples());
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::combine(juce::AudioBuffer<FloatType> &buffer) {
        combine(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void LRSplitter<FloatType>::combine(juce::dsp::AudioBlock<FloatType> block) {
        const auto lBlock = block.getSingleChannelBlock(0);
        const auto rBlock = block.getSingleChannelBlock(1);
        lBlock.copyFrom(lBuffer, 0, 0, block.getNumSamples());
        rBlock.copyFrom(rBuffer, 0, 0, block.getNumSamples());
    }

    template
    class LRSplitter<float>;

    template
    class LRSplitter<double>;
}
