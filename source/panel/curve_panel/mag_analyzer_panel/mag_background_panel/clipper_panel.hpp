// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../helper/helper.hpp"
#include "../../../../gui/gui.hpp"

namespace zlpanel {
    class ClipperPanel final : public juce::Component,
                               private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ClipperPanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~ClipperPanel() override;

        void paint(juce::Graphics &g) override;

        void repaintCallBackSlow();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        float rel_position_{0.f};

        std::atomic<float> mag_min_db_{-54.0};
        std::atomic<bool> to_update_path_{false};

        zldsp::compressor::KneeComputer<float, true> computer_{};
        zldsp::compressor::TanhClipper<float> clipper_{};

        static constexpr std::array kClipperIDs{
            zlp::PThreshold::kID, zlp::PRatio::kID, zlp::PKneeW::kID, zlp::PCurve::kID,
            zlp::PClipperDrive::kID
        };

        static constexpr std::array kNAIDs{
            zlstate::PAnalyzerMinDB::kID
        };

        void parameterChanged(const juce::String &parameter_ID, float new_value) override;
    };
} // zlpanel
