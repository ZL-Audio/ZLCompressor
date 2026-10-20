// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "state_definitions.hpp"
#include "../gui/interface_definitions.hpp"

namespace zlgui {
    enum class MouseActionType {
        kLeftClick,
        kRightClick,
        kLeftDoubleClick,
        kRightDoubleClick,
    };

    enum class KeyActionType {
        kNone,
        kCmdCtrl,
        kShift,
        kAlt,
    };

    enum ColourIdx {
        kTextColour,
        kBackgroundColour,
        kShadowColour,
        kGlowColour,
        kPreColour,
        kPostColour,
        kReductionColour,
        kSideChainColour,
        kComputerColour,
        kGridColour,
        kColourNum
    };

    static constexpr std::array<std::string_view, kColourNum> kColourNames = zlstate::kColourNames;

    enum PanelSettingIdx {
        kLUFSLearnButton,
        kUISettingPanel,
        kUISettingChanged,
        kFFTFrozen,
        kPresetBrowser,
        kAnalyzerSettingPanel,
        kPanelSettingNum
    };

    inline std::array kPanelSettingIdentifiers{
        juce::Identifier("lufs_learn_button"),
        juce::Identifier("ui_setting_panel"),
        juce::Identifier("ui_setting_changed"),
        juce::Identifier("fft_frozen"),
        juce::Identifier("preset_browser"),
        juce::Identifier("analyzer_setting_panel"),
    };

    static constexpr size_t kColourMap1Size = 10;
    static constexpr size_t kColourMap2Size = 6;

    inline std::array kColourMaps = {
        std::vector<juce::Colour>{
            juce::Colour(31, 119, 180),
            juce::Colour(255, 127, 14),
            juce::Colour(44, 160, 44),
            juce::Colour(214, 39, 40),
            juce::Colour(148, 103, 189),
            juce::Colour(140, 86, 75),
            juce::Colour(227, 119, 194),
            juce::Colour(127, 127, 127),
            juce::Colour(188, 189, 34),
            juce::Colour(23, 190, 207)
        },
        std::vector<juce::Colour>{
            juce::Colour(224, 136, 75),
            juce::Colour(0, 128, 241),
            juce::Colour(211, 95, 211),
            juce::Colour(41, 216, 215),
            juce::Colour(107, 152, 66),
            juce::Colour(115, 169, 180),
            juce::Colour(28, 136, 61),
            juce::Colour(128, 128, 128),
            juce::Colour(67, 66, 221),
            juce::Colour(232, 65, 48),
        },
        std::vector<juce::Colour>{
            juce::Colour(76, 114, 176),
            juce::Colour(85, 168, 104),
            juce::Colour(196, 78, 82),
            juce::Colour(129, 114, 178),
            juce::Colour(255, 196, 0),
            juce::Colour(100, 181, 205),
        },
        std::vector<juce::Colour>{
            juce::Colour(179, 141, 79),
            juce::Colour(170, 87, 151),
            juce::Colour(59, 177, 173),
            juce::Colour(126, 141, 77),
            juce::Colour(51, 70, 139),
            juce::Colour(155, 74, 50),
        },
        std::vector<juce::Colour>{
            juce::Colour(0, 63, 255),
            juce::Colour(3, 237, 58),
            juce::Colour(232, 0, 11),
            juce::Colour(138, 43, 226),
            juce::Colour(255, 196, 0),
            juce::Colour(0, 215, 255),
        },
        std::vector<juce::Colour>{
            juce::Colour(255, 192, 0),
            juce::Colour(252, 18, 197),
            juce::Colour(23, 255, 244),
            juce::Colour(117, 212, 29),
            juce::Colour(0, 59, 255),
            juce::Colour(255, 40, 0),
        }
    };

    class UIState {
    public:
        UIBase gui;

        explicit UIState(juce::AudioProcessorValueTreeState& apvts)
            : gui({kColourMaps.begin(), kColourMaps.end()}, zlstate::PColourMapIdx::kChoices),
            state(apvts) {
            loadFromAPVTS();
        }

        juce::Colour getColourByIdx(ColourIdx idx) const {
            const auto index = static_cast<size_t>(idx);
            return index < kThemeColourNum ? gui.getThemeColour(static_cast<ThemeColour>(index))
                                           : plugin_colours_[index - kThemeColourNum];
        }

        void setColourByIdx(const ColourIdx idx, const juce::Colour colour) {
            const auto index = static_cast<size_t>(idx);
            if (index < kThemeColourNum) {
                gui.setThemeColour(static_cast<ThemeColour>(index), colour);
            } else {
                plugin_colours_[index - kThemeColourNum] = colour;
            }
        }

        size_t getRefreshRateID() const {
            return refresh_rate_id_.load(std::memory_order::relaxed);
        }

        void setRefreshRateID(const size_t x) {
            refresh_rate_id_.store(x, std::memory_order::relaxed);
        }

        size_t getFFTQuality() const {
            return fft_quality_.load(std::memory_order::relaxed);
        }

