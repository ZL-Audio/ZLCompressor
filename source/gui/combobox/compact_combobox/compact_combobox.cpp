// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "compact_combobox.hpp"

namespace zlgui::combobox {
    CompactCombobox::CompactCombobox(const juce::StringArray &choices,
                                     UIBase &base, const juce::String &tooltip_text,
                                     const std::vector<juce::String> &item_labels)
        : base_(base),
          box_laf_(base) {
        if (item_labels.size() < static_cast<size_t>(choices.size())) {
            combo_box_.addItemList(choices, 1);
        } else {
            const auto menu = combo_box_.getRootMenu();
            for (int i = 0; i < choices.size(); ++i) {
                juce::PopupMenu::Item item;
                item.itemID = i + 1;
                item.text = choices[i];
                item.tooltipText = item_labels[static_cast<size_t>(i)];
                item.isEnabled = true;
                item.isTicked = false;
                menu->addItem(item);
            }
        }

        combo_box_.setScrollWheelEnabled(false);
        combo_box_.setInterceptsMouseClicks(false, false);
        combo_box_.setLookAndFeel(&box_laf_);
        combo_box_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(combo_box_);

        setEditable(true);

        if (tooltip_text.length() > 0) {
            SettableTooltipClient::setTooltip(tooltip_text);
        }
    }


    CompactCombobox::~CompactCombobox() {
        combo_box_.setLookAndFeel(nullptr);
    }

    void CompactCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                            juce::jmin(bound.getHeight(), base_.getFontSize() * 2.f));
        combo_box_.setBounds(bound.toNearestInt());
    }

    void CompactCombobox::mouseUp(const juce::MouseEvent &event) {
        combo_box_.mouseUp(event);
    }

    void CompactCombobox::mouseDown(const juce::MouseEvent &event) {
        combo_box_.mouseDown(event);
    }

    void CompactCombobox::mouseDrag(const juce::MouseEvent &event) {
        combo_box_.mouseDrag(event);
    }

    void CompactCombobox::mouseEnter(const juce::MouseEvent &event) {
        combo_box_.mouseEnter(event);
        box_laf_.setBoxAlpha(1.f);
        combo_box_.repaint();
    }

    void CompactCombobox::mouseExit(const juce::MouseEvent &event) {
        combo_box_.mouseExit(event);
        box_laf_.setBoxAlpha(0.f);
        combo_box_.repaint();
    }

    void CompactCombobox::mouseMove(const juce::MouseEvent &event) {
        combo_box_.mouseMove(event);
    }
}
