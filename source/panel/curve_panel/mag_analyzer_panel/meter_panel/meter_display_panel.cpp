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
    }

    MeterDisplayPanel::~MeterDisplayPanel() = default;

    void MeterDisplayPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kReductionColour));
        for (auto& a_bound : reduction_rect_) {
            g.fillRect(a_bound.load());
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
        bound = bound.removeFromTop(base_.getFontSize() * 1.25f);
        g.setColour(base_.getBackgroundColour().withAlpha(.5f));
        g.fillRect(bound);
        g.setFont(base_.getFontSize());

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

        const auto x1 = 0.f;
        const auto x2 = x1 + meter_width + meter_padding * .5f;
        const auto x3 = x2 + meter_width + meter_padding;
        const auto x4 = x3 + meter_width + meter_padding * .5f;

        reduction_rect_[0].store({x1, 0.f, meter_width, 0.f});
        reduction_rect_[1].store({x2, 0.f, meter_width, 0.f});

        pre_rect_[0].store({x3, 0.f, meter_width, 0.f});
        pre_rect_[1].store({x4, 0.f, meter_width, 0.f});

        out_rect_[0].store({x3, 0.f, meter_width, 0.f});
        out_rect_[1].store({x4, 0.f, meter_width, 0.f});
    }

    void MeterDisplayPanel::run(const double next_time_stamp,
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
            fifo.finishRead(consumer_id, num_to_read);

            const auto& dbs{meter_receiver_.getDBs()};

            const auto bound = bound_.load();
            const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
                comp_direction_ref_.load(std::memory_order::relaxed)));
            if (direction == zlp::PCompDirection::kInflate || direction == zlp::PCompDirection::kShape) {
                const auto reduction_peak = std::max(dbs[1][0] - dbs[0][0], dbs[1][1] - dbs[0][1]);
                reduction_peak_.store(std::max(reduction_peak, reduction_peak_.load(std::memory_order::relaxed)),
                                      std::memory_order::relaxed);
            } else {
                const auto reduction_peak = std::min(dbs[1][0] - dbs[0][0], dbs[1][1] - dbs[0][1]);
                reduction_peak_.store(std::min(reduction_peak, reduction_peak_.load(std::memory_order::relaxed)),
                                      std::memory_order::relaxed);
            }
            for (size_t chan = 0; chan < 2; ++chan) {
                const auto current_reduction = dbs[1][chan] - dbs[0][chan];
                const auto previous_reduction = previous_reduction_[chan];
                if (direction == zlp::PCompDirection::kInflate || direction == zlp::PCompDirection::kShape) {
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
            for (size_t chan = 0; chan < 2; ++chan) {
                const auto current_pre = dbs[0][chan];
                const auto previous_pre = previous_pre_[chan];
                if (current_pre > previous_pre) {
                    previous_pre_[chan] = current_pre;
                } else {
                    previous_pre_[chan] = std::max(previous_pre - static_cast<float>(delta_time) * kMeterDecayPerSecond,
                                                   current_pre);
                }
                const auto pre_rect = pre_rect_[chan].load();
                const auto pre_height = (1.f - previous_pre_[chan] / min_db) * bound.getHeight();
                pre_rect_[chan].store({pre_rect.getX(), bound.getHeight() - pre_height,
                                       pre_rect.getWidth(), pre_height});
            }
            const auto out_peak = std::max(dbs[2][0], dbs[2][1]);
            out_peak_.store(std::max(out_peak, out_peak_.load(std::memory_order::relaxed)),
                            std::memory_order::relaxed);
            for (size_t chan = 0; chan < 2; ++chan) {
                const auto current_shift = previous_pre_[chan] + previous_reduction_[chan] - dbs[1][chan];
                const auto current_out = std::max(dbs[2][chan], dbs[2][chan] + current_shift);
                const auto pre_rect = out_rect_[chan].load();
                const auto pre_height = (1.f - current_out / min_db) * bound.getHeight();
                out_rect_[chan].store({pre_rect.getX(), bound.getHeight() - pre_height,
                                       pre_rect.getWidth(), pre_height});
            }
        } else {
            is_first_point_ = false;
            start_time_ = next_time_stamp;
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
