// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPUTER_PANEL_HPP
#define COMPUTER_PANEL_HPP

#include "../../PluginProcessor.hpp"
#include "helpers.hpp"

namespace zlPanel {
    class ComputerPanel final : public juce::Component {
    public:
        static constexpr size_t numPoint = 100;

        explicit ComputerPanel();

        void paint(juce::Graphics &g) override;

        void run();

        void resized() override;

    private:
        zlCompressor::KneeComputer<float> computer{};
        AtomicBound atomicBound;

        std::atomic<bool> toUpdate{true};
        juce::Path compPath, nextCompPath;
        juce::SpinLock lock;

        std::atomic<float> minDB{-72.f};
        std::array<float, numPoint> pathY{};
    };
} // zlPanel

#endif //COMPUTER_PANEL_HPP
