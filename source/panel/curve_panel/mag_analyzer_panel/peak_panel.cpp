// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "peak_panel.hpp"

namespace {
    constexpr auto kMaxAnalyzerPointNum = 300;
    constexpr auto kMissingDB = -10000.f;

    constexpr int chooseNumPointsPerSecond(const int sample_rate,
                                           const int preferred_num_points_per_second,
                                           const int time_length) {
        if (sample_rate <= 0 || preferred_num_points_per_second <= 0 || time_length <= 0) {
            return preferred_num_points_per_second;
        }

        const auto max_num_points_per_second = static_cast<int>(kMaxAnalyzerPointNum) / time_length;
        for (auto candidate = preferred_num_points_per_second;
             candidate <= max_num_points_per_second; ++candidate) {
            if (sample_rate % candidate == 0) {
                return candidate;
            }
        }
        return preferred_num_points_per_second;
    }
}

namespace zlpanel {
    PeakPanel::PeakPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        pre_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PPreCurveDisplay::kID)),
        post_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PPostCurveDisplay::kID)),
        delta_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PDeltaCurveDisplay::kID)),
        side_curve_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideChainCurveDisplay::kID)),
        analyzer_stereo_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerStereo::kID)),
        analyzer_mag_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMagType::kID)),
        analyzer_time_length_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerTimeLength::kID)) {
        constexpr auto preallocateSpace = static_cast<int>(kMaxAnalyzerPointNum) * 3 + 1;
        for (auto& path : in_path_.get_buffer()) {
            path.preallocateSpace(preallocateSpace);
        }
        for (auto& path : out_path_.get_buffer()) {
            path.preallocateSpace(preallocateSpace);
        }
        for (auto& path : side_chain_path_.get_buffer()) {
            path.preallocateSpace(preallocateSpace);
        }
        for (auto& path : reduction_path_.get_buffer()) {
            path.preallocateSpace(preallocateSpace);
        }
        setInterceptsMouseClicks(false, false);
    }

    PeakPanel::~PeakPanel() = default;

    void PeakPanel::paint(juce::Graphics& g) {
        if (pre_curve_display_ref_.load(std::memory_order::relaxed) > .5f) {
            in_path_.pull();
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
            g.fillPath(in_path_.get_reader());
        }
        if (side_curve_display_ref_.load(std::memory_order::relaxed) > .5f) {
            side_chain_path_.pull();
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kSideChainColour));
            g.strokePath(side_chain_path_.get_reader(),
                         juce::PathStrokeType(curve_thickness_,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
        if (post_curve_display_ref_.load(std::memory_order::relaxed) > .5f) {
            out_path_.pull();
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
            g.strokePath(out_path_.get_reader(),
                         juce::PathStrokeType(curve_thickness_,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
        if (delta_curve_display_ref_.load(std::memory_order::relaxed) > .5f) {
            reduction_path_.pull();
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
            g.strokePath(reduction_path_.get_reader(),
                         juce::PathStrokeType(curve_thickness_,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
    }

    void PeakPanel::resized() {
        const auto bound = getLocalBounds();
        atomic_bound_.store(bound.toFloat());
        lookAndFeelChanged();
    }

    void PeakPanel::run(const double next_time_stamp, RMSPanel& rms_panel,
                        zldsp::analyzer::FIFOTransferBuffer<
                            zlp::CompressController::kAnalyzerStreamNum>& transfer_buffer,
                        const size_t consumer_id, const MagDBRange& db_range) {
        const auto bound = atomic_bound_.load();
        const auto stereo_type = static_cast<zldsp::analyzer::StereoType>(std::round(
            analyzer_stereo_type_ref_.load(std::memory_order::relaxed)));
        const auto mag_type = static_cast<zldsp::analyzer::MagType>(std::round(
            analyzer_mag_type_ref_.load(std::memory_order::relaxed)));
        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        const auto center_reduction = direction == zlp::PCompDirection::kInflate ||
                                      direction == zlp::PCompDirection::kShape;
        const auto time_length_idx = analyzer_time_length_ref_.load(std::memory_order::relaxed);

        const auto sample_rate = transfer_buffer.getSampleRate();
        const auto max_num_samples = transfer_buffer.getMaxNumSamples();

        if (std::abs(sample_rate_ - sample_rate) > 0.1 ||
            max_num_samples_ != max_num_samples ||
            std::abs(time_length_idx_ - time_length_idx) > 0.1) {
            sample_rate_ = sample_rate;
            max_num_samples_ = max_num_samples;
            time_length_idx_ = time_length_idx;
            const auto time_idx = static_cast<size_t>(std::round(time_length_idx_));
            time_length_ = zlstate::PAnalyzerTimeLength::kLength[time_idx];
            const auto rounded_sample_rate = static_cast<int>(std::round(sample_rate_));
            num_points_per_second_ = chooseNumPointsPerSecond(
                rounded_sample_rate, kNumPointsPerSecond[time_idx], static_cast<int>(std::round(time_length_)));
            num_samples_per_point_ = rounded_sample_rate / num_points_per_second_;
            is_first_point_ = true;
            num_points_ = static_cast<size_t>(num_points_per_second_) * static_cast<size_t>(time_length_);
            second_per_point_ = static_cast<double>(time_length_) / static_cast<double>(num_points_);

            xs_.resize(num_points_ + 2);
            pre_ys_.resize(num_points_ + 2);
            out_ys_.resize(num_points_ + 2);
            side_chain_ys_.resize(num_points_ + 2);
            reduction_ys_.resize(num_points_ + 2);
        }

        updateYMapping(bound, db_range, center_reduction);
        const auto missing_y = std::fma(kMissingDB, db_to_y_scale_, db_to_y_bias_);

        auto& fifo{transfer_buffer.getMulticastFIFO()};
        if (!is_first_point_) {
            // update ys
            while (next_time_stamp - start_time_ > second_per_point_) {
                // if not enough samples
                if (fifo.getNumReady(consumer_id) >= num_samples_per_point_) {
                    const auto range = fifo.prepareToRead(consumer_id, num_samples_per_point_);
                    rms_panel.run(sample_rate_, range, transfer_buffer, db_range);
                    pre_db_ = zldsp::analyzer::MagReceiver::calculate(
                        range,
                        transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerPreStream],
                        mag_type, stereo_type);
                    out_db_ = zldsp::analyzer::MagReceiver::calculate(
                        range,
                        transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerPostStream],
                        mag_type, stereo_type);
                    side_chain_db_ = zldsp::analyzer::MagReceiver::calculate(
                        range,
                        transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerSideChainStream],
                        mag_type, stereo_type);
                    reduction_db_ = zldsp::analyzer::MagReductionReceiver::calculateReduction(
                        range,
                        transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerPreStream],
                        transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerCompressedStream],
                        mag_type, stereo_type);
                    fifo.finishRead(consumer_id, num_samples_per_point_);
                    num_missing_points_ = 0;
                } else {
                    if (num_missing_points_ < kPausedThreshold) {
                        num_missing_points_ += 1;
                    } else if (num_missing_points_ == kPausedThreshold) {
                        const auto shift = static_cast<ptrdiff_t>(
                            pre_ys_.size() - static_cast<size_t>(kPausedThreshold));
                        std::ranges::fill(pre_ys_.begin() + shift, pre_ys_.end(), missing_y);
                        std::ranges::fill(out_ys_.begin() + shift, out_ys_.end(), missing_y);
                        std::ranges::fill(side_chain_ys_.begin() + shift, side_chain_ys_.end(), missing_y);
                        std::ranges::fill(reduction_ys_.begin() + shift, reduction_ys_.end(), reduction_y_bias_);
                    }
                }
                {
                    const auto too_many_missing = num_missing_points_ >= kPausedThreshold;
                    std::ranges::rotate(pre_ys_, pre_ys_.begin() + 1);
                    pre_ys_.back() = too_many_missing
                                         ? missing_y
                                         : std::fma(pre_db_, db_to_y_scale_, db_to_y_bias_);
                    std::ranges::rotate(out_ys_, out_ys_.begin() + 1);
                    out_ys_.back() = too_many_missing
                                         ? missing_y
                                         : std::fma(out_db_, db_to_y_scale_, db_to_y_bias_);
                    std::ranges::rotate(side_chain_ys_, side_chain_ys_.begin() + 1);
                    side_chain_ys_.back() = too_many_missing
                                                ? missing_y
                                                : std::fma(side_chain_db_, db_to_y_scale_, db_to_y_bias_);
                    std::ranges::rotate(reduction_ys_, reduction_ys_.begin() + 1);
                    reduction_ys_.back() = too_many_missing
                                               ? reduction_y_bias_
                                               : std::fma(reduction_db_, db_to_y_scale_, reduction_y_bias_);
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
            if (fifo.getNumReady(consumer_id) >= num_samples_per_point_) {
                is_first_point_ = false;
                start_time_ = next_time_stamp;
                std::ranges::fill(pre_ys_, missing_y);
                std::ranges::fill(out_ys_, missing_y);
                std::ranges::fill(side_chain_ys_, missing_y);
                std::ranges::fill(reduction_ys_, reduction_y_bias_);
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
            updatePaths(bound);
            in_path_.publish();
            out_path_.publish();
            side_chain_path_.publish();
            reduction_path_.publish();
        }
    }

    void PeakPanel::updateYMapping(const juce::Rectangle<float> bound,
                                   const MagDBRange& db_range,
                                   const bool center_reduction) {
        const auto range_db = db_range.getRangeDB();
        const auto effective_height = std::max(bound.getHeight(), 1.f);
        const auto next_scale = std::abs(range_db) > std::numeric_limits<float>::epsilon()
                                    ? effective_height / range_db
                                    : 0.f;
        const auto next_bias = std::fma(-db_range.getMaxDB(), next_scale, bound.getY());
        const auto next_reduction_bias = center_reduction ? bound.getCentreY() : bound.getY();

        if (is_y_mapping_initialized_ && !is_first_point_) {
            const auto scale_changed = std::abs(next_scale - db_to_y_scale_) > 1e-6f;
            const auto bias_changed = std::abs(next_bias - db_to_y_bias_) > 1e-6f;
            const auto reduction_bias_changed = std::abs(next_reduction_bias - reduction_y_bias_) > 1e-6f;
            if ((scale_changed || bias_changed || reduction_bias_changed) &&
                std::abs(db_to_y_scale_) > std::numeric_limits<float>::epsilon()) {
                const auto scale = next_scale / db_to_y_scale_;
                const auto size = pre_ys_.size();
                if (scale_changed || bias_changed) {
                    const auto magnitude_bias = std::fma(-db_to_y_bias_, scale, next_bias);
                    zldsp::vector::fma(pre_ys_.data(), scale, magnitude_bias, size);
                    zldsp::vector::fma(out_ys_.data(), scale, magnitude_bias, size);
                    zldsp::vector::fma(side_chain_ys_.data(), scale, magnitude_bias, size);
                }
                if (scale_changed || reduction_bias_changed) {
                    const auto reduction_bias = std::fma(-reduction_y_bias_, scale, next_reduction_bias);
                    zldsp::vector::fma(reduction_ys_.data(), scale, reduction_bias, size);
                }
            }
        }

        db_to_y_scale_ = next_scale;
        db_to_y_bias_ = next_bias;
        reduction_y_bias_ = next_reduction_bias;
        is_y_mapping_initialized_ = true;
    }

    void PeakPanel::updatePaths(const juce::Rectangle<float> bound) {
        auto& next_in_path{in_path_.get_writer()};
        auto& next_out_path{out_path_.get_writer()};
        auto& next_side_chain_path{side_chain_path_.get_writer()};
        auto& next_reduction_path{reduction_path_.get_writer()};
        next_in_path.clear();
        next_out_path.clear();
        next_side_chain_path.clear();
        next_reduction_path.clear();

        const auto size = pre_ys_.size();

        next_in_path.startNewSubPath(xs_[0], bound.getBottom());
        next_in_path.lineTo(xs_[0], pre_ys_[0]);
        next_out_path.startNewSubPath(xs_[0], out_ys_[0]);
        next_side_chain_path.startNewSubPath(xs_[0], side_chain_ys_[0]);
        next_reduction_path.startNewSubPath(xs_[0], reduction_ys_[0]);
        for (size_t i = 1; i < size; ++i) {
            next_in_path.lineTo(xs_[i], pre_ys_[i]);
            next_out_path.lineTo(xs_[i], out_ys_[i]);
            next_side_chain_path.lineTo(xs_[i], side_chain_ys_[i]);
            next_reduction_path.lineTo(xs_[i], reduction_ys_[i]);
        }
        next_in_path.lineTo(xs_[size - 1], bound.getBottom());
    }

    void PeakPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .2f * base_.getMagCurveThickness();
    }
}
