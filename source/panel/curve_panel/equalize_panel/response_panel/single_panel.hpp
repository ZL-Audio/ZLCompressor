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
#include <atomic>
#include <span>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../../gui/gui.hpp"
#include "../../../../zlp/equalize_controller.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class SinglePanel final : public juce::Component {
    public:
        explicit SinglePanel(zlgui::UIBase& base,
                             size_t& selected_band_idx,
                             std::vector<size_t>& not_off_indices);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void updateDrawingParameters(size_t band,
                                     zlp::EqualizeController::FilterStatus filter_status);

        void run(size_t band,
                 zlp::EqualizeController::FilterStatus filter_status,
                 bool to_update,
                 std::span<const float> xs, float scale, float bias,
                 std::span<const float> magnitudes,
                 float center_x, float center_y, float button_y,
                 float left_x, float right_x,
                 bool is_all_pass, bool is_first_order);

    private:
        static constexpr size_t kNumPoints = 400;
        static constexpr float kFillingAlpha = .125f;
        static constexpr float kNotSelectedAlphaMultiplier = .75f;
        static constexpr float kBypassAlphaMultiplier = .75f;
        static constexpr float kNoBandSelectedAlphaMultiplier = .75f;
        static constexpr float kThickMultiplier = 1.15f;

        zlgui::UIBase& base_;
        size_t& selected_band_idx_;
        std::vector<size_t>& not_off_indices_;

        std::atomic<float> center_y_{0.f};
        std::atomic<float> height_{0.f};

        std::array<BufferedUI<juce::Path>, zlp::kBandNum> paths_{};
        std::array<BufferedUI<juce::Path>, zlp::kBandNum> fills_{};
        std::array<BufferedUI<juce::Line<float>>, zlp::kBandNum> button_lines_{};
        std::array<BufferedUI<juce::Line<float>>, zlp::kBandNum> all_pass_lines_{};

        std::array<float, zlp::kBandNum> stroke_alpha_{};
        std::array<float, zlp::kBandNum> fill_alpha_{};
        std::array<juce::Colour, zlp::kBandNum> stroke_colour_{};

        float curve_thickness_{0.f};
        std::vector<float> temp_y_{};

        template <bool thick = false>
        void drawBand(juce::Graphics& g, size_t band);

        void lookAndFeelChanged() override;
    };
}
