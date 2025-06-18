// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "BinaryData.h"
#include "popup_panel.hpp"

namespace zlpanel {
    PopupPanel::PopupPanel(PluginProcessor &processor, zlgui::UIBase &base, size_t &selected_band_idx)
        : p_ref_(processor), base_(base), selected_band_idx_(selected_band_idx),
          bypass_drawable_(juce::Drawable::createFromImageData(BinaryData::mode_off_on_svg,
                                                               BinaryData::mode_off_on_svgSize)),
          bypass_button_(base, bypass_drawable_.get(), bypass_drawable_.get()),
          close_drawable_(juce::Drawable::createFromImageData(BinaryData::close_svg,
                                                              BinaryData::close_svgSize)),
          close_button_(base, close_drawable_.get(), nullptr, ""),
          ftype_box_(zlp::PFilterType::kChoices, base),
          slope_box_(zlp::POrder::kChoices, base) {
        bypass_button_.getButton().onClick = [this]() {
            auto *para = p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + std::to_string(band_));
            para->beginChangeGesture();
            if (bypass_button_.getButton().getToggleState()) {
                para->setValueNotifyingHost(1.f);
            } else {
                para->setValueNotifyingHost(.5f);
            }
            para->endChangeGesture();
        };
        bypass_button_.setImageAlpha(.5f, .5f, 1.f, 1.f);
        bypass_button_.setBufferedToImage(true);
        addAndMakeVisible(bypass_button_);

        close_button_.getButton().onClick = [this]() {
            auto *para = p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + std::to_string(band_));
            para->beginChangeGesture();
            para->setValueNotifyingHost(0.f);
            para->endChangeGesture();

            selected_band_idx_ = zlp::kBandNum;
        };
        close_button_.setBufferedToImage(true);
        addAndMakeVisible(close_button_);

        for (auto &box: {&ftype_box_, &slope_box_}) {
            box->getLAF().setFontScale(1.125f);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        setBufferedToImage(true);
    }

    void PopupPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getTextColor().withAlpha(.33f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(),
                               base_.getFontSize() * .5f);
        g.setColour(base_.getBackgroundColor().withAlpha(.66f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(),
                               base_.getFontSize() * .5f);
    }

    int PopupPanel::getIdealWidth() const {
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale * .75f);
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale * .75f);
        return slider_width + button_height;
    }

    int PopupPanel::getIdealHeight() const {
        return 2 * juce::roundToInt(base_.getFontSize() * kButtonScale * .75f);
    }

    void PopupPanel::resized() {
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale * .75f);
        auto bound = getLocalBounds();
        auto t_bound = bound.removeFromTop(bound.getHeight() / 2);

        bypass_button_.setBounds(t_bound.removeFromRight(button_height));
        ftype_box_.setBounds(t_bound);

        auto close_button_bound = bound.removeFromRight(button_height);
        const auto close_button_padding = close_button_bound.getHeight() / 16;
        close_button_bound.reduce(close_button_padding, close_button_padding);
        close_button_.setBounds(close_button_bound);
        slope_box_.setBounds(bound);

        const auto popup_option = juce::PopupMenu::Options()
                .withParentComponent(getParentComponent())
                .withMinimumNumColumns(2)
                .withMinimumWidth(ftype_box_.getWidth() * 2);
        for (auto &box: {&ftype_box_, &slope_box_}) {
            box->getLAF().setOption(popup_option);
        }
    }

    void PopupPanel::updateBand() {
        if (band_ == selected_band_idx_) return;
        band_ = selected_band_idx_;

        if (selected_band_idx_ == zlp::kBandNum) {
            return;
        }

        bypass_ref_ = p_ref_.parameters_.getRawParameterValue(zlp::PFilterStatus::kID + std::to_string(band_));

        ftype_attachment_.reset();
        ftype_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true> >(
            ftype_box_.getBox(), p_ref_.parameters_,
            zlp::PFilterType::kID + std::to_string(band_), updater_);

        slope_attachment_.reset();
        slope_attachment_ = std::make_unique<zlgui::attachment::ComboBoxAttachment<true> >(
            slope_box_.getBox(), p_ref_.parameters_,
            zlp::POrder::kID + std::to_string(band_), updater_);

        updater_.updateComponents();
    }

    void PopupPanel::repaintCallBackSlow() {
        updater_.updateComponents();

        if (bypass_ref_ != nullptr) {
            const auto is_on = bypass_ref_->load(std::memory_order::relaxed) > 1.5f;
            bypass_button_.getButton().setToggleState(is_on, juce::dontSendNotification);
        }
    }
} // zlpanel
