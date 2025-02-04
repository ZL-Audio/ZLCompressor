// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#ifndef CURVE_PANEL_HPP
#define CURVE_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../PluginProcessor.hpp"
#include "rms_panel.hpp"
#include "peak_panel.hpp"
#include "computer_panel.hpp"
#include "separate_panel.hpp"

namespace zlPanel {
    class CurvePanel final : public juce::Component,
                             private juce::Thread {
    public:
        explicit CurvePanel(PluginProcessor &processor);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void paintOverChildren(juce::Graphics &g) override;

        void resized() override;

    private:
        PeakPanel peakPanel;
        RMSPanel rmsPanel;
        ComputerPanel computerPanel;
        SeparatePanel separatePanel;
        juce::VBlankAttachment vblank;
        std::atomic<double> nextStamp{0.};
        double rmsPreviousStamp{0.};
        std::atomic<bool> toRunRMS{false};

        void run() override;

        void repaintCallBack(double timeStamp);
    };
} // zlPanel

#endif //CURVE_PANEL_HPP
