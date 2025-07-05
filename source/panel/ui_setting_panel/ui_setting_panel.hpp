// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"

namespace zlpanel {
    class UISettingPanel final : public juce::Component {
    public:
        explicit UISettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~UISettingPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void loadSetting();

        void mouseDown(const juce::MouseEvent &event) override;

        void visibilityChanged() override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        juce::Viewport view_port_;

        zlgui::label::NameLookAndFeel panel_name_laf_;
        juce::Label colour_label_, control_label_, other_label_;

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label version_label_;

        enum PanelIdx {
            kColourP,
            kControlP,
            kOtherP
        };

        PanelIdx panel_idx_ = kColourP;
    };
} // zlpanel
