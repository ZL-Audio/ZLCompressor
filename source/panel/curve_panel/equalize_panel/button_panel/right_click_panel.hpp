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
    class RightClickPanel final : public juce::Component {
    public:
        explicit RightClickPanel(PluginProcessor& processor, zlgui::UIBase& base, size_t& selected_band_idx);

        void paint(juce::Graphics& g) override;

        void updateCopyVisibility(bool show_copy = true);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        size_t& selected_band_idx_;
        zlgui::button::ClickTextButton invert_gain_button_, copy_button_, paste_button_;

        juce::SelectedItemSet<size_t>& items_set_;

        constexpr static std::array kIDs{
            zlp::PFilterStatus::kID,
            zlp::PFilterType::kID,
            zlp::POrder::kID,
            zlp::PFreq::kID,
            zlp::PGain::kID,
            zlp::PQ::kID
        };

        void resized() override;

        void invertGain();

        void copy();

        void paste();
    };
} // zlpanel
