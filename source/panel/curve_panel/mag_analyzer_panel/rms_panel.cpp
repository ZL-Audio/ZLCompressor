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
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)),
        analyzer_sender_(p.getCompressController().getMagAnalyzerSender()) {
        for (auto& receiver : {&in_receiver_, &out_receiver_}) {
            receiver->setHistSize(kNumPoints);
        }

        setBufferedToImage(true);
        setInterceptsMouseClicks(false, false);
    }

    void RMSPanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getTextColour().withAlpha(.25f));
        g.fillPath(in_path_);
        g.setColour(base_.getTextColour().withAlpha(.9f));
        g.strokePath(out_path_,
                     juce::PathStrokeType(curve_thickness_,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void RMSPanel::run(const double sample_rate, const zldsp::container::AbstractFIFO::Range range) {
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
        in_receiver_.run(range, analyzer_sender_.getSampleFIFOs()[0]);
        if (out_receiver_.run(range, analyzer_sender_.getSampleFIFOs()[2])) {
            in_receiver_.updateHeight(bound.getWidth(), in_xs_);
            out_receiver_.updateHeight(bound.getWidth(), out_xs_);

            next_in_path_.clear();
            next_out_path_.clear();
            next_in_path_.startNewSubPath(in_xs_[0], ys_[0]);
            next_out_path_.startNewSubPath(out_xs_[0], ys_[0]);
            for (size_t i = 1; i < kNumPoints; ++i) {
                next_in_path_.lineTo(in_xs_[i], ys_[i]);
                next_out_path_.lineTo(out_xs_[i], ys_[i]);
            }
            next_in_path_.closeSubPath();
            std::lock_guard<std::mutex> lock{mutex_};
            in_path_ = next_in_path_;
            out_path_ = next_out_path_;
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
