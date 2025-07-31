// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlgui::slider {
    class SnappingSlider final : public juce::Slider {
    public:
        explicit SnappingSlider(UIBase &base, const juce::String &name = "") : juce::Slider(name), base_(base) {
        }

        void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override {
            juce::MouseWheelDetails w = wheel;
            if (std::signbit(cumulative_x_) != std::signbit(w.deltaX)) {
                cumulative_x_ = 0.f;
            }
            if (std::signbit(cumulative_y_) != std::signbit(w.deltaY)) {
                cumulative_y_ = 0.f;
            }
            if (e.mods.isShiftDown()) {
                const auto sensitivity_mul =
                    base_.getSensitivity(kMouseWheelFine) * (base_.getIsMouseWheelShiftReverse() ? -1.f : 1.f);
                cumulative_x_ += w.deltaX * sensitivity_mul;
                cumulative_y_ += w.deltaY * sensitivity_mul;
            } else {
                const auto sensitivity_mul = base_.getSensitivity(kMouseWheel);
                cumulative_x_ += w.deltaX * sensitivity_mul;
                cumulative_y_ += w.deltaY * sensitivity_mul;
            }

            const auto current_value = getValue();
            const auto cumulative_wheel_delta = std::abs(cumulative_x_) > std::abs(cumulative_y_)
                                                    ? -cumulative_x_
                                                    : cumulative_y_;
            const auto delta = getMouseWheelDelta(current_value,
                                                  cumulative_wheel_delta * (w.isReversed ? -1.0f : 1.0f));
            if (std::abs(delta) > getInterval() * 0.75) {
                w.deltaX = cumulative_x_;
                w.deltaY = cumulative_y_;
                cumulative_x_ = 0.f;
                cumulative_y_ = 0.f;
                Slider::mouseWheelMove(e, w);
            }
        }

    protected:
        UIBase &base_;
        float cumulative_x_{0.f}, cumulative_y_{0.f};

        double getMouseWheelDelta(const double value, const float wheel_delta) {
            const auto proportion_delta = static_cast<double>(wheel_delta) * 0.15;
            const auto current_pos = valueToProportionOfLength(value);
            const auto new_pos = juce::jlimit(0.0, 1.0, current_pos + proportion_delta);
            if (std::abs(current_pos - 1.0) < 1e-6 && std::abs(new_pos - 1.0) < 1e-6) {
                cumulative_x_ = 0.f;
                cumulative_y_ = 0.f;
            }
            if (std::abs(current_pos - 0.0) < 1e-6 && std::abs(new_pos - 0.0) < 1e-6) {
                cumulative_x_ = 0.f;
                cumulative_y_ = 0.f;
            }
            return proportionOfLengthToValue(new_pos) - value;
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnappingSlider)
    };
}
