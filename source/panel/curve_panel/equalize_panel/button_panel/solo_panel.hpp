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
#include "dragger_panel.hpp"

namespace zlpanel {
    class SoloPanel final : public juce::Component {
    public:
        explicit SoloPanel(PluginProcessor& processor, zlgui::UIBase& base,
                           size_t& selected_band_idx,
                           std::array<std::unique_ptr<DraggerPanel>, zlp::kBandNum>& dragger_panels);

        void paint(juce::Graphics& g) override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        size_t& selected_band_idx_;
        std::array<std::unique_ptr<DraggerPanel>, zlp::kBandNum>& dragger_panels_;
    };
} // zlpanel
