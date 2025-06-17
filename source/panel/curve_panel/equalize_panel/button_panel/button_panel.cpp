// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "button_panel.hpp"

namespace zlpanel {
    ButtonPanel::ButtonPanel(PluginProcessor &processor, zlgui::UIBase &base,
                             size_t &selected_band_idx)
        : p_ref_(processor), base_(base),
          popup_panel_(processor, base) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band] = std::make_unique<DraggerPanel>(
                p_ref_, base_, band, selected_band_idx);
            addChildComponent(*dragger_panels_[band]);
        }
        addChildComponent(popup_panel_);
    }

    void ButtonPanel::resized() {
        const auto bound = getLocalBounds();
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band]->setBounds(bound);
        }
        popup_panel_.setBounds({
            bound.getX(), bound.getY(),
            popup_panel_.getIdealWidth(), popup_panel_.getIdealHeight()
        });
    }

    void ButtonPanel::repaintCallBackSlow() {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band]->repaintCallBackSlow();
        }
        popup_panel_.repaintCallBackSlow();
    }

    void ButtonPanel::setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &status) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto f = status[band] != zlp::EqualizeController::FilterStatus::kOff;
            dragger_panels_[band]->setVisible(f);
            if (f) {
                dragger_panels_[band]->getDragger().getButton().repaint();
            }
        }
    }
} // zlpanel
