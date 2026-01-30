// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "right_control_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    RightControlPanel::RightControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                         const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base), label_laf_(base_),
        gain_slider_("Output Gain", base_,
                     tooltip_helper.getToolTipText(multilingual::kMakeup), 1.f),
        gain_attachment_(gain_slider_.getSlider1(), p.parameters_, zlp::POutGain::kID, updater_),
        wet_slider_("Wet", base_,
                    tooltip_helper.getToolTipText(multilingual::kWet), 1.f),
        wet_attachment_(wet_slider_.getSlider1(), p.parameters_, zlp::PWet::kID, updater_),
        inf_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_inf_svg,
                                                          BinaryData::dline_inf_svgSize)),
        inf_button_(base, inf_drawable_.get(), inf_drawable_.get()),
        inf_attachment_(inf_button_.getButton(), p.parameters_,
                        zlp::PRangeINF::kID, updater_),
        learn_drawable_(juce::Drawable::createFromImageData(BinaryData::dline_l_svg,
                                                            BinaryData::dline_l_svgSize)),
        learn_button_(base, learn_drawable_.get(), learn_drawable_.get(),
                      tooltip_helper.getToolTipText(multilingual::kLoudnessMatch)) {
        juce::ignoreUnused(p_ref_, base_);
        label_laf_.setFontScale(1.5f);

        gain_slider_.setPrecision(3);
        gain_label_.setText("Makeup", juce::dontSendNotification);
        gain_label_.setJustificationType(juce::Justification::centred);
        gain_label_.setInterceptsMouseClicks(false, false);
        gain_label_.setLookAndFeel(&label_laf_);
        gain_label_.setBufferedToImage(true);
        addAndMakeVisible(gain_label_);
        gain_slider_.setComponentID(zlp::POutGain::kID);
        gain_slider_.setBufferedToImage(true);
        addAndMakeVisible(gain_slider_);

        wet_label_.setText("Wet", juce::dontSendNotification);
        wet_label_.setJustificationType(juce::Justification::centred);
        wet_label_.setInterceptsMouseClicks(false, false);
        wet_label_.setLookAndFeel(&label_laf_);
        wet_label_.setBufferedToImage(true);
        addAndMakeVisible(wet_label_);
        wet_slider_.setComponentID(zlp::PWet::kID);
        wet_slider_.setBufferedToImage(true);
        addAndMakeVisible(wet_slider_);

        inf_button_.setImageAlpha(.5f, .5f, 1.f, 1.f);
        inf_button_.setBufferedToImage(true);
        addAndMakeVisible(inf_button_);

        learn_button_.setImageAlpha(.5f, .5f, 1.f, 1.f);
        learn_button_.addMouseListener(this, true);
        learn_button_.getButton().onClick = [this]() {
            if (learn_button_.getButton().getToggleState()) {
                p_ref_.getCompressController().setLUFSMatcherOn(true);
                base_.setPanelProperty(zlgui::PanelSettingIdx::kLUFSLearnButton, true);
            } else {
                p_ref_.getCompressController().setLUFSMatcherOn(false);
                base_.setPanelProperty(zlgui::PanelSettingIdx::kLUFSLearnButton, false);

                const auto c_diff = -p_ref_.getCompressController().getLUFSMatcherDiff();
                const auto c_gain = p_ref_.parameters_.getRawParameterValue(zlp::POutGain::kID)->load();
                const auto c_wet = p_ref_.parameters_.getRawParameterValue(zlp::PWet::kID)->load();

                if (c_wet < 0.1f) {
                    return;
                }
                const auto target_gain = c_gain + c_diff / (c_wet / 100.f);

                auto* para = p_ref_.parameters_.getParameter(zlp::POutGain::kID);
                para->beginChangeGesture();
                para->setValueNotifyingHost(para->convertTo0to1(target_gain));
                para->endChangeGesture();
            }
        };
        learn_button_.setBufferedToImage(true);
        addAndMakeVisible(learn_button_);

        base_.getPanelValueTree().addListener(this);
    }

    RightControlPanel::~RightControlPanel() {
        p_ref_.getCompressController().setLUFSMatcherOn(false);
        base_.getPanelValueTree().removeListener(this);
    }

    void RightControlPanel::paint(juce::Graphics&) {
    }

    int RightControlPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        return padding * 3 + small_slider_width * 2 + button_size;
    }

    void RightControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto small_slider_width = getSmallSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);

        bound.removeFromBottom(padding);
        const auto label_height = juce::roundToInt(base_.getFontSize() * 1.5f);
        {
            auto t_bound = bound.removeFromLeft(button_size);
            t_bound.removeFromTop(padding);
            const auto extra_padding = (t_bound.getHeight() - 2 * button_size) / 4;
            t_bound.removeFromTop(extra_padding);
            inf_button_.setBounds(t_bound.removeFromTop(button_size));
            t_bound.removeFromBottom(extra_padding);
            learn_button_.setBounds(t_bound.removeFromBottom(button_size));
        }
        {
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(small_slider_width);
            gain_label_.setBounds(t_bound.removeFromTop(label_height));
            gain_slider_.setBounds(t_bound);
        }
        {
            bound.removeFromLeft(padding);
            auto t_bound = bound.removeFromLeft(small_slider_width);
            wet_label_.setBounds(t_bound.removeFromTop(label_height));
            wet_slider_.setBounds(t_bound);
        }
    }

    void RightControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void RightControlPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& identifier) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kLUFSLearnButton, identifier)) {
            if (base_.getPanelProperty(zlgui::PanelSettingIdx::kLUFSLearnButton)) {
                gain_label_.setAlpha(.5f);
                gain_slider_.setAlpha(.5f);
                wet_label_.setAlpha(.5f);
                wet_slider_.setAlpha(.5f);

                gain_slider_.setInterceptsMouseClicks(false, false);
                wet_slider_.setInterceptsMouseClicks(false, false);
            } else {
                gain_label_.setAlpha(1.f);
                gain_slider_.setAlpha(1.f);
                wet_label_.setAlpha(1.f);
                wet_slider_.setAlpha(1.f);

                gain_slider_.setInterceptsMouseClicks(true, true);
                wet_slider_.setInterceptsMouseClicks(true, true);
            }
        }
    }
}
