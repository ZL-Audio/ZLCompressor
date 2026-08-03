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
#include "../../../../gui/dragger/dragger_attachment.hpp"
#include "../../../helper/helper.hpp"

namespace zlpanel {
    class DraggerPanel final : public juce::Component {
    public:
        explicit DraggerPanel(PluginProcessor& processor, zlgui::UIBase& base,
                              size_t band_idx, size_t& selected_band_idx);

        void resized() override;

        void repaintCallBackSlow();

        void updateSampleRate(double sample_rate);

        zlgui::dragger::Dragger& getDragger() {
            return dragger_;
        }

        void setEQMaxDB(float db);

    private:
        static constexpr float kScale = 1.f;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        const size_t band_idx_;
        size_t& selected_band_idx_;
        std::atomic<float>& filter_type_ref_;

        float filter_type_{-1.f};

        zlgui::attachment::ComponentUpdater updater_;
        zlgui::dragger::Dragger dragger_;
        std::unique_ptr<zlgui::attachment::DraggerAttachment<false, true>> dragger_attachment_x_;
        std::unique_ptr<zlgui::attachment::DraggerAttachment<false, false>> dragger_attachment_y_;
        juce::NormalisableRange<float> freq_range_{makeFreqRange(static_cast<float>(zlp::getEQFreqMax(48000.0)))};
        float eq_max_db_{30.f};
        double sample_rate_{48000.0};

        static juce::NormalisableRange<float> makeFreqRange(float max_freq);
        void rebuildAttachments();
        void updateDraggerBound();
        void lookAndFeelChanged() override;
    };
} // zlpanel
