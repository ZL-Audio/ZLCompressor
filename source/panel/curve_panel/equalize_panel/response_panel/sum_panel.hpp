// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"
#include "static_freq_array.hpp"

namespace zlpanel {
    class SumPanel final : public juce::Component {
    public:
        explicit SumPanel(PluginProcessor& processor, zlgui::UIBase& base);

        ~SumPanel() override;

        void paint(juce::Graphics& g) override;

        void resized() override;

        bool run(std::array<float, kWsFloat.size()>& xs,
                 std::array<std::array < float, kWsFloat.size()>, 8> &yss,
                 std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum>& filter_status,
                 const juce::Rectangle<float>& bound);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        kfr::univector<float, kWsFloat.size()> ys{};

        juce::Path path_, next_path_;
        std::mutex mutex_;

        float curve_thickness_{0.f};

        void lookAndFeelChanged() override;
    };
} // zlpanel
