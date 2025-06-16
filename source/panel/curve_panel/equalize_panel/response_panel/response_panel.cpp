// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "response_panel.hpp"

namespace zlpanel {
    ResponsePanel::ResponsePanel(PluginProcessor &processor, zlgui::UIBase &base)
        : sum_panel_(processor, base) {
        for (auto &f: filters_) {
            f.prepare(48000.0);
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            single_panels_[band] = std::make_unique<SinglePanel>(processor, base, band, filters_[band]);
            dummy_component_.addChildComponent(single_panels_[band].get());
        }
        addChildComponent(dummy_component_);
        addAndMakeVisible(sum_panel_);

        setInterceptsMouseClicks(false, false);
    }

    ResponsePanel::~ResponsePanel() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dummy_component_.removeAllChildren();
        }
    }

    void ResponsePanel::resized() {
        const auto bound = getLocalBounds();
        bound_.store(bound.toFloat());
        dummy_component_.setBounds(bound);
        for (auto &panel: single_panels_) {
            panel->setBounds(bound);
        }
        sum_panel_.setBounds(bound);
    }

    void ResponsePanel::run(std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &filter_status,
                            bool to_update_sum) {
        const auto new_bound = bound_.load();
        bool to_update_single = false;
        if (std::abs(new_bound.getWidth() - c_bound_.getWidth()) > 1e-3f
            || std::abs(new_bound.getHeight() - c_bound_.getHeight()) > 1e-3f) {
            c_bound_ = new_bound;
            to_update_single = true;

            float x0 = c_bound_.getX();
            const float delta_x = c_bound_.getWidth() / static_cast<float>(xs_.size() - 1);
            for (size_t idx = 0; idx < xs_.size(); ++idx) {
                xs_[idx] = x0;
                x0 += delta_x;
            }
        }
        to_update_sum = to_update_sum || to_update_single;
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto f = single_panels_[band]->run(std::span(xs_), std::span(yss_[band]),
                                                     c_bound_, to_update_single);
            to_update_sum = to_update_sum || f;
        }

        if (to_update_sum) {
            sum_panel_.run(xs_, yss_, filter_status, c_bound_);
        }
    }

    void ResponsePanel::setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &status) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            single_panels_[band]->setVisible(status[band] != zlp::EqualizeController::kOff);
        }
    }
} // zlpanel
