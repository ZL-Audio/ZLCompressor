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
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class FFTAnalyzerPanel final : public juce::Component {
    public:
        explicit FFTAnalyzerPanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~FFTAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void run();

        void resized() override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;

        bool skip_next_repaint_{false};
        AtomicBound<float> atomic_bound_;
        float width_{-.1f};

        std::array<float, zlp::EqualizeController::kAnalyzerPointNum> xs_{}, ys_{};
        juce::Path out_path_, next_out_path_;
        std::mutex mutex_;
    };
} // zlpanel
