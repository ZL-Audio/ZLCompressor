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

namespace zlp {
    inline auto static constexpr kVersionHint = 1;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(bool meta, const std::string &suffix = "",
                                                              bool automate = true) {
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
        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(bool meta, const std::string &suffix = "",
                                                             bool automate = true) {
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
        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::choices, T::kDefaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(bool meta, const std::string &suffix = "",
                                                               bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::choices, T::kDefaultI, attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::choices.size());
        }
    };

    class learn : public BoolParameters<learn> {
    public:
        auto static constexpr kID = "learn";
        auto static constexpr kName = "Learn";
        auto static constexpr kDefaultV = false;
    };

    class threshold : public FloatParameters<threshold> {
    public:
        auto static constexpr kID = "threshold";
        auto static constexpr kName = "Threshold (dB)";
        inline auto static const kRange =
                juce::NormalisableRange<float>(-80.f, 0.f, .1f);
        auto static constexpr kDefaultV = -40.f;
    };

    class ratio : public FloatParameters<ratio> {
    public:
        auto static constexpr kID = "ratio";
        auto static constexpr kName = "Ratio";
        auto static constexpr kDefaultV = 1.f;
        inline auto static const kRange =
                juce::NormalisableRange<float>(1.f, 100.f, 0.01f, 0.2160127f);
    };

    class kneeW : public FloatParameters<kneeW> {
    public:
        auto static constexpr kID = "knee_width";
        auto static constexpr kName = "Knee Width";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 1.f, .01f, .5f);
        auto static constexpr kDefaultV = 0.25f;

        inline static float formatV(const float x) { return std::max(x * 60, .01f); }

        inline static double formatV(const double x) { return std::max(x * 60, 0.01); }
    };

    class curve : public FloatParameters<curve> {
    public:
        auto static constexpr kID = "curve";
        auto static constexpr kName = "Curve";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class attack : public FloatParameters<attack> {
    public:
        auto static constexpr kID = "attack";
        auto static constexpr kName = "Attack (ms)";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 500.f, 0.1f, 0.3010299956639812f);
        auto static constexpr kDefaultV = 50.f;
    };

    class release : public FloatParameters<release> {
    public:
        auto static constexpr kID = "release";
        auto static constexpr kName = "Release (ms)";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 5000.f, 0.1f, 0.3010299956639812f);
        auto static constexpr kDefaultV = 500.f;
    };

    class sideSolo : public BoolParameters<sideSolo> {
    public:
        auto static constexpr kID = "side_solo";
        auto static constexpr kName = "Side Solo";
        auto static constexpr kDefaultV = false;
    };

    class lookahead : public FloatParameters<lookahead> {
    public:
        auto static constexpr kID = "llookahead";
        auto static constexpr kName = "Lookahead";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 20.f, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class rms : public FloatParameters<rms> {
    public:
        auto static constexpr kID = "rms";
        auto static constexpr kName = "RMS";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 40.f, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class effectON : public ChoiceParameters<effectON> {
    public:
        auto static constexpr kID = "effect_on";
        auto static constexpr kName = "Effect ON";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr kDefaultI = 1;
    };

    class staticAutoGain : public ChoiceParameters<staticAutoGain> {
    public:
        auto static constexpr kID = "static_auto_gain";
        auto static constexpr kName = "Auto Gain";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    class smooth : public FloatParameters<smooth> {
    public:
        auto static constexpr kID = "smooth";
        auto static constexpr kName = "Smooth";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 1.f, .01f);
        auto static constexpr kDefaultV = 0.f;
    };

    class outputGain : public FloatParameters<outputGain> {
    public:
        auto static constexpr kID = "output_gain";
        auto static constexpr kName = "Output Gain";
        inline auto static const kRange =
                juce::NormalisableRange<float>(-16.f, 16.f, .01f, 0.5, true);
        auto static constexpr kDefaultV = 0.f;
    };

    class sideChain : public BoolParameters<sideChain> {
    public:
        auto static constexpr kID = "side_chain";
        auto static constexpr kName = "Side Chain";
        auto static constexpr kDefaultV = false;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(threshold::get(), ratio::get(), kneeW::get(), curve::get(),
                   attack::get(), release::get());
        return layout;
    }

    inline void updateParaNotifyHost(juce::RangedAudioParameter *para, float value) {
        para->beginChangeGesture();
        para->setValueNotifyingHost(value);
        para->endChangeGesture();
    }
}
