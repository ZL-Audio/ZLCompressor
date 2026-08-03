// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../helper/helper.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class EqualizerBackgroundPanel final : public juce::Component {
    public:
        explicit EqualizerBackgroundPanel(PluginProcessor& processor, zlgui::UIBase& base);

        void resized() override;

        void setMouseOver(bool is_mouse_on);

        void updateSampleRate(double sample_rate);

    private:
        static constexpr std::array<float, 10> kBackgroundFreqs = {
            20.f, 50.f, 100.f, 200.f, 500.f,
            1000.f, 2000.f, 5000.f, 10000.f, 20000.f
        };

        static constexpr std::array<std::string_view, 10> kBackgroundFreqsNames = {
            "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k"
        };

        static constexpr std::array<float, 6> kBackgroundDBs = {
            0.f, 1.f / 6.f, 2.f / 6.f, 0.5, 4.f / 6.f, 5.f / 6.f
        };

        class Background1 final : public juce::Component {
        public:
            explicit Background1(zlgui::UIBase& base);

            void paint(juce::Graphics& g) override;

            void resized() override;

            void updateFreqMax(double freq_max);

        private:
            zlgui::UIBase& base_;

            juce::RectangleList<float> rect_list_;
            std::array<juce::Rectangle<float>, 10> text_bounds_;
            std::array<bool, 10> visible_freqs_{};
            double freq_max_{22000.0};

            void updateLayout();
        };

        Background1 background1_;
    };
} // zlpanel
