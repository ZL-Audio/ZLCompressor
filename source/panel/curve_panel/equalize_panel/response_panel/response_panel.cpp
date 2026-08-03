// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "response_panel.hpp"

#include <numbers>

namespace zlpanel {
    ResponsePanel::ResponsePanel(PluginProcessor& processor, zlgui::UIBase& base)
        : p_ref_(processor), base_(base),
          eq_max_db_id_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PEQMaxDB::kID)),
          sum_panel_(base) {
        xs_.resize(kNumPoints);
        ws_.resize(kNumPoints);
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            mags_[band].resize(kNumPoints);
            single_panels_[band] = std::make_unique<SinglePanel>(base, band, filters_[band]);
            dummy_component_.addChildComponent(single_panels_[band].get());
            const auto suffix = std::to_string(band);
            for (const auto& id : kBandIDs) {
                const auto parameter_id = id + suffix;
                p_ref_.parameters_.addParameterListener(parameter_id, this);
                parameterChanged(parameter_id,
                                 p_ref_.parameters_.getRawParameterValue(parameter_id)->load(
                                     std::memory_order::relaxed));
            }
        }
        addChildComponent(dummy_component_);
        addAndMakeVisible(sum_panel_);

        setInterceptsMouseClicks(false, false);
    }

    ResponsePanel::~ResponsePanel() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto suffix = std::to_string(band);
            for (const auto& id : kBandIDs) {
                p_ref_.parameters_.removeParameterListener(id + suffix, this);
            }
        }
    }

    void ResponsePanel::resized() {
        {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2.f * base_.getFontSize());
            bound_.store(bound);
        }
        {
            const auto bound = getLocalBounds();
            dummy_component_.setBounds(bound);
            for (auto& panel : single_panels_) {
                panel->setBounds(bound);
            }
            sum_panel_.setBounds(bound);
        }
    }

    void ResponsePanel::run(juce::Thread& thread) {
        updateCurveParameters();
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (!to_update_curve_flags_[band]) {
                continue;
            }
            to_update_curve_flags_[band] = false;
            if (c_filter_status_[band] == zlp::EqualizeController::kOff) {
                continue;
            }

            auto paras = empty_filters_[band].getParas();
            paras.freq = std::min(paras.freq, slider_max_);
            filters_[band].forceUpdate(paras);
            filters_[band].updateMagnitudeSquare(std::span<const float>(ws_.data(), ws_.size()), mags_[band]);
            zldsp::vector::sqr_mag_to_db(mags_[band].data(), mags_[band].size());
            single_panels_[band]->run(std::span<const float>(xs_.data(), xs_.size()),
                                      std::span<const float>(mags_[band].data(), mags_[band].size()),
                                      c_bound_, eq_max_db_, c_sample_rate_, fft_max_);
            to_update_sum_ = to_update_sum_ || c_filter_status_[band] == zlp::EqualizeController::kOn;
            if (thread.threadShouldExit()) {
                return;
            }
        }

        if (to_update_sum_) {
            sum_panel_.run(std::span<const float>(xs_.data(), xs_.size()), mags_, c_filter_status_,
                           c_bound_, eq_max_db_);
            to_update_sum_ = false;
        }
    }

    void ResponsePanel::updateSampleRate(const double sample_rate) {
        sample_rate_.store(sample_rate, std::memory_order::relaxed);
    }

    void ResponsePanel::setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum>& status) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            single_panels_[band]->setVisible(status[band] != zlp::EqualizeController::kOff);
        }
    }

    void ResponsePanel::updateBand(const size_t band) {
        for (auto& panel : single_panels_) {
            panel->setCurveThicknessScale(.5f);
        }
        if (band < zlp::kBandNum) {
            single_panels_[band]->setCurveThicknessScale(1.0f);
            single_panels_[band]->toFront(false);
        }
    }

    void ResponsePanel::parameterChanged(const juce::String& parameter_ID, const float new_value) {
        const auto band = static_cast<size_t>(parameter_ID.getTrailingIntValue());
        if (parameter_ID.startsWith(zlp::PFilterStatus::kID)) {
            filter_status_[band].store(static_cast<zlp::EqualizeController::FilterStatus>(std::round(new_value)),
                                       std::memory_order::relaxed);
            to_update_filter_status_.signal();
        } else if (parameter_ID.startsWith(zlp::PFilterType::kID)) {
            empty_filters_[band].setFilterType(static_cast<zldsp::filter::FilterType>(std::round(new_value)));
            empty_update_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::POrder::kID)) {
            const auto order_idx = static_cast<size_t>(std::round(new_value));
            empty_filters_[band].setOrder(zlp::POrder::kOrderArray[std::min(order_idx,
                                                                              zlp::POrder::kOrderArray.size() - 1)]);
            empty_update_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::PFreq::kID)) {
            empty_filters_[band].setFreq(new_value);
            empty_update_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::PGain::kID)) {
            empty_filters_[band].setGain(new_value);
            empty_update_flags_[band].signal();
        } else if (parameter_ID.startsWith(zlp::PQ::kID)) {
            empty_filters_[band].setQ(new_value);
            empty_update_flags_[band].signal();
        }
    }

    void ResponsePanel::updateCurveParameters() {
        bool force_curve_update = false;

        if (const auto sample_rate = sample_rate_.load(std::memory_order::relaxed);
            std::abs(sample_rate - c_sample_rate_) > 1.0) {
            c_sample_rate_ = sample_rate;
            slider_max_ = zlp::getEQFreqMax(sample_rate);
            fft_max_ = zlp::getEQFFTMax(sample_rate);
            for (auto& filter : filters_) {
                filter.prepare(sample_rate);
            }
            const auto log_range = std::log(fft_max_ / static_cast<double>(zlp::kEQMinFreq));
            const auto freq_scale = 2.0 * std::numbers::pi / sample_rate;
            for (size_t i = 0; i < kNumPoints; ++i) {
                const auto portion = static_cast<double>(i) / static_cast<double>(kNumPoints - 1);
                ws_[i] = static_cast<float>(zlp::kEQMinFreq * std::exp(portion * log_range) * freq_scale);
            }
            force_curve_update = true;
        }

        const auto new_bound = bound_.load();
        if (std::abs(new_bound.getWidth() - c_bound_.getWidth()) > 1e-3f
            || std::abs(new_bound.getHeight() - c_bound_.getHeight()) > 1e-3f) {
            c_bound_ = new_bound;
            if (c_bound_.getWidth() > 0.f && c_bound_.getHeight() > 0.f) {
                const auto delta_x = c_bound_.getWidth() / static_cast<float>(kNumPoints - 1);
                for (size_t i = 0; i < kNumPoints; ++i) {
                    xs_[i] = c_bound_.getX() + static_cast<float>(i) * delta_x;
                }
                force_curve_update = true;
            }
        }

        const auto eq_max_db_id = eq_max_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(eq_max_db_id - eq_max_db_id_) > 1e-3f) {
            eq_max_db_id_ = std::round(eq_max_db_id);
            eq_max_db_ = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(eq_max_db_id_)];
            force_curve_update = true;
        }

        if (to_update_filter_status_.check()) {
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                const auto filter_status = filter_status_[band].load(std::memory_order::relaxed);
                if (c_filter_status_[band] != filter_status) {
                    c_filter_status_[band] = filter_status;
                    to_update_curve_flags_[band] = true;
                    to_update_sum_ = true;
                }
            }
        }

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto empty_updated = empty_update_flags_[band].check();
            to_update_curve_flags_[band] = to_update_curve_flags_[band]
                || force_curve_update || empty_updated;
        }
        to_update_sum_ = to_update_sum_ || force_curve_update;
    }
}
