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
    class SideControlPanel final : public juce::Component {
    public:
        explicit SideControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        [[maybe_unused]] PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        std::atomic<float> &stereo_mode_ref_, &stereo_swap_ref_;
        bool stereo_mode_flag_{false}, stereo_swap_flag_{false};

        std::atomic<float> &panel_show_ref_;
        double previous_time_stamp{0.0};

        const std::unique_ptr<juce::Drawable> stereo_mid_side_drawable_;
        const std::unique_ptr<juce::Drawable> stereo_left_right_drawable_;
        zlgui::button::ClickButton stereo_mode_button_;
        zlgui::attachment::ButtonAttachment<true> stereo_mode_attachment_;

        const std::unique_ptr<juce::Drawable> stereo_swap_drawable_;
        zlgui::button::CompactButton stereo_swap_button_;
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

        const std::unique_ptr<juce::Drawable> ext_side_drawable_;
        zlgui::button::CompactButton ext_side_button_;
        zlgui::attachment::ButtonAttachment<true> ext_side_attachment_;

        const std::unique_ptr<juce::Drawable> side_out_drawable_;
        zlgui::button::CompactButton side_out_button_;
        zlgui::attachment::ButtonAttachment<true> side_out_attachment_;

        void updateLabels();
    };
} // zlpanel