        void setFFTQuality(const size_t x) {
            fft_quality_.store(x, std::memory_order::relaxed);
        }

        float getFFTExtraTilt() const {
            return fft_extra_tilt_.load(std::memory_order::relaxed);
        }

        void setFFTExtraTilt(const float x) {
            fft_extra_tilt_.store(x, std::memory_order::relaxed);
        }

        float getFFTExtraSpeed() const {
            return fft_extra_speed_.load(std::memory_order::relaxed);
        }

        void setFFTExtraSpeed(const float x) {
            fft_extra_speed_.store(x, std::memory_order::relaxed);
        }

        float getMagCurveThickness() const {
            return mag_curve_thickness_.load(std::memory_order::relaxed);
        }

        void setMagCurveThickness(const float x) {
            mag_curve_thickness_.store(x, std::memory_order::relaxed);
        }

        float getEQCurveThickness() const {
            return eq_curve_thickness_.load(std::memory_order::relaxed);
        }

        void setEQCurveThickness(const float x) {
            eq_curve_thickness_.store(x, std::memory_order::relaxed);
        }

        void loadFromAPVTS();

        void saveToAPVTS() const;

        void setIsEditorShowing(const bool x) { is_editor_showing_ = x; }

        bool getIsEditorShowing() const { return is_editor_showing_; }

        juce::ValueTree& getPanelValueTree() { return panel_value_tree_; }

        void setWindowSizeFix(const bool should_fix) { window_size_fix_ = should_fix; }

        bool getWindowSizeFix() const { return window_size_fix_; }

        bool isPanelIdentifier(const PanelSettingIdx idx, const juce::Identifier& identifier) const {
            return identifier == kPanelSettingIdentifiers[static_cast<size_t>(idx)];
        }

        juce::var getPanelProperty(const PanelSettingIdx idx) const {
            return panel_value_tree_.getProperty(kPanelSettingIdentifiers[static_cast<size_t>(idx)]);
        }

        void setPanelProperty(const PanelSettingIdx idx, const juce::var& v) {
            panel_value_tree_.setProperty(kPanelSettingIdentifiers[idx], v, nullptr);
        }

        juce::SelectedItemSet<size_t>& getSelectedBandSet() { return selected_band_set_; }

        bool checkMouseKeyMatch(MouseActionType action_type, const juce::ModifierKeys& mods,
                                MouseActionType mouse_option, KeyActionType key_option) const;

        bool isEnterSoloTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, enter_solo_mouse_, enter_solo_key_);
        }

        bool isExitSoloTriggered(const MouseActionType type, const juce::ModifierKeys& mods) const {
            return checkMouseKeyMatch(type, mods, exit_solo_mouse_, exit_solo_key_);
        }

        MouseActionType getEnterSoloMouse() const { return enter_solo_mouse_; }

        void setEnterSoloMouse(const MouseActionType x) { enter_solo_mouse_ = x; }

        KeyActionType getEnterSoloKey() const { return enter_solo_key_; }

        void setEnterSoloKey(const KeyActionType x) { enter_solo_key_ = x; }

        MouseActionType getExitSoloMouse() const { return exit_solo_mouse_; }

        void setExitSoloMouse(const MouseActionType x) { exit_solo_mouse_ = x; }

        KeyActionType getExitSoloKey() const { return exit_solo_key_; }

        void setExitSoloKey(const KeyActionType x) { exit_solo_key_ = x; }

    private:
        juce::AudioProcessorValueTreeState& state;
        juce::ValueTree panel_value_tree_{"panel_setting"};

        bool window_size_fix_{false};
        static constexpr size_t kThemeColourNum = static_cast<size_t>(ThemeColour::kCount);
        static_assert(kTextColour == 0 && kBackgroundColour == 1 && kShadowColour == 2 && kGlowColour == 3);
        std::array<juce::Colour, kColourNum - kThemeColourNum> plugin_colours_;
        std::atomic<size_t> refresh_rate_id_{2};
        std::atomic<size_t> fft_quality_{zlstate::PFFTQuality::kDefaultI};
        std::atomic<float> fft_extra_tilt_{0.f}, fft_extra_speed_{1.f};
        std::atomic<float> mag_curve_thickness_{1.f}, eq_curve_thickness_{1.f};

        bool is_editor_showing_{false};

        juce::SelectedItemSet<size_t> selected_band_set_;

        MouseActionType enter_solo_mouse_{MouseActionType::kRightClick};
        KeyActionType enter_solo_key_{KeyActionType::kNone};
        MouseActionType exit_solo_mouse_{MouseActionType::kRightClick};
        KeyActionType exit_solo_key_{KeyActionType::kNone};

        float loadPara(const std::string& id) const {
            return state.getRawParameterValue(id)->load(std::memory_order::relaxed);
        }

        void savePara(const std::string& id, const float x) const {
            const auto para = state.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(para->convertTo0to1(x));
            para->endChangeGesture();
        }

    };
}
