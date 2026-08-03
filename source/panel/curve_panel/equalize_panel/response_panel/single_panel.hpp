// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include <span>
#include <vector>

#include "../../../../dsp/filter/filter.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../../zlp/zlp_definitions.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class SinglePanel final : public juce::Component {
    public:
        explicit SinglePanel(zlgui::UIBase& base,
                             size_t band_idx, zldsp::filter::Ideal<float, 16>& filter);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void run(std::span<const float> xs, std::span<const float> dbs,
                 const juce::Rectangle<float>& bound, float max_db,
                 double sample_rate, double fft_max);

        juce::Point<float> getButtonPos() const {
            return button_pos_.load();
        }

        void setCurveThicknessScale(const float scale) {
            curve_thickness_scale = scale;
        }

    private:
        zlgui::UIBase& base_;

        float curve_thickness_scale{.5f};

        const size_t band_idx_;
        zldsp::filter::Ideal<float, 16>& filter_;
        std::vector<float> ys_;

        BufferedUI<juce::Path> path_;
        BufferedUI<juce::Line<float>> line_;

        AtomicPoint<float> button_pos_;

        float curve_thickness_{0.f}, line_thickness_{0.f};

        void visibilityChanged() override;

        void lookAndFeelChanged() override;
    };
}
