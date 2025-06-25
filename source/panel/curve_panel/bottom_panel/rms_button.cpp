// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "BinaryData.h"
#include "rms_button.hpp"

namespace zlpanel {
    RMSButton::RMSButton(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          updater_(),
          rms_drawable_(juce::Drawable::createFromImageData(BinaryData::rms_svg,
                                                            BinaryData::rms_svgSize)),
          rms_button_(base, rms_drawable_.get(), rms_drawable_.get(), ""),
          rms_attachment_(rms_button_.getButton(), p.parameters_,
                          zlp::PRMSON::kID, updater_),
          rms_open_drawable_(juce::Drawable::createFromImageData(BinaryData::arrow_menu_open_svg,
                                                                 BinaryData::arrow_menu_open_svgSize)),
          rms_close_drawable_(juce::Drawable::createFromImageData(BinaryData::arrow_menu_close_svg,
                                                                  BinaryData::arrow_menu_close_svgSize)),
          rms_open_close_button_(base, rms_open_drawable_.get(), rms_close_drawable_.get(), ""),
          rms_open_close_attachment_(rms_open_close_button_.getButton(), p.na_parameters_,
                                     zlstate::PRMSPanelDisplay::kID, updater_) {
        juce::ignoreUnused(p);
        rms_button_.setImageAlpha(.5f, .5f, 1.f, 1.f);
        addAndMakeVisible(rms_button_);
        rms_open_close_button_.setImageAlpha(1.f, 1.f, 1.f, 1.f);
        addChildComponent(rms_open_close_button_);

        setAlpha(0.f);
        setInterceptsMouseClicks(false, true);
        setBufferedToImage(true);
    }

    void RMSButton::paint(juce::Graphics &g) {
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        const auto bound = getLocalBounds().toFloat();
        juce::Path path;
        path.startNewSubPath(0.f, 0.f);
        path.lineTo(bound.getBottomLeft());
        path.lineTo(bound.getBottomRight());
        path.lineTo(bound.getWidth() - static_cast<float>(button_height), bound.getY());
        path.closeSubPath();

        g.setColour(base_.getBackgroundColor());
        g.fillPath(path);
    }

    int RMSButton::getIdealWidth() const {
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        return button_height * 2 + button_height / 2;
    }

    void RMSButton::resized() {
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale);
        auto bound = getLocalBounds();
        rms_button_.setBounds(bound.removeFromLeft(button_height));
        rms_open_close_button_.setBounds(bound.removeFromLeft(button_height / 2));
    }

    void RMSButton::repaintCallBackSlow() {
        updater_.updateComponents();
        rms_open_close_button_.setVisible(rms_button_.getButton().getToggleState());
        if (rms_button_.getButton().getToggleState() || isMouseOver(true)) {
            setAlpha(1.f);
        } else {
            setAlpha(0.f);
        }
        // close rms panel if rms button is off
        if (!rms_button_.getButton().getToggleState() && rms_open_close_button_.getButton().getToggleState()) {
            auto *para = p_ref_.na_parameters_.getParameter(zlstate::PRMSPanelDisplay::kID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(0.f);
            para->endChangeGesture();
        }
    }
} // zlpanel
