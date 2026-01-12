// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../../PluginProcessor.hpp"
#include "../../../../gui/gui.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class FilterParaPanel final : public juce::Component {
    public:
        explicit FilterParaPanel(PluginProcessor& processor, zlgui::UIBase& base, size_t& selected_band_idx);

        void resized() override;

        int getIdealHeight() const;

        int getIdealWidth() const;

        void updateBand();

        void repaintCallBackSlow();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        zlgui::attachment::ComponentUpdater updater_;
        size_t& selected_band_idx_;

        zlgui::label::NameLookAndFeel label_laf_;
        juce::Label freq_label_, gain_label_;

        std::atomic<float>* ftype_ref_{nullptr};
        float c_ftype_{-1.f};

        zlgui::slider::CompactLinearSlider<false, false, false> freq_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> freq_attachment_;

        zlgui::slider::CompactLinearSlider<false, false, false> gain_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true>> gain_attachment_;
    };
} // zlpanel
