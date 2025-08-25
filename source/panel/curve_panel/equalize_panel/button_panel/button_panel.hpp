// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "dragger_panel.hpp"
#include "popup_panel.hpp"
#include "filter_para_panel.hpp"
#include "right_click_panel.hpp"
#include "solo_panel.hpp"

namespace zlpanel {
    class ButtonPanel final : public juce::Component,
                              private juce::Timer {
    public:
        explicit ButtonPanel(PluginProcessor &processor, zlgui::UIBase &base,
                             size_t &selected_band_idx);

        ~ButtonPanel() override;

        void resized() override;

        void repaintCallBackSlow();

        DraggerPanel &getDraggerPanel(const size_t band) {
            return *dragger_panels_[band];
        }

        zlgui::dragger::Dragger &getDragger(const size_t band) {
            return dragger_panels_[band]->getDragger();
        }

        PopupPanel &getPopupPanel() {
            return popup_panel_;
        }

        void updateBand();

        void setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &status);

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        void turnOnSolo(size_t band);

        void turnOffSolo();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        size_t previous_band_idx_{zlp::kBandNum};
        size_t &selected_band_idx_;

        std::atomic<float> &eq_max_db_id_ref_;
        float eq_max_db_id_{-1.f};

        zlgui::attachment::ComponentUpdater updater_;

        std::atomic<float> *filter_type_ref_{nullptr};
        float c_filter_type_{-1.f};

        zlgui::slider::SnappingSlider q_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true> > q_attachment_;

        zlgui::slider::SnappingSlider slope_slider_;
        std::unique_ptr<zlgui::attachment::SliderAttachment<true> > slope_attachment_;

        zlgui::combobox::CompactCombobox eq_db_box_;
        zlgui::attachment::ComboBoxAttachment<true> eq_db_box_attachment_;

        FilterParaPanel para_panel_;
        PopupPanel popup_panel_;
        RightClickPanel right_click_panel_;
        std::array<std::unique_ptr<DraggerPanel>, zlp::kBandNum> dragger_panels_;
        SoloPanel solo_panel_;

        static constexpr std::array init_IDs{
            zlp::PFilterType::kID, zlp::PGain::kID, zlp::PFreq::kID, zlp::PQ::kID, zlp::POrder::kID,
            zlp::PFilterStatus::kID
        };

        void timerCallback() override;
    };
} // zlpanel
