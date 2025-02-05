// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef PEAK_PANEL_HPP
#define PEAK_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../PluginProcessor.hpp"
#include "../helper/helper.hpp"

namespace zlPanel {
    class PeakPanel final : public juce::Component {
    public:
        explicit PeakPanel(PluginProcessor &processor);

        void paint(juce::Graphics &g) override;

        void run(double nextTimeStamp);

        void resized() override;

        void setTimeLength(const float x) {
            magAnalyzer.setTimeLength(x);
            numPerSecond.store(static_cast<double>(zlDSP::Controller::analyzerPointNum - 1) / static_cast<double>(x));
        }

    private:
        zlMagAnalyzer::MagReductionAnalyzer<double, zlDSP::Controller::analyzerPointNum> &magAnalyzer;
        AtomicBound atomicBound;

        juce::Path inPath, outPath, reductionPath;
        juce::Path nextInPath, nextOutPath, nextReductionPath;
        juce::SpinLock lock;

        double startTime{0.0};
        double currentCount{0.0};

        std::atomic<double> numPerSecond{50.0};

        bool isFirstPoint{true};
        double smoothError{0.f};
        int consErrorCount{0};
    };
} // zlPanel

#endif //PEAK_PANEL_HPP
