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

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class PeakPanel final : public juce::Component,
                            private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit PeakPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~PeakPanel() override;

        void paint(juce::Graphics& g) override;

        void run(double next_time_stamp);

        void resized() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& comp_direction_ref_;
        static constexpr std::array kNAIDs{
            zlstate::PAnalyzerMagType::kID,
            zlstate::PAnalyzerMinDB::kID,
            zlstate::PAnalyzerTimeLength::kID
        };

        zldsp::analyzer::MagReductionAnalyzer<float, zlp::CompressController::kAnalyzerPointNum>& mag_analyzer_ref_;
        AtomicBound<float> atomic_bound_;

        std::array<float, zlp::CompressController::kAnalyzerPointNum> xs_{}, in_ys_{}, out_ys_{}, reduction_ys_{};
        juce::Path in_path_, out_path_, reduction_path_;
        juce::Path next_in_path_, next_out_path_, next_reduction_path_;
        std::mutex mutex_;

        float curve_thickness_{0.f};

        std::atomic<bool> to_reset_path_{true};
        double start_time_{0.0}, current_time{0.0};
        double current_count_{0.0};
        std::atomic<double> num_per_second_{50.0};

        bool is_first_point_{true};

        std::atomic<float> analyzer_min_db_{-72.f};

        void updatePaths(juce::Rectangle<float> bound);

        void lookAndFeelChanged() override;

        void parameterChanged(const juce::String& parameter_id, float new_value) override;

        void setTimeLength(const float x) {
            mag_analyzer_ref_.setTimeLength(x);
            num_per_second_.store(
                static_cast<double>(zlp::CompressController::kAnalyzerPointNum - 1) / static_cast<double>(x));
            to_reset_path_.exchange(true, std::memory_order::release);
        }
    };
} // zlpanel
