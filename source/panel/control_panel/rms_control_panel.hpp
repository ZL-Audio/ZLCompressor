// Copyright (C) 2026 - zsliu98
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
    class RMSControlPanel final : public juce::Component {
    public:
        explicit RMSControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper);

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label rms_length_label_, rms_speed_label_, rms_mix_label_;

        zlgui::slider::CompactLinearSlider<false, false, false> rms_length_slider_;
        zlgui::attachment::SliderAttachment<true> rms_length_attachment_;

        zlgui::slider::CompactLinearSlider<false, false, false> rms_speed_slider_;
        zlgui::attachment::SliderAttachment<true> rms_speed_attachment_;

        zlgui::slider::CompactLinearSlider<false, false, false> rms_mix_slider_;
        zlgui::attachment::SliderAttachment<true> rms_mix_attachment_;
    };
}
