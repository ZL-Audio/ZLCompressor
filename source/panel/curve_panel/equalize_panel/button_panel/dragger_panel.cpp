// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_panel.hpp"

namespace zlpanel {
    DraggerPanel::DraggerPanel(PluginProcessor& processor, zlgui::UIBase& base,
                               size_t band_idx, size_t& selected_band_idx)
        : p_ref_(processor), base_(base),
          band_idx_(band_idx), selected_band_idx_(selected_band_idx),
          filter_type_ref_(*p_ref_.parameters_.getRawParameterValue(
              zlp::PFilterType::kID + std::to_string(band_idx))),
          dragger_(base) {
        dragger_.getButton().setBufferedToImage(true);
        dragger_.setBroughtToFrontOnMouseClick(true);

        lookAndFeelChanged();

        dragger_.setScale(kScale);
        addAndMakeVisible(dragger_);

        dragger_.getButton().onStateChange = [this]() {
            if (dragger_.getButton().getToggleState()) {
                selected_band_idx_ = band_idx_;
            }
        };

        rebuildAttachments();
        setInterceptsMouseClicks(false, true);
    }

    juce::NormalisableRange<float> DraggerPanel::makeFreqRange(const float max_freq) {
        return {
            zlp::kEQMinFreq, max_freq,
            [](const float range_start, const float range_end, const float value_to_remap) {
                return std::exp(value_to_remap * std::log(range_end / range_start)) * range_start;
            },
            [](const float range_start, const float range_end, const float value_to_remap) {
                return std::log(value_to_remap / range_start) / std::log(range_end / range_start);
            },
            [](const float range_start, const float range_end, const float value_to_remap) {
                return std::clamp(value_to_remap, range_start, range_end);
            }
        };
    }

    void DraggerPanel::resized() {
        filter_type_ = filter_type_ref_.load(std::memory_order::relaxed);
        updateDraggerBound();
    }

    void DraggerPanel::repaintCallBackSlow() {
        const auto filter_type = filter_type_ref_.load(std::memory_order::relaxed);
        if (std::abs(filter_type - filter_type_) > 0.01f) {
            filter_type_ = filter_type;
            updateDraggerBound();
        }
    }

    void DraggerPanel::setEQMaxDB(const float db) {
        eq_max_db_ = db;
        rebuildAttachments();
    }

    void DraggerPanel::updateSampleRate(const double sample_rate) {
        sample_rate_ = sample_rate;
        freq_range_ = makeFreqRange(static_cast<float>(zlp::getEQFreqMax(sample_rate)));
        rebuildAttachments();
        updateDraggerBound();
    }

    void DraggerPanel::rebuildAttachments() {
        dragger_attachment_x_.reset();
        dragger_attachment_x_ = std::make_unique<zlgui::attachment::DraggerAttachment<false, true>>(
            dragger_, p_ref_.parameters_,
            zlp::PFreq::kID + std::to_string(band_idx_), freq_range_,
            updater_
        );
        dragger_attachment_y_.reset();
        dragger_attachment_y_ = std::make_unique<zlgui::attachment::DraggerAttachment<false, false>>(
            dragger_, p_ref_.parameters_,
            zlp::PGain::kID + std::to_string(band_idx_), juce::NormalisableRange<float>(-eq_max_db_, eq_max_db_, .01f),
            updater_
        );
    }

    void DraggerPanel::updateDraggerBound() {
        dragger_.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        const auto fft_max = static_cast<float>(zlp::getEQFFTMax(sample_rate_));
        const auto slider_max = static_cast<float>(zlp::getEQFreqMax(sample_rate_));
        const auto width_portion = std::clamp(
            std::log(slider_max / zlp::kEQMinFreq) / std::log(fft_max / zlp::kEQMinFreq), 0.f, 1.f);
        bound.removeFromRight((1.f - width_portion) * bound.getWidth());
        const auto filter_type = static_cast<zldsp::filter::FilterType>(filter_type_);
        switch (filter_type) {
        case zldsp::filter::FilterType::kPeak: {
            dragger_.setXYEnabled(true, true);
            dragger_.setButtonArea(
                bound.withSizeKeepingCentre(
                    bound.getWidth(), bound.getHeight() - 2 * base_.getFontSize()));
            break;
        }
        case zldsp::filter::FilterType::kLowShelf:
        case zldsp::filter::FilterType::kHighShelf:
        case zldsp::filter::FilterType::kTiltShelf:
        case zldsp::filter::FilterType::kFlatTilt: {
            dragger_.setXYEnabled(true, true);
            dragger_.setButtonArea(
                bound.withSizeKeepingCentre(
                    bound.getWidth(), bound.getHeight() * .5f - base_.getFontSize()));
            break;
        }
        case zldsp::filter::FilterType::kNotch:
        case zldsp::filter::FilterType::kLowPass:
        case zldsp::filter::FilterType::kHighPass:
        case zldsp::filter::FilterType::kBandPass:
        case zldsp::filter::FilterType::kAllPass: {
            dragger_.setXYEnabled(true, false);
            dragger_.setButtonArea(bound.withSizeKeepingCentre(
                bound.getWidth(), kScale * base_.getFontSize()));
            break;
        }
        }
    }

    void DraggerPanel::lookAndFeelChanged() {
        dragger_.getLAF().setColour(base_.getColourMap1(band_idx_));
    }
} // zlpanel
