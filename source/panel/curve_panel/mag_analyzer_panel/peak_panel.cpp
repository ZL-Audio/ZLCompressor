// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "peak_panel.hpp"

namespace zlpanel {
    PeakPanel::PeakPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : p_ref_(processor), base_(base),
          mag_analyzer_ref_(processor.getCompressController().getMagAnalyzer()) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::CompressController::kAnalyzerPointNum) * 3 + 1;
        for (auto &path: {&in_path_, &out_path_, &reduction_path_}) {
            path->preallocateSpace(preallocateSpace);
        }
        mag_analyzer_ref_.setMagType(zldsp::analyzer::MagType::kPeak);
        mag_analyzer_ref_.setToReset();

        for (auto &ID: kNAIDs) {
            p_ref_.na_parameters_.addParameterListener(ID, this);
            parameterChanged(ID, p_ref_.na_parameters_.getRawParameterValue(ID)->load(std::memory_order::relaxed));
        }

        setInterceptsMouseClicks(false, false);
    }

    PeakPanel::~PeakPanel() {
        for (auto &ID: kNAIDs) {
            p_ref_.na_parameters_.removeParameterListener(ID, this);
        }
    }

    void PeakPanel::paint(juce::Graphics &g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
        g.fillPath(in_path_);
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
        g.strokePath(out_path_,
                     juce::PathStrokeType(base_.getFontSize() * .2f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        g.strokePath(reduction_path_,
                     juce::PathStrokeType(base_.getFontSize() * .2f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void PeakPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        constexpr auto pad_p = 1.f / static_cast<float>(zlp::CompressController::kAnalyzerPointNum - 1);
        const auto pad = std::max(bound.getWidth() * pad_p, 1.f);
        bound = bound.withWidth(bound.getWidth() + pad);
        atomic_bound_.store(bound);
    }

    void PeakPanel::run(const double next_time_stamp) {
        const auto current_bound = atomic_bound_.load();
        if (to_reset_path_.exchange(false, std::memory_order::acquire)) {
            is_first_point_ = true;
        }
        if (is_first_point_) {
            auto [actual_delta, to_reset_shift] = mag_analyzer_ref_.run(1, 0);
            if (actual_delta > 0) {
                is_first_point_ = false;
                current_count_ = 0.;
                start_time_ = next_time_stamp;
                mag_analyzer_ref_.createReductionPath(xs_, in_ys_, out_ys_, reduction_ys_,
                                                      current_bound.getWidth(), current_bound.getHeight(), 0.f,
                                                      analyzer_min_db_.load(std::memory_order::relaxed), 0.f);
                updatePaths(current_bound);
            }
        } else {
            const auto c_num_per_second = num_per_second_.load(std::memory_order::relaxed);
            const auto tolerance = std::max(1.0, c_num_per_second / 15.f);
            const auto target_count = (next_time_stamp - start_time_) * c_num_per_second;
            const auto target_delta = target_count - current_count_;
            auto [actual_delta, to_reset_shift] = mag_analyzer_ref_.run(
                static_cast<int>(std::floor(target_delta)),
                static_cast<int>(std::round(tolerance)));
            current_count_ += static_cast<double>(actual_delta);

            if (to_reset_shift) {
                start_time_ = next_time_stamp;
                current_count_ = 0.;
                consecutive_reset_count_ = 1.0;
            } else {
                consecutive_reset_count_ *= 0.9;
            }

            const auto shift = to_reset_shift ? 0.0 : target_count - current_count_;
            mag_analyzer_ref_.createReductionPath(xs_, in_ys_, out_ys_, reduction_ys_,
                                                  current_bound.getWidth(), current_bound.getHeight(),
                                                  static_cast<float>(shift),
                                                  analyzer_min_db_.load(std::memory_order::relaxed), 0.f);
            if (consecutive_reset_count_ < 0.75) {
                updatePaths(current_bound);
            }
        }
        if (consecutive_reset_count_ < 0.75) {
            std::lock_guard<std::mutex> lock{mutex_};
            in_path_.swapWithPath(next_in_path_);
            out_path_.swapWithPath(next_out_path_);
            reduction_path_.swapWithPath(next_reduction_path_);
        }
    }

    void PeakPanel::updatePaths(const juce::Rectangle<float> bound) {
        next_in_path_.clear();
        next_out_path_.clear();
        next_reduction_path_.clear();

        next_in_path_.startNewSubPath(xs_[0], bound.getBottom());
        next_in_path_.lineTo(xs_[0], in_ys_[0]);
        next_out_path_.startNewSubPath(xs_[0], out_ys_[0]);
        next_reduction_path_.startNewSubPath(xs_[0], reduction_ys_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            next_in_path_.lineTo(xs_[i], in_ys_[i]);
            next_out_path_.lineTo(xs_[i], out_ys_[i]);
            next_reduction_path_.lineTo(xs_[i], reduction_ys_[i]);
        }
        next_in_path_.lineTo(xs_[xs_.size() - 1], bound.getBottom());
    }

    void PeakPanel::parameterChanged(const juce::String &parameter_id, const float new_value) {
        if (parameter_id == zlstate::PAnalyzerMagType::kID) {
            mag_analyzer_ref_.setMagType(static_cast<zldsp::analyzer::MagType>(std::round(new_value)));
        } else if (parameter_id == zlstate::PAnalyzerMinDB::kID) {
            analyzer_min_db_.store(zlstate::PAnalyzerMinDB::getMinDBFromIndex(new_value), std::memory_order::relaxed);
        } else if (parameter_id == zlstate::PAnalyzerTimeLength::kID) {
            setTimeLength(zlstate::PAnalyzerTimeLength::getTimeLengthFromIndex(new_value));
        }
    }
}
