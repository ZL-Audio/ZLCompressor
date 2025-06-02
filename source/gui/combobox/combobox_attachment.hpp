// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../attachment/component_updater.hpp"

namespace zlgui::attachment {
    template<bool UpdateFromAPVTS = true>
    class ComboBoxAttachment final : public ComponentAttachment,
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ComboBox::Listener {
    public:
        ComboBoxAttachment(juce::ComboBox &box,
                           juce::AudioProcessorValueTreeState &apvts,
                           const juce::String &parameter_ID,
                           ComponentUpdater &updater,
                           const juce::NotificationType notification_type =
                                   juce::NotificationType::sendNotificationSync)
            : box_(box), notification_type_(notification_type),
              apvts_(apvts), parameter_ID_(parameter_ID),
              parameter_ref_(*apvts_.getParameter(parameter_ID_)),
              updater_ref_(updater) {
            // add parameter listener
            if (UpdateFromAPVTS) {
                apvts_.addParameterListener(parameter_ID_, this);
                parameterChanged(parameter_ID_,
                                 apvts_.getRawParameterValue(parameter_ID_)->load(std::memory_order::relaxed));
                updater_ref_.addAttachment(*this);
            } else {
                parameterChanged(parameter_ID_,
                                 apvts_.getRawParameterValue(parameter_ID_)->load(std::memory_order::relaxed));
                updateComponent();
            }
            // add combobox listener
            box_.addListener(this);
        }

        ~ComboBoxAttachment() override {
            if (UpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_ID_, this);
            }
        }

        void updateComponent() override {
            const auto current_index = atomic_index_.load(std::memory_order::relaxed);
            if (current_index != box_.getSelectedItemIndex()) {
                box_.setSelectedItemIndex(current_index, notification_type_);
            }
        }

    private:
        juce::ComboBox &box_;
        juce::NotificationType notification_type_{juce::NotificationType::sendNotificationSync};
        juce::AudioProcessorValueTreeState &apvts_;
        juce::String parameter_ID_;
        juce::RangedAudioParameter &parameter_ref_;
        ComponentUpdater &updater_ref_;
        std::atomic<int> atomic_index_{0};

        void parameterChanged(const juce::String &, const float new_value) override {
            atomic_index_.store(static_cast<int>(new_value), std::memory_order::relaxed);
            updater_ref_.getFlag().store(true, std::memory_order::release);
        }

        void comboBoxChanged(juce::ComboBox *) override {
            parameter_ref_.beginChangeGesture();
            parameter_ref_.setValueNotifyingHost(
                parameter_ref_.convertTo0to1(static_cast<float>(box_.getSelectedItemIndex())));
            parameter_ref_.endChangeGesture();
        }
    };
}
