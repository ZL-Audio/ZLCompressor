// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../helper/helper.hpp"

namespace zlPanel {
    class ComputerPanel final : public juce::Component {
    public:
        static constexpr size_t numPoint = 100;

        explicit ComputerPanel();

        void paint(juce::Graphics &g) override;

        void run();

        void resized() override;

    private:
        zldsp::compressor::KneeComputer<float> computer{};
        AtomicBound atomicBound;

        std::atomic<bool> toUpdate{true};
        juce::Path compPath, nextCompPath;
        juce::SpinLock lock;

        std::atomic<float> minDB{-72.f};
    };
} // zlPanel
