// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "BinaryData.h"
#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    class BottomControlPanel final : public juce::Component {
    public:
        explicit BottomControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        class Background final : public juce::Component {
        public:
            explicit Background(zlgui::UIBase &base);

            void paint(juce::Graphics &g) override;

            void setShowButtons(const bool f) {
                show_buttons_ = f;
            }

        private:
            zlgui::UIBase &base_;

            bool show_buttons_{false};
        };

        class Buttons final : public juce::Component {
        public:
            explicit Buttons(PluginProcessor &p, zlgui::UIBase &base);

            void repaintCallBack();

            void resized() override;

        private:
            zlgui::UIBase &base_;
            zlgui::attachment::ComponentUpdater updater_;

            zlgui::CompactButton ext_side_button_;
            zlgui::attachment::ButtonAttachment<true> ext_side_attachment_;
            const std::unique_ptr<juce::Drawable> ext_side_drawable_;

            zlgui::CompactButton side_out_button_;
            zlgui::attachment::ButtonAttachment<true> side_out_attachment_;
            const std::unique_ptr<juce::Drawable> side_out_drawable_;
        };

        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        Background background_;
        Buttons buttons_;

        zlgui::label::NameLookAndFeel label_laf_;

        juce::Label threshold_label_, ratio_label_, attack_label_, release_label_;

        zlgui::combobox::CompactCombobox style_box_;
        zlgui::attachment::ComboBoxAttachment<true> style_attachment_;

        [[maybe_unused]] bool show_buttons_{false};
        double previous_time_stamp{0.0};
    };
} // zlpanel
