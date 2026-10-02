// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "meter_display_panel.hpp"

namespace zlpanel {
    MeterDisplayPanel::MeterDisplayPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        meter_top_panel_(base),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        analyzer_mag_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMagType::kID)) {

        const auto target_refresh_id = p.state_.getRawParameterValue(
            zlstate::PTargetRefreshSpeed::kID)->load(std::memory_order::relaxed);
        const auto circular_capacity = static_cast<size_t>(
            zlstate::PTargetRefreshSpeed::kRates[static_cast<size_t>(std::round(target_refresh_id))]);
        circular_min_max_.setCapacity(circular_capacity);
        circular_min_max_.setSize(circular_capacity);

        meter_top_panel_.setBufferedToImage(true);
        addAndMakeVisible(meter_top_panel_);
    }

    MeterDisplayPanel::~MeterDisplayPanel() = default;

    void MeterDisplayPanel::paint(juce::Graphics& g) {
        g.setFont(base_.getFontSize());
        const auto text_height = base_.getFontSize() * 1.25f;
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        for (auto& a_bound : reduction_rect_) {
            g.fillRect(a_bound.load());
        }
        const auto reduction_max_rect = reduction_max_rect_.load();
        if (reduction_max_rect.getY() > .5f * text_height) {
            g.fillRect(reduction_max_rect);

            const auto reduction_max = reduction_max_value_.load(std::memory_order::relaxed);
            const auto is_upwards = a_is_upwards_.load(std::memory_order::relaxed);
            g.setColour(base_.getTextColour());
            g.drawText(formatValue(std::abs(reduction_max)),
                       juce::Rectangle<float>{reduction_max_rect.getX(),
                                              is_upwards
                                              ? reduction_max_rect.getY() - text_height
                                              : reduction_max_rect.getY(),
                                              reduction_max_rect.getWidth(), text_height},
                       juce::Justification::centred, false);
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
        for (auto& a_bound : pre_rect_) {
            g.fillRect(a_bound.load());
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
        for (auto& a_bound : out_rect_) {
            g.fillRect(a_bound.load());
        }
        for (auto& a_bound : out_arrow_) {
            g.fillRect(a_bound.load());
        }
    }

    void MeterDisplayPanel::resized() {
        auto bound = getLocalBounds();
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        meter_top_panel_.setBounds(bound.removeFromTop(getTopPanelHeight(font_size) - padding / 2));
        bound.removeFromTop(padding / 2);
        pending_bound_.store(bound.toFloat());
        lookAndFeelChanged();
    }

    void MeterDisplayPanel::updateSize() {
        if (!size_changed_.check()) {
            return;
        }
        bound_ = pending_bound_.load();
        const auto meter_width = bound_.getWidth() * .2f;
        const auto meter_padding = meter_width * .5f;

        constexpr auto x1 = 0.f;
        const auto x2 = x1 + meter_width + meter_padding * .5f;
        const auto x3 = x2 + meter_width + meter_padding;
        const auto x4 = x3 + meter_width + meter_padding * .5f;

        reduction_rect_[0].store({x1, 0.f, meter_width, 0.f});
        reduction_rect_[1].store({x2, 0.f, meter_width, 0.f});

        pre_rect_[0].store({x3, 0.f, meter_width, 0.f});
        pre_rect_[1].store({x4, 0.f, meter_width, 0.f});

        const auto thickness = pending_thickness_.load(std::memory_order::relaxed);

        out_rect_[0].store({x3, 0.f, meter_width, thickness});
        out_rect_[1].store({x4, 0.f, meter_width, thickness});

        out_arrow_[0].store({x3 + meter_width * .5f - thickness * .5f,
                             0.f, thickness, 0.f});
        out_arrow_[1].store({x4 + meter_width * .5f - thickness * .5f,
                             0.f, thickness, 0.f});

        reduction_max_rect_.store({x1, 0.f, x2 + meter_width, thickness});
    }

    void MeterDisplayPanel::repaintCallBackSlow() {
        meter_top_panel_.updateValue(reduction_peak_.load(std::memory_order::relaxed),
                                     out_peak_.load(std::memory_order::relaxed));
    }

    void MeterDisplayPanel::run(const double next_time_stamp,
                                zldsp::analyzer::FIFOTransferBuffer<zlp::CompressController::kAnalyzerStreamNum>& transfer_buffer,
                                const size_t consumer_id, const MagDBRange& db_range) {
        updateSize();
        if (is_first_point_) {
            is_first_point_ = false;
            start_time_ = next_time_stamp;
            return;
        }
        if (reset_peaks_.exchange(false, std::memory_order::relaxed)) {
            reduction_peak_.store(0.f, std::memory_order::relaxed);
            out_peak_.store(-240.f, std::memory_order::relaxed);
        }
        const auto mag_type = static_cast<zldsp::analyzer::MagType>(std::round(
            analyzer_mag_type_ref_.load(std::memory_order::relaxed)));

        const auto delta_time = std::clamp(next_time_stamp - start_time_, 0.01, 1.0);
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
        reduction_receiver_.run(
            range,
            transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerPreStream],
            transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerCompressedStream],
            mag_type);
        pre_receiver_.run(
            range, transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerPreStream], mag_type);
        out_receiver_.run(
            range, transfer_buffer.getSampleFIFOs()[zlp::CompressController::kAnalyzerPostStream], mag_type);
        fifo.finishRead(consumer_id, num_to_read);

        const auto& reduction_dbs{reduction_receiver_.getReductions()};
        const auto& pre_dbs{pre_receiver_.getDBs()};
        const auto& out_dbs{out_receiver_.getDBs()};

        const auto bound = bound_;
        const auto thickness = pending_thickness_.load(std::memory_order::relaxed);
        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        is_upwards_ = (direction == zlp::PCompDirection::kInflate || direction == zlp::PCompDirection::kShape);
        a_is_upwards_.store(is_upwards_, std::memory_order::relaxed);
        // update reduction peak
        if (is_upwards_) {
            const auto reduction_peak = std::max(reduction_dbs[0], reduction_dbs[1]);
            reduction_peak_.store(std::max(reduction_peak, reduction_peak_.load(std::memory_order::relaxed)),
                                  std::memory_order::relaxed);
        } else {
            const auto reduction_peak = std::min(reduction_dbs[0], reduction_dbs[1]);
            reduction_peak_.store(std::min(reduction_peak, reduction_peak_.load(std::memory_order::relaxed)),
                                  std::memory_order::relaxed);
        }
        // update reduction meter
        for (size_t chan = 0; chan < 2; ++chan) {
            const auto current_reduction = reduction_dbs[chan];
            const auto previous_reduction = previous_reduction_[chan];
            if (is_upwards_) {
                previous_reduction_[chan] = std::max(
                    previous_reduction - static_cast<float>(delta_time) * kReductionDecayPerSecond,
                    current_reduction);
                const auto reduction_rect = reduction_rect_[chan].load();
                const auto reduction_height = std::abs(
                    db_range.getReductionYProportion(previous_reduction_[chan])) * bound.getHeight();
                if (previous_reduction_[chan] > 0.f) {
                    reduction_rect_[chan].store({
                        reduction_rect.getX(), bound.getCentreY() - reduction_height,
                        reduction_rect.getWidth(), reduction_height});
                } else {
                    reduction_rect_[chan].store({
                        reduction_rect.getX(), bound.getCentreY(),
                        reduction_rect.getWidth(), reduction_height});
                }
            } else {
                previous_reduction_[chan] = std::min(
                    previous_reduction + static_cast<float>(delta_time) * kReductionDecayPerSecond,
                    current_reduction);
                const auto reduction_rect = reduction_rect_[chan].load();
                reduction_rect_[chan].store({
                    reduction_rect.getX(), bound.getY(),
                    reduction_rect.getWidth(),
                    db_range.getReductionYProportion(previous_reduction_[chan]) * bound.getHeight()});
            }
        }
        // update reduction short-term max display
        float reduction_max_value;
        float reduction_max_pos;
        if (is_upwards_) {
            reduction_max_value = std::max(0.f, std::max(reduction_dbs[0], reduction_dbs[1]));
            reduction_max_value = circular_min_max_.push(reduction_max_value);
            reduction_max_pos = (.5f + db_range.getReductionYProportion(reduction_max_value)) * bound.getHeight();
        } else {
            reduction_max_value = std::max(0.f, std::max(-reduction_dbs[0], -reduction_dbs[1]));
            reduction_max_value = circular_min_max_.push(reduction_max_value);
            reduction_max_pos = -db_range.getReductionYProportion(reduction_max_value) * bound.getHeight();
        }
        reduction_max_rect_.setY(reduction_max_pos + bound.getY() - thickness * .5f);
        reduction_max_value_.store(reduction_max_value, std::memory_order::relaxed);
        // update out peak
        const auto out_peak = std::max(out_dbs[0], out_dbs[1]);
        out_peak_.store(std::max(out_peak, out_peak_.load(std::memory_order::relaxed)),
                        std::memory_order::relaxed);
        // update independent meter decays, then constrain their displayed gap
        const auto meter_delta_time = static_cast<float>(delta_time);
        const auto meter_decay = meter_delta_time * kMeterDecayPerSecond;
        const auto decay_acceleration = 1.f + 3.f * meter_delta_time;
        for (size_t chan = 0; chan < 2; ++chan) {
            const auto current_pre = pre_dbs[chan];
            const auto current_out = out_dbs[chan];
            // advance from the last displayed gap
            const auto previous_gap = static_cast<double>(previous_out_[chan]) - previous_pre_[chan];
            const auto target_gap = static_cast<double>(current_out) - current_pre;
            if (std::abs(target_gap - target_gap_db_[chan]) > 1e-5) {
                target_gap_db_[chan] = target_gap;
                gap_remaining_seconds_[chan] = kMeterGapConvergenceSeconds;
            }
            const auto remaining_seconds = gap_remaining_seconds_[chan];
            auto gap = remaining_seconds > delta_time
                ? std::lerp(previous_gap, target_gap, delta_time / remaining_seconds)
                : target_gap;
            gap_remaining_seconds_[chan] = std::max(0.0, remaining_seconds - delta_time);

            const auto pre_attack = current_pre > previous_pre_[chan];
            const auto out_attack = current_out > previous_out_[chan];
            previous_pre_[chan] = std::max(previous_pre_[chan] - pre_decay_mul_[chan] * meter_decay, current_pre);
            previous_out_[chan] = std::max(previous_out_[chan] - out_decay_mul_[chan] * meter_decay, current_out);
            pre_decay_mul_[chan] = pre_attack
                ? 1.f
                : std::min(pre_decay_mul_[chan] * decay_acceleration, 10.f);
            out_decay_mul_[chan] = out_attack
                ? 1.f
                : std::min(out_decay_mul_[chan] * decay_acceleration, 10.f);

            gap = std::clamp(gap, static_cast<double>(current_out) - previous_pre_[chan],
                             static_cast<double>(previous_out_[chan]) - current_pre);
            if (target_gap > 0.0) {
                gap = std::max(gap, 0.0);
            } else if (target_gap < 0.0) {
                gap = std::min(gap, 0.0);
            }
            // keep the corrected positions as the next frame's decay state
            if (static_cast<double>(previous_out_[chan]) - previous_pre_[chan] > gap) {
                previous_out_[chan] = std::clamp(static_cast<float>(previous_pre_[chan] + gap),
                                                 current_out, previous_out_[chan]);
            } else {
                previous_pre_[chan] = std::clamp(static_cast<float>(previous_out_[chan] - gap),
                                                 current_pre, previous_pre_[chan]);
            }

            const auto pre_y = db_range.getYProportion(previous_pre_[chan]) * bound.getHeight() + bound.getY();
            pre_rect_[chan].setY(pre_y);
            pre_rect_[chan].setHeight(bound.getBottom() - pre_y);

            const auto out_y = db_range.getYProportion(previous_out_[chan]) * bound.getHeight() + bound.getY();
            out_rect_[chan].setY(out_y - thickness * .5f);

            out_arrow_[chan].setY(std::min(pre_y, out_y));
            out_arrow_[chan].setHeight(std::abs(out_y - pre_y));
        }
    }

    void MeterDisplayPanel::mouseDoubleClick(const juce::MouseEvent&) {
        reset_peaks_.store(true, std::memory_order::relaxed);
    }

    std::string MeterDisplayPanel::formatValue(const float value) {
        std::stringstream ss;
        const auto abs_value = std::abs(value);
        if (abs_value < 10.f) {
            ss << std::fixed << std::setprecision(2) << value;
        } else if (abs_value < 100.f) {
            ss << std::fixed << std::setprecision(1) << value;
        } else {
            ss << std::fixed << std::setprecision(0) << value;
        }
        return ss.str();
    }

    void MeterDisplayPanel::lookAndFeelChanged() {
        pending_thickness_.store(base_.getFontSize() * .2f * base_.getMagCurveThickness(), std::memory_order::relaxed);
        size_changed_.signal();
    }
}
