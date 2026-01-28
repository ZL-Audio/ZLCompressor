// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "meter_panel.hpp"

namespace zlpanel {
    MeterPanel::MeterPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        analyzer_mag_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMagType::kID)),
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {

    }

    MeterPanel::~MeterPanel() = default;

    void MeterPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        for (auto& a_bound : reduction_rect_) {
            g.fillRect(a_bound.load());
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
        for (auto& a_bound : pre_rect_) {
            g.fillRect(a_bound.load());
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
        for (auto& a_bound : out_rect_) {
            g.fillRect(a_bound.load());
        }
    }

    void MeterPanel::resized() {
        bound_.store(getLocalBounds().toFloat());
    }

    void MeterPanel::run(const double next_time_stamp,
                         zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer,
                         const size_t consumer_id) {
        if (!is_first_point_) {
            const auto mag_type = static_cast<zldsp::analyzer::MagType>(std::round(
                analyzer_mag_type_ref_.load(std::memory_order::relaxed)));
            const auto min_db = zlstate::PAnalyzerMinDB::kDBs[static_cast<size_t>(std::round(
                analyzer_min_db_ref_.load(std::memory_order::relaxed)))];

            const auto delta_time = next_time_stamp - start_time_;
            start_time_ = next_time_stamp;
            // run meter receiver
            auto& fifo{transfer_buffer.getMulticastFIFO()};
            const auto delta_num_samples = static_cast<int>(delta_time * transfer_buffer.getSampleRate());
            const auto num_ready = fifo.getNumReady(consumer_id);
            const auto threshold = 2 * std::max(static_cast<int>(transfer_buffer.getMaxNumSamples()),
                                                delta_num_samples);
            const int num_to_read = num_ready > threshold
                ? num_ready - threshold
                : std::min(num_ready, delta_num_samples);
            const auto range = fifo.prepareToRead(consumer_id, num_to_read);
            meter_receiver_.run(range, transfer_buffer.getSampleFIFOs(), mag_type);

        } else {
            is_first_point_ = false;
            start_time_ = next_time_stamp;
        }
    }
}
