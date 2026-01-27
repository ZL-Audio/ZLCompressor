// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "peak_panel.hpp"

namespace zlpanel {
    PeakPanel::PeakPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        analyzer_stereo_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerStereo::kID)),
        analyzer_mag_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMagType::kID)),
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)),
        analyzer_time_length_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerTimeLength::kID)) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::CompressController::kAnalyzerPointNum) * 3 + 1;
        for (auto& path : {&in_path_, &out_path_, &reduction_path_}) {
            path->preallocateSpace(preallocateSpace);
        }
        setInterceptsMouseClicks(false, false);
    }

    PeakPanel::~PeakPanel() = default;

    void PeakPanel::paint(juce::Graphics& g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
        g.fillPath(in_path_);
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
        g.strokePath(out_path_,
                     juce::PathStrokeType(curve_thickness_,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        g.strokePath(reduction_path_,
                     juce::PathStrokeType(curve_thickness_,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void PeakPanel::resized() {
        auto bound = getLocalBounds();
        atomic_bound_.store(bound.toFloat());
        lookAndFeelChanged();
    }

    void PeakPanel::run(const double next_time_stamp, RMSPanel& rms_panel,
                        zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer,
                        size_t consumer_id) {
        const auto bound = atomic_bound_.load();
        const auto stereo_type = static_cast<zldsp::analyzer::StereoType>(std::round(
            analyzer_stereo_type_ref_.load(std::memory_order::relaxed)));
        const auto mag_type = static_cast<zldsp::analyzer::MagType>(std::round(
            analyzer_mag_type_ref_.load(std::memory_order::relaxed)));
        const auto min_db = zlstate::PAnalyzerMinDB::kDBs[static_cast<size_t>(std::round(
            analyzer_min_db_ref_.load(std::memory_order::relaxed)))];
        const auto time_length_idx = analyzer_time_length_ref_.load(std::memory_order::relaxed);

        const auto sample_rate = transfer_buffer.getSampleRate();
        const auto max_num_samples = transfer_buffer.getMaxNumSamples();

        if (sample_rate > 20000.0 && max_num_samples > 0) {
            if (std::abs(sample_rate_ - sample_rate) > 0.1 ||
                max_num_samples_ != max_num_samples ||
                std::abs(time_length_idx_ - time_length_idx) > 0.1) {
                sample_rate_ = sample_rate;
                max_num_samples_ = max_num_samples;
                time_length_idx_ = time_length_idx;
                const auto time_idx = static_cast<size_t>(std::round(time_length_idx_));
                num_points_per_second_ = kNumPointsPerSecond[time_idx];
                num_samples_per_point_ = static_cast<int>(sample_rate_) / num_points_per_second_;
                time_length_ = zlstate::PAnalyzerTimeLength::kLength[time_idx];
                is_first_point_ = true;
                num_points_ = static_cast<size_t>(num_points_per_second_) * static_cast<size_t>(time_length_);
                second_per_point_ = static_cast<double>(time_length_) / static_cast<double>(num_points_);

                xs_.resize(num_points_ + 2);
                pre_ys_.resize(num_points_ + 2);
                out_ys_.resize(num_points_ + 2);
                post_ys_.resize(num_points_ + 2);
            }
        }

        auto& fifo{transfer_buffer.getMulticastFIFO()};
        if (!is_first_point_) {
            // update ys
            while (next_time_stamp - start_time_ > second_per_point_) {
                // if not enough samples
                if (fifo.getNumReady(consumer_id) >= num_samples_per_point_) {
                    const auto range = fifo.prepareToRead(consumer_id, num_samples_per_point_);
                    rms_panel.run(sample_rate_, range, transfer_buffer);
                    analyzer_receiver_.run(range, transfer_buffer.getSampleFIFOs(),
                                           mag_type, stereo_type);
                    fifo.finishRead(consumer_id, num_samples_per_point_);
                    num_missing_points_ = 0;
                } else {
                    if (num_missing_points_ < kPausedThreshold) {
                        num_missing_points_ += 1;
                    } else if (num_missing_points_ == kPausedThreshold) {
                        for (auto y : {std::span{pre_ys_}, std::span{post_ys_}, std::span{out_ys_}}) {
                            const auto start_idx = y.size() - static_cast<size_t>(kPausedThreshold);
                            for (size_t idx = start_idx; idx < y.size(); ++idx) {
                                y[idx] = 100000.f;
                            }
                        }
                    }
                }
                if (num_missing_points_ < kPausedThreshold) {
                    analyzer_receiver_.updateY(bound.getHeight(), 0.f, min_db,
                                               {std::span{pre_ys_}, std::span{post_ys_}, std::span{out_ys_}});
                } else {
                    for (auto y : {std::span{pre_ys_}, std::span{post_ys_}, std::span{out_ys_}}) {
                        std::ranges::rotate(y, y.begin() + 1);
                        y.back() = 100000.f;
                    }
                }
                start_time_ += second_per_point_;
            }
            // if too much samples
            const auto num_ready = fifo.getNumReady(consumer_id);
            const auto threshold = 2 * std::max(static_cast<int>(max_num_samples_), num_samples_per_point_);
            if (num_ready > threshold) {
                too_much_samples_ += (num_ready - threshold) / num_samples_per_point_;
                if (too_much_samples_ > kTooMuchResetThreshold) {
                    (void)fifo.prepareToRead(consumer_id, num_ready - threshold);
                    fifo.finishRead(consumer_id, num_ready - threshold);
                    too_much_samples_ = 0;
                }
            } else {
                too_much_samples_ = 0;
            }
        } else {
            if (num_samples_per_point_ > 0 && fifo.getNumReady(consumer_id) >= num_samples_per_point_) {
                is_first_point_ = false;
                start_time_ = next_time_stamp;
                std::ranges::fill(pre_ys_, 100000.f);
                std::ranges::fill(out_ys_, 100000.f);
                std::ranges::fill(post_ys_, 100000.f);
            }
        }

        // update xs
        if (!is_first_point_) {
            const auto x_scale = static_cast<double>(bound.getWidth()) / static_cast<double>(time_length_);
            auto x0 = -(next_time_stamp - start_time_) * x_scale;
            const auto delta_x = second_per_point_ * x_scale;
            for (size_t i = 0; i < xs_.size(); ++i) {
                xs_[i] = static_cast<float>(x0);
                x0 += delta_x;
            }
        }

        if (!is_first_point_) {
            const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
                comp_direction_ref_.load(std::memory_order::relaxed)));
            if (direction == zlp::PCompDirection::kInflate || direction == zlp::PCompDirection::kShape) {
                updatePaths<true>(bound);
            } else {
                updatePaths<false>(bound);
            }
            std::lock_guard<std::mutex> lock{mutex_};
            in_path_.swapWithPath(next_in_path_);
            out_path_.swapWithPath(next_out_path_);
            reduction_path_.swapWithPath(next_reduction_path_);
        }
    }

    template <bool center>
    void PeakPanel::updatePaths(const juce::Rectangle<float> bound) {
        next_in_path_.clear();
        next_out_path_.clear();
        next_reduction_path_.clear();

        next_in_path_.startNewSubPath(xs_[0], bound.getBottom());
        next_in_path_.lineTo(xs_[0], pre_ys_[0]);
        next_out_path_.startNewSubPath(xs_[0], out_ys_[0]);
        next_reduction_path_.startNewSubPath(xs_[0], post_ys_[0] - pre_ys_[0]);
        const auto center_y = bound.getCentreY();
        for (size_t i = 1; i < xs_.size(); ++i) {
            next_in_path_.lineTo(xs_[i], pre_ys_[i]);
            next_out_path_.lineTo(xs_[i], out_ys_[i]);
            if constexpr (center) {
                next_reduction_path_.lineTo(xs_[i], post_ys_[i] - pre_ys_[i] + center_y);
            } else {
                next_reduction_path_.lineTo(xs_[i], post_ys_[i] - pre_ys_[i]);
            }
        }
        next_in_path_.lineTo(xs_[xs_.size() - 1], bound.getBottom());
    }

    void PeakPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .2f * base_.getMagCurveThickness();
    }
}
