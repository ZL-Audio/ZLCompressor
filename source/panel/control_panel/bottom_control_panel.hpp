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
#include "rms_control_panel.hpp"

namespace zlpanel {
    class BottomControlPanel final : public juce::Component {
    public:
        explicit BottomControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                    const multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void repaintCallBackSlow();

        int getIdealHeight() const;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        RMSControlPanel rms_control_panel_;

        std::atomic<float>& rms_on_ref_;

        std::atomic<float>& comp_direction_ref_;
        zlp::PCompDirection::Direction c_comp_direction_{zlp::PCompDirection::kCompress};

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label threshold_label_, ratio_label_, attack_label_, release_label_;

        zlgui::combobox::CompactCombobox style_box_;
        zlgui::attachment::ComboBoxAttachment<true> style_attachment_;

        const std::unique_ptr<juce::Drawable> rms_drawable_;
        zlgui::button::ClickButton rms_button_;
        zlgui::attachment::ButtonAttachment<true> rms_attachment_;
    };
}
