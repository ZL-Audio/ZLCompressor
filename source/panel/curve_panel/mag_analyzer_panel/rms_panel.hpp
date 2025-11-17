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
    class RMSPanel final : public juce::Component {
    public:
        explicit RMSPanel(PluginProcessor& processor, zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void run(bool update_path);

        void resized() override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;

    private:
        zlgui::UIBase& base_;
        zldsp::analyzer::MultipleMagAvgAnalyzer<float, 2, zlp::CompressController::kAvgAnalyzerPointNum>&
        avg_analyzer_ref_;
        std::atomic<float>& min_db_ref_;

        AtomicBound<float> atomic_bound_;

        float curve_thickness_{0.f};

        std::array<float, zlp::CompressController::kAvgAnalyzerPointNum> in_xs_{}, out_xs{}, ys_{};
        juce::Path in_path_, out_path_;
        juce::Path next_in_path_, next_out_path_;
        std::mutex mutex_;

        void lookAndFeelChanged() override;
    };
} // zlpanel
