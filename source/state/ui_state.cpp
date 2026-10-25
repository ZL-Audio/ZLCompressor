// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "ui_state.hpp"

namespace zlgui {
    void UIState::loadFromAPVTS() {
        for (size_t i = 0; i < kColourNum; ++i) {
            const auto r = static_cast<juce::uint8>(std::round(loadPara(std::string(kColourNames[i]) + "_r")));
            const auto g = static_cast<juce::uint8>(std::round(loadPara(std::string(kColourNames[i]) + "_g")));
            const auto b = static_cast<juce::uint8>(std::round(loadPara(std::string(kColourNames[i]) + "_b")));
            const auto o = loadPara(std::string(kColourNames[i]) + "_o");
            setColourByIdx(static_cast<ColourIdx>(i), juce::Colour(r, g, b, o));
        }
        gui.setFontMode(static_cast<size_t>(std::round(loadPara(zlstate::PFontMode::kID))));
        gui.setFontScale(loadPara(zlstate::PFontScale::kID));
        gui.setStaticFontSize(loadPara(zlstate::PStaticFontSize::kID));
        window_size_fix_ = loadPara(zlstate::PWindowSizeFix::kID) > .5f;
        gui.setSensitivity(state.getRawParameterValue(zlstate::PWheelSensitivity::kID)->load(), kMouseWheel);
        gui.setSensitivity(state.getRawParameterValue(zlstate::PWheelFineSensitivity::kID)->load(), kMouseWheelFine);
        gui.setSensitivity(state.getRawParameterValue(zlstate::PSliderSensitivity::kID)->load(), kMouseSlider);
        gui.setSensitivity(state.getRawParameterValue(zlstate::PSliderFineSensitivity::kID)->load(), kMouseSliderFine);
        gui.setSensitivity(state.getRawParameterValue(zlstate::PDraggerSensitivity::kID)->load(), kMouseDragger);
        gui.setSensitivity(state.getRawParameterValue(zlstate::PDraggerFineSensitivity::kID)->load(), kMouseDraggerFine);
        gui.setSensitivity(loadPara(zlstate::PWheelComboboxSensitivity::kID), kMouseWheelCombobox);
        gui.setIsMouseWheelShiftReverse(state.getRawParameterValue(zlstate::PWheelShiftReverse::kID)->load() > .5f);
        gui.setRotaryStyleID(static_cast<size_t>(state.getRawParameterValue(zlstate::PRotaryStyle::kID)->load()));
        gui.setIsSliderDoubleClickOpenEditor(loadPara(zlstate::PSliderDoubleClickFunc::kID) > .5f);
        refresh_rate_id_.store(
            static_cast<size_t>(std::round(state.getRawParameterValue(zlstate::PTargetRefreshSpeed::kID)->load())));
        fft_quality_.store(static_cast<size_t>(std::round(loadPara(zlstate::PFFTQuality::kID))));
        fft_extra_tilt_.store(loadPara(zlstate::PFFTExtraTilt::kID));
        fft_extra_speed_.store(loadPara(zlstate::PFFTExtraSpeed::kID));
        gui.setComboboxAlignment(static_cast<size_t>(std::round(loadPara(zlstate::PComboboxAlignment::kID))));
        mag_curve_thickness_.store(loadPara(zlstate::PMagCurveThickness::kID));
        eq_curve_thickness_.store(loadPara(zlstate::PEQCurveThickness::kID));
        gui.setTooltipLandID(
            static_cast<size_t>(std::round(state.getRawParameterValue(zlstate::PTooltipLang::kID)->load())));
        gui.setCMap1Idx(static_cast<size_t>(loadPara(zlstate::PColourMap1Idx::kID)));
        gui.setCMap2Idx(static_cast<size_t>(loadPara(zlstate::PColourMap2Idx::kID)));
        enter_solo_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PEnterSoloMouse::kID)));
        enter_solo_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PEnterSoloKey::kID)));
        exit_solo_mouse_ = static_cast<MouseActionType>(std::round(loadPara(zlstate::PExitSoloMouse::kID)));
        exit_solo_key_ = static_cast<KeyActionType>(std::round(loadPara(zlstate::PExitSoloKey::kID)));
    }

    void UIState::saveToAPVTS() const {
        for (size_t i = 0; i < kColourNum; ++i) {
            const auto colour = getColourByIdx(static_cast<ColourIdx>(i));
            const std::array<float, 4> rgbo = {
                static_cast<float>(colour.getRed()),
                static_cast<float>(colour.getGreen()),
                static_cast<float>(colour.getBlue()),
                colour.getFloatAlpha()
            };
            const std::array<std::string, 4> ID{
                std::string(kColourNames[i]) + "_r",
                std::string(kColourNames[i]) + "_g",
                std::string(kColourNames[i]) + "_b",
                std::string(kColourNames[i]) + "_o"
            };
            for (size_t j = 0; j < 4; ++j) {
                savePara(ID[j], rgbo[j]);
            }
        }
        savePara(zlstate::PFontMode::kID, static_cast<float>(gui.getFontMode()));
        savePara(zlstate::PFontScale::kID, gui.getFontScale());
        savePara(zlstate::PStaticFontSize::kID, gui.getStaticFontSize());
        savePara(zlstate::PWindowSizeFix::kID, window_size_fix_);
        savePara(zlstate::PWheelSensitivity::kID, gui.getSensitivity(kMouseWheel));
        savePara(zlstate::PWheelFineSensitivity::kID, gui.getSensitivity(kMouseWheelFine));
        savePara(zlstate::PSliderSensitivity::kID, gui.getSensitivity(kMouseSlider));
        savePara(zlstate::PSliderFineSensitivity::kID, gui.getSensitivity(kMouseSliderFine));
        savePara(zlstate::PDraggerSensitivity::kID, gui.getSensitivity(kMouseDragger));
        savePara(zlstate::PDraggerFineSensitivity::kID, gui.getSensitivity(kMouseDraggerFine));
        savePara(zlstate::PWheelComboboxSensitivity::kID, gui.getSensitivity(kMouseWheelCombobox));
        savePara(zlstate::PWheelShiftReverse::kID,
                 static_cast<float>(gui.getIsMouseWheelShiftReverse()));
        savePara(zlstate::PRotaryStyle::kID, static_cast<float>(gui.getRotaryStyleID()));
        savePara(zlstate::PSliderDoubleClickFunc::kID,
                 static_cast<float>(gui.getIsSliderDoubleClickOpenEditor()));
        savePara(zlstate::PTargetRefreshSpeed::kID,
                 static_cast<float>(refresh_rate_id_.load(std::memory_order::relaxed)));
        savePara(zlstate::PFFTQuality::kID, static_cast<float>(fft_quality_.load(std::memory_order::relaxed)));
        savePara(zlstate::PFFTExtraTilt::kID, fft_extra_tilt_.load(std::memory_order::relaxed));
        savePara(zlstate::PFFTExtraSpeed::kID, fft_extra_speed_.load(std::memory_order::relaxed));
        savePara(zlstate::PComboboxAlignment::kID,
                 static_cast<float>(gui.getComboboxAlignment()));
        savePara(zlstate::PMagCurveThickness::kID, mag_curve_thickness_.load(std::memory_order::relaxed));
        savePara(zlstate::PEQCurveThickness::kID, eq_curve_thickness_.load(std::memory_order::relaxed));
        savePara(zlstate::PTooltipLang::kID, static_cast<float>(gui.getTooltipLangID()));
        savePara(zlstate::PColourMap1Idx::kID, static_cast<float>(gui.getCMap1Idx()));
        savePara(zlstate::PColourMap2Idx::kID, static_cast<float>(gui.getCMap2Idx()));
        savePara(zlstate::PEnterSoloMouse::kID, static_cast<float>(enter_solo_mouse_));
        savePara(zlstate::PEnterSoloKey::kID, static_cast<float>(enter_solo_key_));
        savePara(zlstate::PExitSoloMouse::kID, static_cast<float>(exit_solo_mouse_));
        savePara(zlstate::PExitSoloKey::kID, static_cast<float>(exit_solo_key_));
    }

    bool UIState::checkMouseKeyMatch(const MouseActionType action_type, const juce::ModifierKeys& mods,
                                    const MouseActionType mouse_option, const KeyActionType key_option) const {
        bool key_match{false};
        switch (key_option) {
        case KeyActionType::kNone:
            key_match = !mods.isCommandDown() && !mods.isShiftDown() && !mods.isAltDown();
            break;
        case KeyActionType::kCmdCtrl:
            key_match = mods.isCommandDown();
            break;
        case KeyActionType::kShift:
            key_match = mods.isShiftDown();
            break;
        case KeyActionType::kAlt:
            key_match = mods.isAltDown();
            break;
        }
        return key_match && action_type == mouse_option;
    }
}
