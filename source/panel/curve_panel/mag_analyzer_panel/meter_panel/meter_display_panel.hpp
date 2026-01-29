// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"
#include "../../../../dsp/analyzer/analyzer_base/fifo_transfer_buffer.hpp"
#include "../../../../dsp/analyzer/mag_analyzer/mag_meter_receiver.hpp"
#include "../../../../dsp/analyzer/mag_analyzer/mag_reduction_receiver.hpp"
#include "meter_top_panel.hpp"

namespace zlpanel {
    class MeterDisplayPanel final : public juce::Component {
    public:
        explicit MeterDisplayPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~MeterDisplayPanel() override;

        void paint(juce::Graphics& g) override;

        void run(double next_time_stamp,
                 zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer,
                 size_t consumer_id);

        void resized() override;

        void repaintCallBackSlow();

    private:
        static constexpr float kReductionDecayPerSecond = 16.f;
        static constexpr float kMeterDecayPerSecond = 8.f;
        zlgui::UIBase& base_;
        MeterTopPanel meter_top_panel_;

        std::atomic<float>& comp_direction_ref_;
        std::atomic<float>& analyzer_mag_type_ref_;
        std::atomic<float>& analyzer_min_db_ref_;

        AtomicBound<float> bound_;
        std::array<float, 2> previous_reduction_{0.f, 0.f};
        std::array<float, 2> previous_pre_{-240.f, -240.f};
        std::array<float, 2> pre_decay_mul_{1.f, 1.f};
        std::array<float, 2> previous_out_{-240.f, -240.f};
        std::array<float, 2> out_decay_mul_{1.f, 1.f};
        std::array<AtomicBound<float>, 2> reduction_rect_{};
        std::array<AtomicBound<float>, 2> pre_rect_{};
        std::array<AtomicBound<float>, 2> out_rect_{};

        double start_time_{0.0};
        bool is_first_point_{true};

        zldsp::container::CircularMinMaxBuffer<float, zldsp::container::kFindMax> circular_min_max_;
        AtomicBound<float> reduction_max_rect_{};
        std::atomic<float> reduction_max_value_{0.f};

        zldsp::analyzer::MagReductionReceiver reduction_receiver_{};
        zldsp::analyzer::MagMeterReceiver pre_receiver_{};
        zldsp::analyzer::MagMeterReceiver out_receiver_{};

        std::atomic<float> reduction_peak_{0.f};
        std::atomic<float> out_peak_{-240.f};

        bool is_upwards_{false};
        std::atomic<bool> a_is_upwards_{false};

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        static std::string formatValue(float value);
    };
}
