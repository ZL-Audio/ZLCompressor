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
    class TopControlPanel final : public juce::Component {
    public:
        explicit TopControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        [[maybe_unused]] PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        zlgui::label::NameLookAndFeel label_laf_;

        const std::unique_ptr<juce::Drawable> on_drawable_;
        zlgui::button::CompactButton on_button_;
        zlgui::attachment::ButtonAttachment<true> on_attachment_;

        const std::unique_ptr<juce::Drawable> delta_drawable_;
        zlgui::button::CompactButton delta_button_;
        zlgui::attachment::ButtonAttachment<true> delta_attachment_;

        double old_lookahead_value_{0.0};
        juce::Label lookahead_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> lookahead_slider_;
        zlgui::attachment::SliderAttachment<true> lookahead_attachment_;

        int old_oversample_id_{0};
        juce::Label oversample_label_;
        zlgui::combobox::CompactCombobox oversample_box_;
        zlgui::attachment::ComboBoxAttachment<true> oversample_attachment_;

        double old_clipper_value_{0.0};
        juce::Label clipper_label_;
        zlgui::slider::CompactLinearSlider<false, false, false> clipper_slider_;
        zlgui::attachment::SliderAttachment<true> clipper_attachment_;

        void setLookaheadAlpha(const float alpha);

        void setOversampleAlpha(const float alpha);

        void setClipperAlpha(const float alpha);
    };
}
