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
#include "../helper/helper.hpp"

namespace zlpanel {
    class RightControlPanel : public juce::Component {
    public:
        explicit RightControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        zlgui::slider::CompactLinearSlider<true, true, true> range_slider_;
        zlgui::attachment::SliderAttachment<true> range_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> hold_slider_;
        zlgui::attachment::SliderAttachment<true> hold_attachment_;

        juce::Label gain_label_;
        zlgui::slider::TwoValueRotarySlider<true, false, false> gain_slider_;
        zlgui::attachment::SliderAttachment<true> gain_attachment_;

        juce::Label wet_label_;
        zlgui::slider::TwoValueRotarySlider<true, false, false> wet_slider_;
        zlgui::attachment::SliderAttachment<true> wet_attachment_;

        zlgui::label::NameLookAndFeel label_laf_;
    };
} // zlpanel
