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

namespace zlstate {
    inline static constexpr int kVersionHint = 1;

    inline static constexpr size_t kBandNUM = 8;

    // float
    template <class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID, kVersionHint),
                                                               T::kName, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string& suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string& suffix, const bool meta,
                                                              const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName).
                                                                    withMeta(meta);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::kRange.convertTo0to1(x);
        }
    };

    // bool
    template <class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID, kVersionHint),
                                                              T::kName, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string& suffix, bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string& suffix, const bool meta,
                                                             const bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName).
                                                                   withMeta(meta);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        inline static float convertTo01(const bool x) {
            return x ? 1.f : 0.f;
        }
    };

    // choice
    template <class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID, kVersionHint),
                                                                T::kName, T::kChoices, T::kDefaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string& suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string& suffix, const bool meta,
                                                               const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName).
                                                                     withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::kChoices.size() - 1);
        }
    };

    class PEQMaxDB : public ChoiceParameters<PEQMaxDB> {
    public:
        static constexpr auto kID = "eq_max_db";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "6 dB", "12 dB", "30 dB"
        };
        static constexpr std::array kDBs = {6.f, 12.f, 30.f};
        static constexpr int kDefaultI = 1;
    };

    class PAnalyzerMagType : public ChoiceParameters<PAnalyzerMagType> {
    public:
        static constexpr auto kID = "analyzer_mag_type";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Peak", "RMS"
        };
        static constexpr int kDefaultI = 0;
    };

    class PAnalyzerMoveType : public ChoiceParameters<PAnalyzerMoveType> {
    public:
        static constexpr auto kID = "analyzer_move_type";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Sync", "Slow", "Roll"
        };
        static constexpr int kDefaultI = 0;
    };

    class PAnalyzerMaxDB : public ChoiceParameters<PAnalyzerMaxDB> {
    public:
        static constexpr auto kID = "analyzer_max_db";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "6", "0", "-6", "-12", "-18", "-24", "-36"
        };
        static constexpr std::array kDBs = {6.f, 0.f, -6.f, -12.f, -18.f, -24.f, -36.f};
        static constexpr int kDefaultI = 1;

        static constexpr float getDBFromIndex(const float x) {
            return kDBs[static_cast<size_t>(std::round(x))];
        }
    };

    class PAnalyzerMinDB : public ChoiceParameters<PAnalyzerMinDB> {
    public:
        static constexpr auto kID = "analyzer_min_db";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "-9", "-18", "-36", "-54", "-72", "-120"
        };
        static constexpr std::array kDBs = {-9.f, -18.f, -36.f, -54.f, -72.f, -120.f};
        static constexpr int kDefaultI = 3;

        static constexpr float getDBFromIndex(const float x) {
            return kDBs[static_cast<size_t>(std::round(x))];
        }
    };

    class PAnalyzerTimeLength : public ChoiceParameters<PAnalyzerTimeLength> {
    public:
        static constexpr auto kID = "analyzer_time_length";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "6 s", "9 s", "12 s", "18 s"
        };
        static constexpr std::array kLength = {6.f, 9.f, 12.f, 18.f};
        static constexpr int kDefaultI = 1;
    };

    class PAnalyzerStereo : public ChoiceParameters<PAnalyzerStereo> {
    public:
        static constexpr auto kID = "analyzer_stereo";
        static constexpr auto kName = "Analyzer Stereo";
        inline static const auto kChoices = juce::StringArray{
            "M+S", "L+R", "Left", "Right", "Mid", "Side"
        };
        static constexpr int kDefaultI = 0;
    };

    class PPreCurveDisplay : public BoolParameters<PPreCurveDisplay> {
    public:
        static constexpr auto kID = "pre_curve_display";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = true;
    };

    class PPostCurveDisplay : public BoolParameters<PPostCurveDisplay> {
    public:
        static constexpr auto kID = "post_curve_display";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = true;
    };

    class PDeltaCurveDisplay : public BoolParameters<PDeltaCurveDisplay> {
    public:
        static constexpr auto kID = "delta_curve_display";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = true;
    };

    class PSideChainCurveDisplay : public BoolParameters<PSideChainCurveDisplay> {
    public:
        static constexpr auto kID = "side_chain_curve_display";
        static constexpr auto kName = "";
        static constexpr auto kDefaultV = false;
    };

    class PSideControlDisplay : public ChoiceParameters<PSideControlDisplay> {
    public:
        static constexpr auto kID = "side_control_display";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "OFF", "ON"
        };
        static constexpr int kDefaultI = 0;
    };

    class PSideEQDisplay : public ChoiceParameters<PSideEQDisplay> {
    public:
        static constexpr auto kID = "side_eq_display";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "OFF", "ON"
        };
        static constexpr int kDefaultI = 0;
    };

    class PComputerCurveDisplay : public ChoiceParameters<PComputerCurveDisplay> {
    public:
        static constexpr auto kID = "computer_curve_display";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "OFF", "ON"
        };
        static constexpr int kDefaultI = 1;
    };

    class PRMSAnalyzerDisplay : public ChoiceParameters<PRMSAnalyzerDisplay> {
    public:
        static constexpr auto kID = "rms_analyzer_display";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "OFF", "ON"
        };
        static constexpr int kDefaultI = 1;
    };

    class PMeterDisplay : public ChoiceParameters<PMeterDisplay> {
    public:
        static constexpr auto kID = "meter_display";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "OFF", "ON"
        };
        static constexpr int kDefaultI = 1;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PEQMaxDB::get(),
                   PAnalyzerMagType::get(), PAnalyzerStereo::get(), PAnalyzerMoveType::get(),
                   PPreCurveDisplay::get(), PPostCurveDisplay::get(),
                   PDeltaCurveDisplay::get(), PSideChainCurveDisplay::get(),
                   PAnalyzerTimeLength::get(), PAnalyzerMaxDB::get(), PAnalyzerMinDB::get(),
                   PSideControlDisplay::get(), PSideEQDisplay::get(),
                   PComputerCurveDisplay::get(),
                   PRMSAnalyzerDisplay::get(), PMeterDisplay::get());
        return layout;
    }

    class PWindowW : public FloatParameters<PWindowW> {
    public:
        static constexpr auto kID = "window_w";
        static constexpr auto kName = "";
        inline static constexpr float minV = 600.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float kDefaultV = 600.f;
        inline static const auto kRange = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class PWindowH : public FloatParameters<PWindowH> {
    public:
        static constexpr auto kID = "window_h";
        static constexpr auto kName = "";
        inline static constexpr float minV = 282.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float kDefaultV = 371.f;
        inline static const auto kRange = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class PWindowSizeFix : public ChoiceParameters<PWindowSizeFix> {
    public:
        static constexpr auto kID = "window_size_fix";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Off", "On"
        };
        static constexpr int kDefaultI = 0;
    };

    class PFontMode : public ChoiceParameters<PFontMode> {
    public:
        static constexpr auto kID = "font_mode";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Scale", "Static"
        };
        static constexpr int kDefaultI = 0;
    };

    class PFontScale : public FloatParameters<PFontScale> {
    public:
        static constexpr auto kID = "font_scale";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(.5f, 1.f, .01f);
        static constexpr auto kDefaultV = .9f;
    };

    class PStaticFontSize : public FloatParameters<PStaticFontSize> {
    public:
        static constexpr auto kID = "static_font_size";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(.1f, 600.f, .01f);
        static constexpr auto kDefaultV = .9f;
    };

    class PWheelSensitivity : public FloatParameters<PWheelSensitivity> {
    public:
        static constexpr auto kID = "wheel_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PWheelFineSensitivity : public FloatParameters<PWheelFineSensitivity> {
    public:
        static constexpr auto kID = "wheel_fine_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = .12f;
    };

    class PWheelShiftReverse : public ChoiceParameters<PWheelShiftReverse> {
    public:
        static constexpr auto kID = "wheel_shift_reverse";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "No Change", "Reverse"
        };
        static constexpr int kDefaultI = 0;
    };

    class PSliderSensitivity : public FloatParameters<PSliderSensitivity> {
    public:
        static constexpr auto kID = "slider_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PSliderFineSensitivity : public FloatParameters<PSliderFineSensitivity> {
    public:
        static constexpr auto kID = "slider_fine_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = .25f;
    };

    class PDraggerSensitivity : public FloatParameters<PDraggerSensitivity> {
    public:
        static constexpr auto kID = "dragger_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PDraggerFineSensitivity : public FloatParameters<PDraggerFineSensitivity> {
    public:
        static constexpr auto kID = "dragger_fine_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = .25f;
    };

    class PWheelComboboxSensitivity : public FloatParameters<PWheelComboboxSensitivity> {
    public:
        static constexpr auto kID = "wheel_combobox_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        static constexpr auto kDefaultV = .5f;
    };

    class PRotaryStyle : public ChoiceParameters<PRotaryStyle> {
    public:
        static constexpr auto kID = "rotary_style";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Circular", "Horizontal", "Vertical", "Horiz + Vert"
        };
        static constexpr int kDefaultI = 3;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class PRotaryDragSensitivity : public FloatParameters<PRotaryDragSensitivity> {
    public:
        static constexpr auto kID = "rotary_drag_sensitivity";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        static constexpr auto kDefaultV = 10.f;
    };

    class PSliderDoubleClickFunc : public ChoiceParameters<PSliderDoubleClickFunc> {
    public:
        static constexpr auto kID = "slider_double_click_func";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Return Default", "Open Editor"
        };
        static constexpr int kDefaultI = 1;
    };

    class PMouseOption {
    public:
        inline static const auto kChoices = juce::StringArray{
            "Left Click", "Right Click", "Left Double Click", "Right Double Click"
        };
    };

    class PKeyOption {
    public:
        inline static const auto kChoices = juce::StringArray{
#if JUCE_MAC
            "None", "Command", "Shift", "Option"
#else
            "None", "Ctrl", "Shift", "Alt"
#endif
        };
    };

    class PEnterSoloMouse : public ChoiceParameters<PEnterSoloMouse> {
    public:
        static constexpr auto kID = "enter_solo_mouse";
        static constexpr auto kName = "";
        inline static const auto kChoices = PMouseOption::kChoices;
        static constexpr int kDefaultI = 1;
    };

    class PEnterSoloKey : public ChoiceParameters<PEnterSoloKey> {
    public:
        static constexpr auto kID = "enter_solo_key";
        static constexpr auto kName = "";
        inline static const auto kChoices = PKeyOption::kChoices;
        static constexpr int kDefaultI = 0;
    };

    class PExitSoloMouse : public ChoiceParameters<PExitSoloMouse> {
    public:
        static constexpr auto kID = "exit_solo_mouse";
        static constexpr auto kName = "";
        inline static const auto kChoices = PMouseOption::kChoices;
        static constexpr int kDefaultI = 1;
    };

    class PExitSoloKey : public ChoiceParameters<PExitSoloKey> {
    public:
        static constexpr auto kID = "exit_solo_key";
        static constexpr auto kName = "";
        inline static const auto kChoices = PKeyOption::kChoices;
        static constexpr int kDefaultI = 0;
    };

    class PTargetRefreshSpeed : public ChoiceParameters<PTargetRefreshSpeed> {
    public:
        static constexpr auto kID = "target_refresh_speed_id";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "120 Hz", "90 Hz", "60 Hz", "30 Hz", "15 Hz"
        };
        static constexpr std::array<double, 5> kRates{120.0, 90.0, 60.0, 30.0, 15.0};
        static constexpr int kDefaultI = 3;
    };

    class PFFTExtraTilt : public FloatParameters<PFFTExtraTilt> {
    public:
        static constexpr auto kID = "fft_extra_tilt";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(-4.5f, 4.5f, .01f);
        static constexpr auto kDefaultV = 0.f;
    };

    class PFFTExtraSpeed : public FloatParameters<PFFTExtraSpeed> {
    public:
        static constexpr auto kID = "fft_extra_speed";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 2.f, .01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PMagCurveThickness : public FloatParameters<PMagCurveThickness> {
    public:
        static constexpr auto kID = "mag_curve_thickness";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PEQCurveThickness : public FloatParameters<PEQCurveThickness> {
    public:
        static constexpr auto kID = "eq_curve_thickness";
        static constexpr auto kName = "";
        inline static const auto kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        static constexpr auto kDefaultV = 1.f;
    };

    class PTooltipLang : public ChoiceParameters<PTooltipLang> {
    public:
        static constexpr auto kID = "tool_tip_lang";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Off",
            "System",
            "English",
            juce::String(juce::CharPointer_UTF8("简体中文")),
            juce::String(juce::CharPointer_UTF8("繁體中文")),
            juce::String(juce::CharPointer_UTF8("Italiano")),
            juce::String(juce::CharPointer_UTF8("日本語")),
            juce::String(juce::CharPointer_UTF8("Deutsch")),
            juce::String(juce::CharPointer_UTF8("Español"))
        };
        static constexpr int kDefaultI = 1;
    };

    class PColourMapIdx : public ChoiceParameters<PColourMapIdx> {
    public:
        static constexpr auto kID = "colour_map_idx";
        static constexpr auto kName = "";
        inline static const auto kChoices = juce::StringArray{
            "Default Light", "Default Dark",
            "Seaborn Normal Light", "Seaborn Normal Dark",
            "Seaborn Bright Light", "Seaborn Bright Dark"
        };

        enum ColourMapName {
            kDefaultLight,
            kDefaultDark,
            kSeabornNormalLight,
            kSeabornNormalDark,
            kSeabornBrightLight,
            kSeabornBrightDark,
            kColourMapNum
        };

        static constexpr int kDefaultI = 0;
    };

    class PColourMap1Idx : public ChoiceParameters<PColourMap1Idx> {
    public:
        static constexpr auto kID = "colour_map_1_idx";
        static constexpr auto kName = "";
        inline static const auto kChoices = PColourMapIdx::kChoices;
        static constexpr int kDefaultI = 1;
    };

    class PColourMap2Idx : public ChoiceParameters<PColourMap2Idx> {
    public:
        static constexpr auto kID = "colour_map_2_idx";
        static constexpr auto kName = "";
        inline static const auto kChoices = PColourMapIdx::kChoices;
        static constexpr int kDefaultI = 5;
    };

    inline void addOneColour(juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                             const std::string& suffix = "",
                             const int red = 0, const int green = 0, const int blue = 0,
                             const bool add_opacity = false, const float opacity = 1.f) {
        layout.add(std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_r", kVersionHint), "",
                       0, 255, red),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_g", kVersionHint), "",
                       0, 255, green),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_b", kVersionHint), "",
                       0, 255, blue));
        if (add_opacity) {
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(suffix + "_o", kVersionHint), "",
                juce::NormalisableRange<float>(0.f, 1.f, .01f), opacity));
        }
    }

    static constexpr std::array<std::string_view, 10> kColourNames{
        "text", "background",
        "shadow", "glow",
        "pre", "post", "reduction", "sidechain",
        "computer", "grid"
    };

    struct ColourDefaultSetting {
        int r, g, b;
        bool has_opacity;
        float opacity;
    };

    static constexpr std::array<ColourDefaultSetting, 10> kColourDefaults{
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, 1.f},
        ColourDefaultSetting{(255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, true, 1.f},
        ColourDefaultSetting{0, 0, 0, true, 1.f},
        ColourDefaultSetting{70, 66, 62, true, 1.f},
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, .25f},
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, 1.f},
        ColourDefaultSetting{252, 18, 197, true, 1.f},
        ColourDefaultSetting{23, 255, 244, true, 1.f},
        ColourDefaultSetting{255, 165, 0, true, 1.f},
        ColourDefaultSetting{255 - 8, 255 - 9, 255 - 11, true, .1f}
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getStateParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PWindowW::get(), PWindowH::get(), PWindowSizeFix::get(),
                   PFontMode::get(), PFontScale::get(), PStaticFontSize::get(),
                   PWheelSensitivity::get(), PWheelFineSensitivity::get(), PWheelShiftReverse::get(),
                   PSliderSensitivity::get(), PSliderFineSensitivity::get(),
                   PDraggerSensitivity::get(), PDraggerFineSensitivity::get(),
                   PWheelComboboxSensitivity::get(),
                   PRotaryStyle::get(), PRotaryDragSensitivity::get(),
                   PSliderDoubleClickFunc::get(),
                   PEnterSoloMouse::get(), PEnterSoloKey::get(),
                   PExitSoloMouse::get(), PExitSoloKey::get(),
                   PTargetRefreshSpeed::get(),
                   PFFTExtraTilt::get(), PFFTExtraSpeed::get(),
                   PMagCurveThickness::get(), PEQCurveThickness::get(),
                   PTooltipLang::get());

        for (size_t i = 0; i < kColourNames.size(); ++i) {
            const auto name = std::string(kColourNames[i]);
            const auto& dv = kColourDefaults[i];
            addOneColour(layout, name, dv.r, dv.g, dv.b, dv.has_opacity, dv.opacity);
        }

        layout.add(PColourMap1Idx::get(), PColourMap2Idx::get());
        return layout;
    }
}
