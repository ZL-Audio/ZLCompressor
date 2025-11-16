// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "solo_panel.hpp"

namespace zlpanel {
    SoloPanel::SoloPanel(PluginProcessor &processor, zlgui::UIBase &base, size_t &selected_band_idx,
                         std::array<std::unique_ptr<DraggerPanel>, zlp::kBandNum> &dragger_panels)
        : p_ref_(processor), base_(base), selected_band_idx_(selected_band_idx),
          dragger_panels_(dragger_panels) {
        setInterceptsMouseClicks(false, false);
    }

    void SoloPanel::paint(juce::Graphics &g) {
        if (selected_band_idx_ >= zlp::kBandNum) {
            return;
        }
        auto &filter{p_ref_.getEqualizeController().getFilter(selected_band_idx_)};
        g.setColour(base_.getBackgroundColour().withAlpha(.75f));
        switch (filter.getFilterType<true>()) {
            case zldsp::filter::FilterType::kLowShelf:
            case zldsp::filter::FilterType::kHighPass: {
                const auto x = dragger_panels_[selected_band_idx_]->getDragger().getButtonPos().x;

                auto bound = getLocalBounds().toFloat();
                bound.removeFromLeft(x);
                g.fillRect(bound);
                break;
            }
            case zldsp::filter::FilterType::kHighShelf:
            case zldsp::filter::FilterType::kLowPass: {
                const auto x = dragger_panels_[selected_band_idx_]->getDragger().getButtonPos().x;

                auto bound = getLocalBounds().toFloat();
                bound = bound.removeFromLeft(x);
                g.fillRect(bound);
                break;
            }
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf:
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kBandPass: {
                const auto x = dragger_panels_[selected_band_idx_]->getDragger().getButtonPos().x;
                const auto solo_q = p_ref_.getEqualizeController().getFilter(selected_band_idx_).getQ();
                const auto bw = static_cast<float>(std::asinh(0.5f / solo_q) / std::log(2200.f));

                auto bound = getLocalBounds().toFloat();
                const auto bound_width = bound.getWidth();
                const auto left_width = x - bw * bound_width;
                const auto right_width = bound_width - x - bw * bound_width;
                const auto left_area = bound.removeFromLeft(left_width);
                const auto right_area = bound.removeFromRight(right_width);
                g.fillRect(left_area);
                g.fillRect(right_area);
                break;
            }
            case zldsp::filter::FilterType::kTiltShelf: {
            }
        }
    }
} // zlpanel
