// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <BinaryData.h>

namespace zlstate {
    inline static constexpr int kVersionHint = 1;

    inline static constexpr size_t kBandNUM = 8;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID, kVersionHint),
                                                               T::kName, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix, const bool meta,
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
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID, kVersionHint),
                                                              T::kName, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix, bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix, const bool meta,
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
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID, kVersionHint),
                                                                T::kName, T::kChoices, T::kDefaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix, const bool meta,
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
        auto static constexpr kID = "eq_max_db";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "6 dB", "12 dB", "30 dB"
        };
        static constexpr std::array dBs = {6.f, 12.f, 30.f};
        int static constexpr kDefaultI = 1;
    };

    class PAnalyzerMagType : public ChoiceParameters<PAnalyzerMagType> {
    public:
        auto static constexpr kID = "analyzer_mag_type";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Peak", "RMS"
        };
        int static constexpr kDefaultI = 0;
    };

    class PAnalyzerMinDB : public ChoiceParameters<PAnalyzerMinDB> {
    public:
        auto static constexpr kID = "analyzer_min_db";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "-9", "-18", "-36", "-54", "-72"
        };
        static constexpr std::array kDBs = {-9.f, -18.f, -36.f, -54.f, -72.f};
        int static constexpr kDefaultI = 3;

        static float getMinDBFromIndex(const float x) {
            return kDBs[static_cast<size_t>(std::round(x))];
        }
    };

    class PAnalyzerTimeLength : public ChoiceParameters<PAnalyzerTimeLength> {
    public:
        auto static constexpr kID = "analyzer_time_length";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "6 s ", "9 s ", "12 s ", "18 s "
        };
        static constexpr std::array kLength = {6.f, 9.f, 12.f, 18.f};
        int static constexpr kDefaultI = 0;

        static float getTimeLengthFromIndex(const float x) {
            return kLength[static_cast<size_t>(std::round(x))];
        }
    };

    class PSideControlDisplay : public ChoiceParameters<PSideControlDisplay> {
    public:
        auto static constexpr kID = "side_control_display";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    class PSideEQDisplay : public ChoiceParameters<PSideEQDisplay> {
    public:
        auto static constexpr kID = "side_eq_display";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getNAParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PEQMaxDB::get(),
                   PAnalyzerMagType::get(), PAnalyzerMinDB::get(), PAnalyzerTimeLength::get(),
                   PSideControlDisplay::get(), PSideEQDisplay::get());
        return layout;
    }

    inline std::string appendSuffix(const std::string &s, const size_t i) {
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        return s + suffix;
    }

    class PWindowW : public FloatParameters<PWindowW> {
    public:
        auto static constexpr kID = "window_w";
        auto static constexpr kName = "";
        inline static constexpr float minV = 600.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float kDefaultV = 704.f;
        inline auto static const kRange = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class PWindowH : public FloatParameters<PWindowH> {
    public:
        auto static constexpr kID = "window_h";
        auto static constexpr kName = "";
        inline static constexpr float minV = 282.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float kDefaultV = 440.f;
        inline auto static const kRange = juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class PWheelSensitivity : public FloatParameters<PWheelSensitivity> {
    public:
        auto static constexpr kID = "wheel_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PWheelFineSensitivity : public FloatParameters<PWheelFineSensitivity> {
    public:
        auto static constexpr kID = "wheel_fine_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        auto static constexpr kDefaultV = .12f;
    };

    class PWheelShiftReverse : public ChoiceParameters<PWheelShiftReverse> {
    public:
        auto static constexpr kID = "wheel_shift_reverse";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "No Change", "Reverse"
        };
        int static constexpr kDefaultI = 0;
    };

    class PDragSensitivity : public FloatParameters<PDragSensitivity> {
    public:
        auto static constexpr kID = "drag_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PDragFineSensitivity : public FloatParameters<PDragFineSensitivity> {
    public:
        auto static constexpr kID = "drag_fine_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.01f, 1.f, 0.01f);
        auto static constexpr kDefaultV = .25f;
    };

    class PRotaryStyle : public ChoiceParameters<PRotaryStyle> {
    public:
        auto static constexpr kID = "rotary_style";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Circular", "Horizontal", "Vertical", "Horiz + Vert"
        };
        int static constexpr kDefaultI = 3;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class PRotaryDragSensitivity : public FloatParameters<PRotaryDragSensitivity> {
    public:
        auto static constexpr kID = "rotary_drag_sensitivity";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        auto static constexpr kDefaultV = 10.f;
    };

    class PSliderDoubleClickFunc : public ChoiceParameters<PSliderDoubleClickFunc> {
    public:
        auto static constexpr kID = "slider_double_click_func";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "Return Default", "Open Editor"
        };
        int static constexpr kDefaultI = 1;
    };

    class PTargetRefreshSpeed : public ChoiceParameters<PTargetRefreshSpeed> {
    public:
        auto static constexpr kID = "target_refresh_speed_id";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
            "120Hz", "90Hz", "60Hz", "30Hz", "15Hz"
        };
        static constexpr std::array<double, 5> kRates{120.0, 90.0, 60.0, 30.0, 15.0};
