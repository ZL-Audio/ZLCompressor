// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "BinaryData.h"
#include "lufs_button.hpp"

namespace zlpanel {
    LUFSButton::LUFSButton(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          learn_drawable_(juce::Drawable::createFromImageData(BinaryData::learn_svg, BinaryData::learn_svgSize)),
          learn_button_(base, learn_drawable_.get(), learn_drawable_.get()) {
        learn_button_.setImageAlpha(.5f, .5f, 1.f, 1.f);
        learn_button_.addMouseListener(this, true);

        learn_button_.getButton().onClick = [this]() {
            if (learn_button_.getButton().getToggleState()) {
                p_ref_.getCompressController().setLUFSMatcherOn(true);
                base_.setProperty(zlgui::PanelSettingIdx::kLUFSLearnButton, true);
                setAlpha(1.f);
            } else {
                p_ref_.getCompressController().setLUFSMatcherOn(false);
                base_.setProperty(zlgui::PanelSettingIdx::kLUFSLearnButton, false);

                const auto c_diff = -p_ref_.getCompressController().getLUFSMatcherDiff();
                const auto c_gain = p_ref_.parameters_.getRawParameterValue(zlp::POutGain::kID)->load();
                const auto c_wet = p_ref_.parameters_.getRawParameterValue(zlp::PWet::kID)->load();

                if (c_wet < 0.1f) {
                    return;
                }
                const auto target_gain = c_gain + c_diff / (c_wet / 100.f);

                auto *para = p_ref_.parameters_.getParameter(zlp::POutGain::kID);
                para->beginChangeGesture();
                para->setValueNotifyingHost(para->convertTo0to1(target_gain));
                para->endChangeGesture();

                setAlpha(mouse_over_ ? 1.f : 0.f);
            }
        };

        addAndMakeVisible(learn_button_);

        setInterceptsMouseClicks(false, true);
        setAlpha(.0f);
        setBufferedToImage(true);
    }

    LUFSButton::~LUFSButton() {
        p_ref_.getCompressController().setLUFSMatcherOn(false);
    }

    void LUFSButton::paint(juce::Graphics &g) {
        const auto bound = getLocalBounds().toFloat();
        juce::Path path;
        path.startNewSubPath(bound.getBottomLeft());
        path.lineTo(bound.getBottomRight());
        path.lineTo(bound.getRight() - bound.getHeight(), bound.getY());
        path.lineTo(bound.getX() + bound.getHeight(), bound.getY());

        g.setColour(base_.getBackgroundColor());
        g.fillPath(path);
    }

    void LUFSButton::resized() {
        learn_button_.setBounds(getLocalBounds());
    }

    void LUFSButton::mouseEnter(const juce::MouseEvent &) {
        mouse_over_ = true;
        setAlpha(1.f);
    }

    void LUFSButton::mouseExit(const juce::MouseEvent &) {
        mouse_over_ = false;
        if (learn_button_.getButton().getToggleState()) {
            setAlpha(1.f);
        } else {
            setAlpha(0.f);
        }
    }
} // zlpanel
