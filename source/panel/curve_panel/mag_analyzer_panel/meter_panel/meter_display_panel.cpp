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
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        analyzer_mag_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMagType::kID)),
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {

        const auto target_refresh_id = p.state_.getRawParameterValue(
            zlstate::PTargetRefreshSpeed::kID)->load(std::memory_order::relaxed);
        const auto circular_capacity = static_cast<size_t>(
            zlstate::PTargetRefreshSpeed::kRates[static_cast<size_t>(std::round(target_refresh_id))]);
        circular_min_max_.setCapacity(circular_capacity);
        circular_min_max_.setSize(circular_capacity);
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
        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        if (direction == zlp::PCompDirection::kInflate || direction == zlp::PCompDirection::kShape) {
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
            for (auto& a_bound : out_rect_) {
                g.fillRect(a_bound.load());
            }
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
            for (auto& a_bound : pre_rect_) {
                g.fillRect(a_bound.load());
            }
        } else {
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
            for (auto& a_bound : pre_rect_) {
                g.fillRect(a_bound.load());
            }
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
            for (auto& a_bound : out_rect_) {
                g.fillRect(a_bound.load());
            }
        }

        auto bound = getLocalBounds().toFloat();
        bound = bound.removeFromTop(text_height);
        g.setColour(base_.getBackgroundColour().withAlpha(.5f));
        g.fillRect(bound);

        const auto meter_width = bound.getWidth() * .45f;
        const auto reduction_peak = reduction_peak_.load(std::memory_order::relaxed);
        g.setColour(base_.getTextColour());
        g.drawText(formatValue(std::abs(reduction_peak)), bound.removeFromLeft(meter_width),
                   juce::Justification::centred, false);

        const auto out_peak = out_peak_.load(std::memory_order::relaxed);
        g.setColour(out_peak < 0.f ? base_.getTextColour() : base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        if (out_peak < -120.f) {
            g.drawText("inf", bound.removeFromRight(meter_width),
                       juce::Justification::centred, false);
        } else {
            g.drawText(formatValue(out_peak), bound.removeFromRight(meter_width),
                       juce::Justification::centred, false);
        }
    }

    void MeterDisplayPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        bound_.store(bound);
        const auto meter_width = bound.getWidth() * .2f;
        const auto meter_padding = meter_width * .5f;
        bound.removeFromLeft(meter_padding);

        constexpr auto x1 = 0.f;
        const auto x2 = x1 + meter_width + meter_padding * .5f;
        const auto x3 = x2 + meter_width + meter_padding;
        const auto x4 = x3 + meter_width + meter_padding * .5f;

        reduction_rect_[0].store({x1, 0.f, meter_width, 0.f});
        reduction_rect_[1].store({x2, 0.f, meter_width, 0.f});

        pre_rect_[0].store({x3, 0.f, meter_width, 0.f});
        pre_rect_[1].store({x4, 0.f, meter_width, 0.f});

        out_rect_[0].store({x3, 0.f, meter_width, 0.f});
        out_rect_[1].store({x4, 0.f, meter_width, 0.f});

        reduction_max_rect_.store({x1, 0.f, x2 + meter_width, base_.getFontSize() * .25f});
    }

    void MeterDisplayPanel::run(const double next_time_stamp,
                                zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer,
                                const size_t consumer_id) {
        if (is_first_point_) {
            is_first_point_ = false;
            start_time_ = next_time_stamp;
            return;
        }
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
        fifo.finishRead(consumer_id, num_to_read);

        const auto& dbs{meter_receiver_.getDBs()};

        const auto bound = bound_.load();
        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        is_upwards_ = (direction == zlp::PCompDirection::kInflate || direction == zlp::PCompDirection::kShape);
        a_is_upwards_.store(is_upwards_, std::memory_order::relaxed);
        // update reduction peak
        if (is_upwards_) {
            const auto reduction_peak = std::max(dbs[1][0] - dbs[0][0], dbs[1][1] - dbs[0][1]);
            reduction_peak_.store(std::max(reduction_peak, reduction_peak_.load(std::memory_order::relaxed)),
                                  std::memory_order::relaxed);
        } else {
            const auto reduction_peak = std::min(dbs[1][0] - dbs[0][0], dbs[1][1] - dbs[0][1]);
            reduction_peak_.store(std::min(reduction_peak, reduction_peak_.load(std::memory_order::relaxed)),
                                  std::memory_order::relaxed);
        }
        // update reduction meter
        for (size_t chan = 0; chan < 2; ++chan) {
            const auto current_reduction = dbs[1][chan] - dbs[0][chan];
            const auto previous_reduction = previous_reduction_[chan];
            if (is_upwards_) {
                if (current_reduction < previous_reduction) {
                    previous_reduction_[chan] = std::max(
                        previous_reduction - static_cast<float>(delta_time) * kReductionDecayPerSecond,
                        current_reduction);
                } else {
                    previous_reduction_[chan] = current_reduction;
                }
                const auto reduction_rect = reduction_rect_[chan].load();
                const auto reduction_height = std::abs(previous_reduction_[chan] / min_db) * bound.getHeight();
                if (previous_reduction_[chan] > 0.f) {
                    reduction_rect_[chan].store({
                        reduction_rect.getX(), .5f * bound.getHeight() - reduction_height,
                        reduction_rect.getWidth(), reduction_height});
                } else {
                    reduction_rect_[chan].store({
                        reduction_rect.getX(), .5f * bound.getHeight(),
                        reduction_rect.getWidth(), reduction_height});
                }
            } else {
                if (current_reduction < previous_reduction) {
                    previous_reduction_[chan] = current_reduction;
                } else {
                    previous_reduction_[chan] = std::min(
                        previous_reduction + static_cast<float>(delta_time) * kReductionDecayPerSecond,
                        current_reduction);
                }
                const auto reduction_rect = reduction_rect_[chan].load();
                reduction_rect_[chan].store({
                    reduction_rect.getX(), 0.f,
                    reduction_rect.getWidth(), previous_reduction_[chan] / min_db * bound.getHeight()});
            }
        }
        // update reduction short-term max display
        auto reduction_max_value = std::max(std::abs(dbs[1][0] - dbs[0][0]),
                                            std::abs(dbs[1][1] - dbs[0][1]));
        reduction_max_value = circular_min_max_.push(reduction_max_value);
        float reduction_max_pos;
        if (is_upwards_) {
            reduction_max_pos = (.5f - std::abs(reduction_max_value / min_db)) * bound.getHeight();
        } else {
            reduction_max_pos = std::abs(reduction_max_value / min_db) * bound.getHeight();
        }
        reduction_max_rect_.setY(reduction_max_pos - reduction_max_rect_.getWidth() * .5f);
        reduction_max_value_.store(reduction_max_value, std::memory_order::relaxed);
        // update pre meter
        for (size_t chan = 0; chan < 2; ++chan) {
            const auto current_pre = dbs[0][chan];
            const auto previous_pre = previous_pre_[chan];
            if (current_pre > previous_pre) {
                previous_pre_[chan] = current_pre;
                pre_decay_mul_[chan] = 1.f;
            } else {
                previous_pre_[chan] = std::max(
                    previous_pre - pre_decay_mul_[chan] * static_cast<float>(delta_time) * kMeterDecayPerSecond,
                    current_pre);
                pre_decay_mul_[chan] = std::min(pre_decay_mul_[chan] + 4.f * static_cast<float>(delta_time), 10.f);
            }
            const auto pre_rect = pre_rect_[chan].load();
            const auto pre_height = (1.f - previous_pre_[chan] / min_db) * bound.getHeight();
            pre_rect_[chan].store({pre_rect.getX(), bound.getHeight() - pre_height,
                                   pre_rect.getWidth(), pre_height});
        }
        // update out peak
        const auto out_peak = std::max(dbs[2][0], dbs[2][1]);
        out_peak_.store(std::max(out_peak, out_peak_.load(std::memory_order::relaxed)),
                        std::memory_order::relaxed);
        // update out meter
        for (size_t chan = 0; chan < 2; ++chan) {
            const auto current_shift = previous_pre_[chan] + previous_reduction_[chan] - dbs[1][chan];
            const auto current_out = std::max(dbs[2][chan], dbs[2][chan] + current_shift);
            const auto pre_rect = out_rect_[chan].load();
            const auto pre_height = (1.f - current_out / min_db) * bound.getHeight();
            out_rect_[chan].store({pre_rect.getX(), bound.getHeight() - pre_height,
                                   pre_rect.getWidth(), pre_height});
        }
    }

    void MeterDisplayPanel::mouseDoubleClick(const juce::MouseEvent&) {
        reduction_peak_.store(0.f, std::memory_order::relaxed);
        out_peak_.store(-240.f, std::memory_order::relaxed);
    }

    std::string MeterDisplayPanel::formatValue(const float value) {
        std::stringstream ss;
        if (std::abs(value) < 10.f) {
            ss << std::fixed << std::setprecision(1) << value;
        } else {
            ss << std::fixed << std::setprecision(0) << value;
        }
        return ss.str();
    }
}
