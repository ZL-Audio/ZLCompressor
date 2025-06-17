// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"
#include "static_freq_array.hpp"

namespace zlpanel {
    class SinglePanel final : public juce::Component,
                              private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit SinglePanel(PluginProcessor &processor, zlgui::UIBase &base,
                             size_t band_idx, zldsp::filter::Ideal<float, 16> &filter);

        ~SinglePanel() override;

        void paint(juce::Graphics &g) override;

        bool run(std::span<float> xs, std::span<float> ys,
                 const juce::Rectangle<float> &bound, bool force = false);

        juce::Point<float> getButtonPos() const {
            return button_pos_.load();
        }

        void setCurveThicknessScale(const float scale) {
            curve_thickness_scale = scale;
        }

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;

        float curve_thickness_scale{.5f};

        const size_t band_idx_;
        zldsp::filter::Ideal<float, 16> &filter_;

        constexpr static std::array kBandIDs{
            zlp::PFilterType::kID, zlp::POrder::kID,
            zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID
        };

        juce::Path path_, next_path_;
        juce::Line<float> line_, next_line_;
        std::mutex mutex_;

        AtomicPoint<float> button_pos_;

        void parameterChanged(const juce::String &parameter_ID, float new_value) override;

        void visibilityChanged() override;
    };
} // zlpanel
