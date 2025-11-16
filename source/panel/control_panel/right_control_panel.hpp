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
#include "../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class RightControlPanel final : public juce::Component,
                                    private juce::ValueTree::Listener {
    public:
        explicit RightControlPanel(PluginProcessor &p, zlgui::UIBase &base,
                                   multilingual::TooltipHelper &tooltip_helper);

        ~RightControlPanel() override;

        void paint(juce::Graphics &g) override;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label gain_label_;
        zlgui::slider::TwoValueRotarySlider<false, false, false> gain_slider_;
        zlgui::attachment::SliderAttachment<true> gain_attachment_;

        juce::Label wet_label_;
        zlgui::slider::TwoValueRotarySlider<false, false, false> wet_slider_;
        zlgui::attachment::SliderAttachment<true> wet_attachment_;

        void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &identifier) override;
    };
} // zlpanel
