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
#include "../../../dsp/analyzer/mag_analyzer/mag_analyzer_receiver.hpp"
#include "rms_panel.hpp"

namespace zlpanel {
    class PeakPanel final : public juce::Component {
    public:
        explicit PeakPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~PeakPanel() override;

        void paint(juce::Graphics& g) override;

        void run(double next_time_stamp, RMSPanel& rms_panel);

        void resized() override;

    private:
        static constexpr std::array<int, 4> kNumPointsPerSecond{40, 30, 20, 15};
        static constexpr int kPausedThreshold = 6;
        static constexpr int kTooMuchResetThreshold = 64;
        zlgui::UIBase& base_;

        std::atomic<float>& comp_direction_ref_;
        std::atomic<float>& analyzer_stereo_type_ref_;
        std::atomic<float>& analyzer_mag_type_ref_;
        std::atomic<float>& analyzer_min_db_ref_;
        std::atomic<float>& analyzer_time_length_ref_;

        zldsp::analyzer::MagAnalyzerSender<float, 3>& analyzer_sender_;
        zldsp::analyzer::MagAnalyzerReceiver<3> analyzer_receiver_{};

        AtomicBound<float> atomic_bound_;

        kfr::univector<float> xs_{}, pre_ys_{}, post_ys_{}, out_ys_{};
        juce::Path in_path_, out_path_, reduction_path_;
        juce::Path next_in_path_, next_out_path_, next_reduction_path_;
        std::mutex mutex_;

        float curve_thickness_{0.f};

        double start_time_{0.0};

        bool is_first_point_{true};
        int too_much_samples_{0};
        int num_missing_points_{0};

        double sample_rate_{0.};
        size_t max_num_samples_{0};
        float time_length_idx_{0.f}, time_length_{6.f};

        size_t num_points_{0};
        int num_samples_per_point_{0};
        int num_points_per_second_{0};
        double second_per_point_{0};

        template <bool center>
        void updatePaths(juce::Rectangle<float> bound);

        void lookAndFeelChanged() override;
    };
}
