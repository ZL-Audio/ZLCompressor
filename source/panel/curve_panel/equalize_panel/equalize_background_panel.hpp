// Copyright (C) 2025 - zsliu98
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
        explicit EqualizerBackgroundPanel(PluginProcessor &processor, zlgui::UIBase &base);

        void resized() override;

        void setMouseOver(bool is_mouse_on);

    private:
        /** stl does not support constexpr log/pow,
         * (np.log([20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000]) - np.log(10)) / (np.log(22000) - np.log(10)) */
        static constexpr std::array<float, 10> kBackgroundFreqs = {
            0.09006341f, 0.20912077f, 0.29918418f, 0.3892476f, 0.50830495f,
            0.59836837f, 0.68843178f, 0.80748914f, 0.89755255f, 0.98761596f
        };

        static constexpr std::array<std::string, 10> kBackgroundFreqsNames = {
            "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k"
        };

        static constexpr std::array<float, 6> kBackgroundDBs = {
            0.f, 1.f / 6.f, 2.f / 6.f, 0.5, 4.f / 6.f, 5.f / 6.f
        };

        class Background1 final : public juce::Component {
        public:
            explicit Background1(zlgui::UIBase &base);

            void paint(juce::Graphics &g) override;

            void resized() override;

        private:
            zlgui::UIBase &base_;

            juce::RectangleList<float> rect_list_;
            std::array<juce::Rectangle<float>, 10> text_bounds_;
        };

        Background1 background1_;
    };
} // zlpanel
