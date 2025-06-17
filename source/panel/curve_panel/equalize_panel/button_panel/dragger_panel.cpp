// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "dragger_panel.hpp"

namespace zlpanel {
    DraggerPanel::DraggerPanel(PluginProcessor &processor, zlgui::UIBase &base,
                               size_t band_idx, size_t &selected_band_idx)
        : p_ref_(processor), base_(base),
          band_idx_(band_idx), selected_band_idx_(selected_band_idx),
          filter_type_ref_(*p_ref_.parameters_.getRawParameterValue(
              zlp::PFilterType::kID + std::to_string(band_idx))),
          dragger_(base),
          dragger_attachment_(dragger_, p_ref_.parameters_,
                              zlp::PFreq::kID + std::to_string(band_idx), kFreqRange,
                              zlp::PGain::kID + std::to_string(band_idx), zlp::PGain::kRange,
                              updater_) {
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

        setInterceptsMouseClicks(false, true);
    }

    void DraggerPanel::resized() {
        updateDraggerBound();
    }

    void DraggerPanel::repaintCallBackSlow() {
        const auto filter_type = filter_type_ref_.load(std::memory_order::relaxed);
        if (std::abs(filter_type - filter_type_) > 0.01f) {
            filter_type_ = filter_type;
            updateDraggerBound();
        }
    }

    void DraggerPanel::updateDraggerBound() {
        dragger_.setBounds(getLocalBounds());
        auto bound = getLocalBounds().toFloat();
        bound.removeFromRight((1 - 0.98761596f) * bound.getWidth());
        const auto filter_type = static_cast<zldsp::filter::FilterType>(filter_type_);
        switch (filter_type) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                dragger_.setXYEnabled(true, true);
                dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(), bound.getHeight() - 2 * base_.getFontSize()));
                break;
            }
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kTiltShelf: {
                dragger_.setXYEnabled(true, true);
                dragger_.setButtonArea(
                    bound.withSizeKeepingCentre(
                        bound.getWidth(), bound.getHeight() * .5f - base_.getFontSize()));
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                dragger_.setXYEnabled(true, false);
                dragger_.setButtonArea(bound.withSizeKeepingCentre(
                    bound.getWidth(), kScale * base_.getFontSize()));
                break;
            }
        }
    }

    void DraggerPanel::lookAndFeelChanged() {
        dragger_.getLAF().setColour(base_.getColorMap1(band_idx_));
    }
} // zlpanel
