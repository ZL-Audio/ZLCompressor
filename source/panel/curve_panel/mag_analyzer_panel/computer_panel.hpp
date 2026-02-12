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
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/lock/spin_lock.hpp"

namespace zlpanel {
    class ComputerPanel final : public juce::Component,
                                private juce::AudioProcessorValueTreeState::Listener {
    public:
        static constexpr size_t kNumPoint = 200;

        explicit ComputerPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~ComputerPanel() override;

        void paint(juce::Graphics& g) override;

        void run();

        void resized() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        static constexpr std::array kComputerIDs{
            zlp::PCompDirection::kID,
            zlp::PThreshold::kID, zlp::PRatio::kID, zlp::PKneeW::kID, zlp::PCurve::kID, zlp::PFloor::kID
        };
        static constexpr std::array kNAIDs{zlstate::PAnalyzerMinDB::kID};

        std::atomic<float>& comp_direction_ref_;
        std::atomic<float>& min_db_ref_;

        zldsp::compressor::CompressionComputer<float> compression_computer_{};
        zldsp::compressor::ExpansionComputer<float> expansion_computer_{};
        zldsp::compressor::InflationComputer<float> inflation_computer_{};
        AtomicBound<float> atomic_bound_;

        float curve_thickness_{0.f};

        std::atomic<bool> to_update_{true};
        juce::Path comp_path_, next_comp_path_;
        zldsp::lock::SpinLock mutex_;

        void lookAndFeelChanged() override;

        void parameterChanged(const juce::String& parameter_ID, float value) override;

        template <typename C, bool is_downward>
        void updateComputerPath(C& c);
    };
}
