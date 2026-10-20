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

#include "BinaryData.h"
#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"
#include "../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class LeftControlPanel final : public juce::Component {
    public:
        explicit LeftControlPanel(PluginProcessor& p, zlgui::UIState& base,
                                  multilingual::TooltipHelper& tooltip_helper);

        void resized() override;

        void repaintCallBackSlow();

    private:
        constexpr static int kButtonNum = 6;

        zlgui::UIState& base_;
        zlgui::attachment::ComponentUpdater updater_;

        std::atomic<float>& pre_curve_display_ref_;
        std::atomic<float>& post_curve_display_ref_;
        std::atomic<float>& delta_curve_display_ref_;
        std::atomic<float>& side_curve_display_ref_;

        zlgui::button::ClickButton side_control_show_button_;
        zlgui::attachment::ButtonAttachment<true> side_control_show_attachment_;

        zlgui::button::ClickButton equalize_show_button_;
        zlgui::attachment::ButtonAttachment<true> equalize_show_attachment_;

        zlgui::button::ClickButton computer_show_button_;
        zlgui::attachment::ButtonAttachment<true> computer_show_attachment_;

        zlgui::button::ClickButton rms_show_button_;
        zlgui::attachment::ButtonAttachment<true> rms_show_attachment_;

        zlgui::button::ClickButton meter_show_button_;
        zlgui::attachment::ButtonAttachment<true> meter_show_attachment_;

        zlgui::button::ClickButton analyzer_setting_show_button_;
    };
}
