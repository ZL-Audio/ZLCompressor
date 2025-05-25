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
    class LeftControlPanel final : public juce::Component {
    public:
        explicit LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;
        zlgui::label::NameLookAndFeel label_laf_;

        zlgui::slider::CompactLinearSlider<true, true, true> knee_slider_;
        zlgui::attachment::SliderAttachment<true> knee_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> curve_slider_;
        zlgui::attachment::SliderAttachment<true> curve_attachment_;

        juce::Label th_label_;
        zlgui::slider::TwoValueRotarySlider<true, false, false> th_slider_;
        zlgui::attachment::SliderAttachment<true> th_attachment_;

        juce::Label ratio_label_;
        zlgui::slider::TwoValueRotarySlider<true, false, false> ratio_slider_;
        zlgui::attachment::SliderAttachment<true> ratio_attachment_;
    };
} // zlpanel
