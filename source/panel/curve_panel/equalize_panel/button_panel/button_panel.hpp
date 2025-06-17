// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "dragger_panel.hpp"
#include "popup_panel.hpp"

namespace zlpanel {
    class ButtonPanel final : public juce::Component {
    public:
        explicit ButtonPanel(PluginProcessor &processor, zlgui::UIBase &base,
                             size_t &selected_band_idx);

        ~ButtonPanel() override = default;

        void resized() override;

        void repaintCallBackSlow();

        DraggerPanel &getDraggerPanel(const size_t band) {
            return *dragger_panels_[band];
        }

        zlgui::dragger::Dragger &getDragger(const size_t band) {
            return dragger_panels_[band]->getDragger();
        }

        PopupPanel &getPopupPanel() {
            return popup_panel_;
        }

        void setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &status);

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;

        PopupPanel popup_panel_;
        std::array<std::unique_ptr<DraggerPanel>, zlp::kBandNum> dragger_panels_;
    };
} // zlpanel
