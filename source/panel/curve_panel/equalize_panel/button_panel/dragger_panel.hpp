// Copyright (C) 2025 - zsliu98
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

        zlgui::dragger::Dragger& getDragger() {
            return dragger_;
        }

        void setEQMaxDB(float db);

    private:
        const juce::NormalisableRange<float> kFreqRange{
            10.f, 20000.f,
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return std::exp(valueToRemap * std::log(
                    rangeEnd / rangeStart)) * rangeStart;
            },
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return std::log(valueToRemap / rangeStart) / std::log(
                    rangeEnd / rangeStart);
            },
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return juce::jlimit(
                    rangeStart, rangeEnd, valueToRemap);
            }
        };
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

        void updateDraggerBound();
        void lookAndFeelChanged() override;
    };
} // zlpanel
