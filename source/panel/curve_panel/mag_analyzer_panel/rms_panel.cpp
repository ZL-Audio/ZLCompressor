// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "rms_panel.hpp"

namespace zlpanel {
    RMSPanel::RMSPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        for (auto& receiver : {&in_receiver_, &out_receiver_}) {
            receiver->setHistSize(kNumPoints);
        }

        setBufferedToImage(true);
        setInterceptsMouseClicks(false, false);
    }

    void RMSPanel::paint(juce::Graphics& g) {
        in_path_.pull();
        out_path_.pull();
        g.setColour(base_.getTextColour().withAlpha(.25f));
        g.fillPath(in_path_.get_reader());
        g.setColour(base_.getTextColour().withAlpha(.9f));
        g.strokePath(out_path_.get_reader(),
                     juce::PathStrokeType(curve_thickness_,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void RMSPanel::run(const double sample_rate, const zldsp::container::FIFORange range,
                       zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer) {
        const auto bound = atomic_bound_.load();
        if (std::abs(bound.getHeight() - height_) > .1f) {
            height_ = bound.getHeight();
            auto y0 = 0.f;
            const auto delta_y = height_ / static_cast<float>(ys_.size() - 1);
            for (size_t i = 0; i < kNumPoints; ++i) {
                ys_[i] = y0;
                y0 += delta_y;
            }
            std::cout << std::endl;
        }
        if (std::abs(sample_rate - sample_rate_) > 0.1) {
            sample_rate_ = sample_rate;
            const auto max_num_samples = static_cast<size_t>(sample_rate_ * 0.1);
            for (auto& receiver : {&in_receiver_, &out_receiver_}) {
                receiver->setMaxNumSamples(max_num_samples);
            }
        }
        if (std::abs(min_db_idx_ - analyzer_min_db_ref_.load(std::memory_order::relaxed)) > .1f) {
            min_db_idx_ = analyzer_min_db_ref_.load(std::memory_order::relaxed);
            const auto min_db = zlstate::PAnalyzerMinDB::kDBs[static_cast<size_t>(std::round(min_db_idx_))];
            for (auto& receiver : {&in_receiver_, &out_receiver_}) {
                receiver->setMinDB(min_db);
            }
        }
        if (to_reset_.exchange(false, std::memory_order::relaxed)) {
            for (auto& receiver : {&in_receiver_, &out_receiver_}) {
                receiver->reset();
            }
        }
        in_receiver_.run(range, transfer_buffer.getSampleFIFOs()[0]);
        if (out_receiver_.run(range, transfer_buffer.getSampleFIFOs()[2])) {
            in_receiver_.updateHeight(bound.getWidth(), in_xs_);
            out_receiver_.updateHeight(bound.getWidth(), out_xs_);

            auto& next_in_path{in_path_.get_writer()};
            auto& next_out_path{out_path_.get_writer()};
            next_in_path.clear();
            next_out_path.clear();
            next_in_path.startNewSubPath(0.f, ys_.front());
            next_in_path.lineTo(in_xs_.front(), ys_.front());
            next_out_path.startNewSubPath(out_xs_.front(), ys_.front());
            for (size_t i = 1; i < kNumPoints; ++i) {
                next_in_path.lineTo(in_xs_[i], ys_[i]);
                next_out_path.lineTo(out_xs_[i], ys_[i]);
            }
            next_in_path.lineTo(0.f, ys_.back());
            next_in_path.closeSubPath();

            in_path_.publish();
            out_path_.publish();
        }
    }

    void RMSPanel::resized() {
        atomic_bound_.store(getLocalBounds().toFloat());
        lookAndFeelChanged();
    }

    void RMSPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .2f * base_.getMagCurveThickness();
    }
}
