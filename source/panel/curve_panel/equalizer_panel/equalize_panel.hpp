// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../helper/helper.hpp"
#include "../../../gui/gui.hpp"
#include "fft_analyzer_panel.hpp"

namespace zlpanel {
    class EqualizePanel final : public juce::Component {
    public:
        explicit EqualizePanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~EqualizePanel() override;

        void paint(juce::Graphics &g) override;

        void run(juce::Thread &thread);

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        zlgui::UIBase &base_;
        FFTAnalyzerPanel fft_analyzer_panel_;
    };
} // zlpanel
