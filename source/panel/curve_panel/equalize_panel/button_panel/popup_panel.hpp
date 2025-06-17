// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class PopupPanel final : public juce::Component {
    public:
        explicit PopupPanel(PluginProcessor &processor, zlgui::UIBase &base, size_t &selected_band_idx);

        void paint(juce::Graphics &g) override;

        void resized() override;

        int getIdealHeight() const;

        int getIdealWidth() const;

        void updateBand();

        void repaintCallBackSlow();

    private:
        [[maybe_unused]] PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;
        size_t &selected_band_idx_;
        size_t band_{zlp::kBandNum};
        std::atomic<float> *bypass_ref_{nullptr};

        const std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::ClickButton bypass_button_;

        const std::unique_ptr<juce::Drawable> close_drawable_;
        zlgui::button::ClickButton close_button_;

        zlgui::combobox::CompactCombobox ftype_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true> > ftype_attachment_;

        zlgui::combobox::CompactCombobox slope_box_;
        std::unique_ptr<zlgui::attachment::ComboBoxAttachment<true> > slope_attachment_;
    };
} // zlpanel
