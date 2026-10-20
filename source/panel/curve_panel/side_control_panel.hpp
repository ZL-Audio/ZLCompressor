// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../state/ui_state.hpp"

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"
#include "../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class SideControlPanel final : public juce::Component {
    public:
        explicit SideControlPanel(PluginProcessor& p, zlgui::UIState& base,
                                  multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIState& base_;
        zlgui::attachment::ComponentUpdater updater_;

        std::atomic<float> &stereo_mode_ref_, &stereo_swap_ref_;
        bool stereo_mode_flag_{false}, stereo_swap_flag_{false};

        std::atomic<float>& panel_show_ref_;

        zlgui::combobox::ClickCombobox stereo_mode_box_;
        zlgui::attachment::ComboBoxAttachment<true> stereo_mode_attachment_;

        zlgui::button::ClickButton stereo_swap_button_;
        zlgui::attachment::ButtonAttachment<true> stereo_swap_attachment_;

        zlgui::slider::CompactLinearSlider<true, true, true> stereo_link_slider_;
        zlgui::attachment::SliderAttachment<true> stereo_link_attachment_;

        zlgui::slider::CompactLinearSlider<false, false, false> stereo_wet1_slider_;
        zlgui::attachment::SliderAttachment<true> stereo_wet1_attachment_;

        zlgui::slider::CompactLinearSlider<false, false, false> stereo_wet2_slider_;
        zlgui::attachment::SliderAttachment<true> stereo_wet2_attachment_;

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label side_in1_label_, side_in2_label_, side_out1_label_, side_out2_label_;

        zlgui::slider::CompactLinearSlider<true, true, true> side_gain_slider_;
        zlgui::attachment::SliderAttachment<true> side_gain_attachment_;

        zlgui::button::ClickButton ext_side_button_;
        zlgui::attachment::ButtonAttachment<true> ext_side_attachment_;

        zlgui::button::ClickButton side_out_button_;
        zlgui::attachment::ButtonAttachment<true> side_out_attachment_;

        zlgui::button::ClickButton eq_bypass_button_;
        zlgui::attachment::ButtonAttachment<true> eq_bypass_attachment_;

        void updateLabels();
    };
}
