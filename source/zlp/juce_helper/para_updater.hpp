// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>

namespace zlp::juce_helper {
    /**
     * a helper class that run parameter updating on the message thread (async)
     */
    class ParaUpdater final : private juce::AsyncUpdater {
    public:
        explicit ParaUpdater(const juce::AudioProcessorValueTreeState &parameter,
                             const std::string &parameter_idx) {
            para_ = parameter.getParameter(parameter_idx);
        }

        void update(const float para_value) {
            value_.store(para_value);
            triggerAsyncUpdate();
        }

        void updateSync(const float para_value) {
            para_->beginChangeGesture();
            para_->setValueNotifyingHost(para_value);
            para_->endChangeGesture();
        }

        juce::RangedAudioParameter *getPara() const { return para_; }

    private:
        juce::RangedAudioParameter *para_;
        std::atomic<float> value_{};

        void handleAsyncUpdate() override {
            para_->beginChangeGesture();
            para_->setValueNotifyingHost(value_.load());
            para_->endChangeGesture();
        }
    };
}
