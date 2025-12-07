// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "right_click_panel.hpp"

namespace zlpanel {
    RightClickPanel::RightClickPanel(PluginProcessor& processor, zlgui::UIBase& base,
                                     size_t& selected_band_idx)
        : p_ref_(processor), base_(base), selected_band_idx_(selected_band_idx),
          invert_gain_button_(base, "Invert Gain", ""),
          copy_button_(base, "Copy", ""),
          paste_button_(base, "Paste", ""),
          items_set_(base.getSelectedBandSet()) {
        invert_gain_button_.getButton().onClick = [this]() {
            invertGain();
        };
        copy_button_.getButton().onClick = [this]() {
            copy();
        };
        paste_button_.getButton().onClick = [this]() {
            paste();
        };
        for (auto& button : {&invert_gain_button_, &copy_button_, &paste_button_}) {
            button->getLAF().setFontScale(1.25f);
            addAndMakeVisible(button);
        }
        setInterceptsMouseClicks(false, true);
    }

    void RightClickPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    void RightClickPanel::updateCopyVisibility(const bool show_copy) {
        if (!show_copy) {
            copy_button_.setAlpha(.25f);
            copy_button_.setInterceptsMouseClicks(false, false);
        }
        else {
            copy_button_.setAlpha(1.f);
            copy_button_.setInterceptsMouseClicks(false, true);
        }
    }

    void RightClickPanel::resized() {
        auto bound = getLocalBounds();
        bound.removeFromLeft(juce::roundToInt(base_.getFontSize()));
        bound.removeFromRight(juce::roundToInt(base_.getFontSize()));
        const auto button_height = bound.getHeight() / 3;
        invert_gain_button_.setBounds(bound.removeFromTop(button_height));
        copy_button_.setBounds(bound.removeFromTop(button_height));
        paste_button_.setBounds(bound.removeFromTop(button_height));
    }

    void RightClickPanel::invertGain() {
        setVisible(false);

        auto* para = p_ref_.parameters_.getParameter(zlp::PGain::kID + std::to_string(selected_band_idx_));
        para->beginChangeGesture();
        para->setValueNotifyingHost(1.0f - para->getValue());
        para->endChangeGesture();
    }

    void RightClickPanel::copy() {
        setVisible(false);

        juce::ValueTree tree{"filter_info"};
        auto selected_band = items_set_.getItemArray();
        if (selected_band.isEmpty()) {
            selected_band.add(selected_band_idx_);
        }

        int i = 0;
        for (const size_t band : selected_band) {
            juce::ValueTree filter{juce::Identifier{"filter" + std::to_string(i)}};
            tree.addChild(filter, i, nullptr);
            i += 1;
            const auto band_string = std::to_string(band);
            for (auto& para_ID : kIDs) {
                filter.setProperty(para_ID,
                                   p_ref_.parameters_.getParameter(
                                       para_ID + band_string)->getCurrentValueAsText(),
                                   nullptr);
            }
        }

        juce::SystemClipboard::copyTextToClipboard(tree.toXmlString());
    }

    void RightClickPanel::paste() {
        const auto tree = juce::ValueTree::fromXml(juce::SystemClipboard::getTextFromClipboard());
        if (!tree.hasType("filter_info")) { return; }

        setVisible(false);

        base_.getSelectedBandSet().deselectAll();
        for (size_t i = 0; i < zlp::kBandNum; ++i) {
            const auto filter = tree.getChildWithName(juce::Identifier{"filter" + std::to_string(i)});

            if (!filter.isValid()) { return; }

            size_t band_idx = zlp::kBandNum;
            for (size_t next_available = 0; next_available < zlp::kBandNum; ++next_available) {
                if (p_ref_.parameters_.getRawParameterValue(
                        zlp::PFilterStatus::kID + std::to_string(next_available))->load(std::memory_order::relaxed) <
                    .5f) {
                    band_idx = next_available;
                    break;
                }
            }
            if (band_idx == zlp::kBandNum) { return; }

            const auto band_string = std::to_string(band_idx);
            for (auto& para_ID : kIDs) {
                if (filter.hasProperty(para_ID)) {
                    auto* para = p_ref_.parameters_.getParameter(para_ID + band_string);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(para->getValueForText(filter.getProperty(para_ID)));
                    para->endChangeGesture();
                }
            }
            base_.getSelectedBandSet().addToSelection(band_idx);
            selected_band_idx_ = band_idx;
        }
    }
}