#if defined(JUCE_MAC)
        int static constexpr kDefaultI = 2;
#else
        int static constexpr kDefaultI = 3;
#endif
    };

    class PFFTExtraTilt : public FloatParameters<PFFTExtraTilt> {
    public:
        auto static constexpr kID = "fft_extra_tilt";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(-4.5f, 4.5f, .01f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PFFTExtraSpeed : public FloatParameters<PFFTExtraSpeed> {
    public:
        auto static constexpr kID = "fft_extra_speed";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 2.f, .01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PSingleCurveThickness : public FloatParameters<PSingleCurveThickness> {
    public:
        auto static constexpr kID = "single_curve_thickness";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PSumCurveThickness : public FloatParameters<PSumCurveThickness> {
    public:
        auto static constexpr kID = "sum_curve_thickness";
        auto static constexpr kName = "";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 4.f, .01f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PColourMapIdx : public ChoiceParameters<PColourMapIdx> {
    public:
        auto static constexpr kID = "colour_map_idx";
        auto static constexpr kName = "";
        inline auto static const kChoices = juce::StringArray{
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

        int static constexpr kDefaultI = 0;
    };

    class PColourMap1Idx : public ChoiceParameters<PColourMap1Idx> {
    public:
        auto static constexpr kID = "colour_map_1_idx";
        auto static constexpr kName = "";
        inline auto static const kChoices = PColourMapIdx::kChoices;
        int static constexpr kDefaultI = 1;
    };

    class PColourMap2Idx : public ChoiceParameters<PColourMap2Idx> {
    public:
        auto static constexpr kID = "colour_map_2_idx";
        auto static constexpr kName = "";
        inline auto static const kChoices = PColourMapIdx::kChoices;
        int static constexpr kDefaultI = 5;
    };

    inline void addOneColour(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                             const std::string &suffix = "",
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

    inline juce::AudioProcessorValueTreeState::ParameterLayout getStateParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PWindowW::get(), PWindowH::get(),
                   PWheelSensitivity::get(), PWheelFineSensitivity::get(), PWheelShiftReverse::get(),
                   PDragSensitivity::get(), PDragFineSensitivity::get(),
                   PRotaryStyle::get(), PRotaryDragSensitivity::get(),
                   PSliderDoubleClickFunc::get(),
                   PTargetRefreshSpeed::get(),
                   PFFTExtraTilt::get(), PFFTExtraSpeed::get(),
                   PSingleCurveThickness::get(), PSumCurveThickness::get());
        addOneColour(layout, "text", 255 - 8, 255 - 9, 255 - 11, true, 1.f);
        addOneColour(layout, "background", (255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, true, 1.f);
        addOneColour(layout, "shadow", 0, 0, 0, true, 1.f);
        addOneColour(layout, "glow", 70, 66, 62, true, 1.f);

        addOneColour(layout, "pre", 255 - 8, 255 - 9, 255 - 11, true, 0.1f);
        addOneColour(layout, "post", 255 - 8, 255 - 9, 255 - 11, true, 0.1f);
        addOneColour(layout, "reduction", 252, 18, 197, true, 0.1f);
        addOneColour(layout, "computer", 255, 165, 0, true, 1.f);

        layout.add(PColourMap1Idx::get(), PColourMap2Idx::get());
        return layout;
    }
}
