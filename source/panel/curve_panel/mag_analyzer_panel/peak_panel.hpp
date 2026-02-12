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
#include "../../../dsp/analyzer/analyzer_base/fifo_transfer_buffer.hpp"
#include "../../../dsp/analyzer/mag_analyzer/mag_receiver.hpp"
#include "../../../dsp/analyzer/mag_analyzer/mag_reduction_receiver.hpp"
#include "../../../dsp/lock/spin_lock.hpp"

#include "rms_panel.hpp"

namespace zlpanel {
    class PeakPanel final : public juce::Component {
    public:
        explicit PeakPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~PeakPanel() override;

        void paint(juce::Graphics& g) override;

        void run(double next_time_stamp, RMSPanel& rms_panel,
            zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer,
            size_t consumer_id);

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

        AtomicBound<float> atomic_bound_;

        float pre_db_{-240.f}, out_db_{-240.f}, reduction_db_{0.f};
        kfr::univector<float> xs_{}, pre_ys_{}, reduction_ys_{}, out_ys_{};
        juce::Path in_path_, out_path_, reduction_path_;
        juce::Path next_in_path_, next_out_path_, next_reduction_path_;
        zldsp::lock::SpinLock mutex_;

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
