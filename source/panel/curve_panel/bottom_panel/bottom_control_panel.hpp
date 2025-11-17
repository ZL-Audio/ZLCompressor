// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "rms_button.hpp"
#include "lufs_button.hpp"

namespace zlpanel {
    class BottomControlPanel final : public juce::Component {
    public:
        explicit BottomControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                    multilingual::TooltipHelper& tooltip_helper);

        void paint(juce::Graphics& g) override;

        void resized() override;

        void repaintCallBackSlow();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;

        std::atomic<float> &side_control_show_ref_, &side_eq_show_ref_;
        bool show_path1_{false};

        juce::Path background_path0_, background_path1_;

        zlgui::combobox::CompactCombobox time_length_box_;
        zlgui::attachment::ComboBoxAttachment<true> time_length_attachment_;

        zlgui::combobox::CompactCombobox mag_type_box_;
        zlgui::attachment::ComboBoxAttachment<true> mag_type_attachment_;

        zlgui::combobox::CompactCombobox min_db_box_;
        zlgui::attachment::ComboBoxAttachment<true> min_db_attachment_;

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label threshold_label_, ratio_label_, attack_label_, release_label_;

        zlgui::combobox::CompactCombobox style_box_;
        zlgui::attachment::ComboBoxAttachment<true> style_attachment_;

        RMSButton rms_button_;

        LUFSButton lufs_button_;
    };
} // zlpanel
