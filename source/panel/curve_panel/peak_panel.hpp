// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../PluginProcessor.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    class PeakPanel final : public juce::Component {
    public:
        explicit PeakPanel(PluginProcessor &processor);

        void paint(juce::Graphics &g) override;

        void run(double next_time_stamp);

        void resized() override;

        void setTimeLength(const float x) {
            mag_analyzer_ref_.setTimeLength(x);
            num_per_second_.store(static_cast<double>(zlp::CompressorController::kAnalyzerPointNum - 1) / static_cast<double>(x));
        }

    private:
        zldsp::analyzer::MagReductionAnalyzer<float, zlp::CompressorController::kAnalyzerPointNum> &mag_analyzer_ref_;
        AtomicBound<float> atomic_bound_;

        std::array<float, zlp::CompressorController::kAnalyzerPointNum> xs_, in_ys_, out_ys_, reduction_ys_;
        juce::Path in_path_, out_path_, reduction_path_;
        juce::Path next_in_path_, next_out_path_, next_reduction_path_;
        juce::SpinLock path_lock_;

        double start_time_{0.0};
        double current_count_{0.0};

        std::atomic<double> num_per_second_{50.0};

        bool is_first_point_{true};
        double smooth_error_{0.f};
        int cons_error_count_{0};

        void updatePaths(juce::Rectangle<float> bound);
    };
} // zlpanel
