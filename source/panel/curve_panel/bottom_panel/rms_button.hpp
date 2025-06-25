// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class RMSButton final : public juce::Component {
    public:
        explicit RMSButton(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        const std::unique_ptr<juce::Drawable> rms_drawable_;
        zlgui::button::ClickButton rms_button_;
        zlgui::attachment::ButtonAttachment<true> rms_attachment_;

        const std::unique_ptr<juce::Drawable> rms_open_drawable_;
        const std::unique_ptr<juce::Drawable> rms_close_drawable_;
        zlgui::button::ClickButton rms_open_close_button_;
        zlgui::attachment::ButtonAttachment<true> rms_open_close_attachment_;
    };
} // zlpanel
