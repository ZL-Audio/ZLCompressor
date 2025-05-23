// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../attachment/component_attachment.hpp"

namespace zlgui::attachment {
    template<bool UpdateFromAPVTS = true>
    class ButtonAttachment final : public ComponentAttachment,
                                   private juce::AudioProcessorValueTreeState::Listener,
                                   private juce::Button::Listener {
    public:
        ButtonAttachment(juce::Button &button,
                         juce::AudioProcessorValueTreeState &apvts, const juce::String &parameter_ID,
                         std::atomic<bool> &updater_flag,
                         const juce::NotificationType notification_type =
                                 juce::NotificationType::sendNotificationSync)
            : button_(button), notification_type_(notification_type),
              apvts_(apvts), parameter_ID_(parameter_ID),
              parameter_ref_(*apvts_.getParameter(parameter_ID_)),
              updater_flag_ref_(updater_flag) {
            // add parameter listener
            if (UpdateFromAPVTS) {
                apvts_.addParameterListener(parameter_ID_, this);
                parameterChanged(parameter_ID_, apvts_.getRawParameterValue(parameter_ID_)->load());
            }
            // add combobox listener
            button_.addListener(this);
        }

        ~ButtonAttachment() override {
            apvts_.removeParameterListener(parameter_ID_, this);
        }

        void updateComponent() override {
            if (UpdateFromAPVTS) {
                const auto current_flag = atomic_flag_.load();
                if (current_flag != button_.getToggleState()) {
                    button_.setToggleState(current_flag, notification_type_);
                }
            }
        }

    private:
        juce::Button &button_;
        juce::NotificationType notification_type_{juce::NotificationType::sendNotificationSync};
        juce::AudioProcessorValueTreeState &apvts_;
        juce::String parameter_ID_;
        juce::RangedAudioParameter &parameter_ref_;
        std::atomic<bool> &updater_flag_ref_;
        std::atomic<bool> atomic_flag_{false};

        void parameterChanged(const juce::String &parameter_ID, const float new_value) override {
            atomic_flag_.store(new_value > .5f);
            updater_flag_ref_.store(true);
        }

        void buttonStateChanged(juce::Button *) override {
            parameter_ref_.beginChangeGesture();
            parameter_ref_.setValueNotifyingHost(static_cast<float>(atomic_flag_.load()));
            parameter_ref_.endChangeGesture();
        }

        void buttonClicked(juce::Button *) override {
        }
    };
}
