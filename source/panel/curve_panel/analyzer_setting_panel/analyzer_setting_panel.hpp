// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "BinaryData.h"

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"
#include "../../background/panel_background.hpp"

namespace zlpanel {
    class AnalyzerSettingPanel final : public juce::Component,
                                       private juce::ValueTree::Listener {
    public:
        explicit AnalyzerSettingPanel(PluginProcessor& p, zlgui::UIBase& base,
                                      multilingual::TooltipHelper& tooltip_helper);

        ~AnalyzerSettingPanel() override;

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        PanelBackground control_background_;

        zlgui::combobox::CompactCombobox mag_type_box_;
        zlgui::attachment::ComboBoxAttachment<true> mag_type_attachment_;

        zlgui::combobox::CompactCombobox mag_stereo_box_;
        zlgui::attachment::ComboBoxAttachment<true> mag_stereo_attachment_;

        zlgui::combobox::CompactCombobox move_type_box_;
        zlgui::attachment::ComboBoxAttachment<true> move_type_attachment_;

        zlgui::button::ClickTextButton pre_button_;
        zlgui::attachment::ButtonAttachment<true> pre_button_attachment_;

        zlgui::button::ClickTextButton post_button_;
        zlgui::attachment::ButtonAttachment<true> post_button_attachment_;

        zlgui::button::ClickTextButton delta_button_;
        zlgui::attachment::ButtonAttachment<true> delta_button_attachment_;

        zlgui::button::ClickTextButton side_button_;
        zlgui::attachment::ButtonAttachment<true> side_button_attachment_;

        zlgui::combobox::CompactCombobox time_length_box_;
        zlgui::attachment::ComboBoxAttachment<true> time_length_attachment_;

        zlgui::combobox::CompactCombobox max_db_box_;
        zlgui::attachment::ComboBoxAttachment<true> max_db_attachment_;

        zlgui::combobox::CompactCombobox min_db_box_;
        zlgui::attachment::ComboBoxAttachment<true> min_db_attachment_;
        int c_max_db_idx_{-1};

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label delimiter_label_;
        juce::Label db_label_;

        void updateMinDBChoices(int max_db_idx);

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
