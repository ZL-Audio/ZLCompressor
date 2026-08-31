// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <span>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../../gui/gui.hpp"
#include "../../../../zlp/equalize_controller.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class SumPanel final : public juce::Component {
    public:
        explicit SumPanel(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void run(bool to_update, bool has_not_off_filter,
                 std::span<const size_t> on_indices,
                 std::span<const float> xs, float scale, float bias,
                 const std::array<zldsp::vector::aligned_vector<float>, zlp::kBandNum>& magnitudes);

    private:
        static constexpr size_t kNumPoints = 400;

        zlgui::UIBase& base_;
        BufferedUI<juce::Path> path_{};
        float curve_thickness_{0.f};
        std::vector<float> temp_y_{};

        void lookAndFeelChanged() override;
    };
}
