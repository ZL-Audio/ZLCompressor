// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include "BinaryData.h"
#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    class LeftControlPanel final : public juce::Component {
    public:
        explicit LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        constexpr static int kButtonNum = 5;

        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        const std::unique_ptr<juce::Drawable> side_control_show_drawable_;
        zlgui::button::CompactButton side_control_show_button_;
        zlgui::attachment::ButtonAttachment<false> side_control_show_attachment_;

        const std::unique_ptr<juce::Drawable> equalize_show_drawable_;
        zlgui::button::CompactButton equalize_show_button_;
        zlgui::attachment::ButtonAttachment<false> equalize_show_attachment_;
    };
} // zlpanel
