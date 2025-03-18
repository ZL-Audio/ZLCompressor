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

namespace zlDSP {
    inline auto static constexpr versionHint = 1;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(bool meta, const std::string &suffix = "",
                                                              bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::range.convertTo0to1(x);
        }
    };

    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name + suffix, T::defaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(bool meta, const std::string &suffix = "",
                                                             bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name + suffix, T::defaultV, attributes);
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
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(bool meta, const std::string &suffix = "",
                                                               bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::choices.size());
        }
    };

    class learn : public BoolParameters<learn> {
    public:
        auto static constexpr ID = "learn";
        auto static constexpr name = "Learn";
        auto static constexpr defaultV = false;
    };

    class threshold : public FloatParameters<threshold> {
    public:
        auto static constexpr ID = "threshold";
        auto static constexpr name = "Threshold (dB)";
        inline auto static const range =
                juce::NormalisableRange<float>(-80.f, 0.f, .1f);
        auto static constexpr defaultV = -40.f;
    };

    class ratio : public FloatParameters<ratio> {
    public:
        auto static constexpr ID = "ratio";
        auto static constexpr name = "Ratio";
        auto static constexpr defaultV = 1.f;
        inline auto static const range =
                juce::NormalisableRange<float>(1.f, 100.f, 0.01f, 0.2160127f);
    };

    class kneeW : public FloatParameters<kneeW> {
    public:
        auto static constexpr ID = "knee_width";
        auto static constexpr name = "Knee Width";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 1.f, .01f, .5f);
        auto static constexpr defaultV = 0.25f;

        inline static float formatV(const float x) { return std::max(x * 60, .01f); }

        inline static double formatV(const double x) { return std::max(x * 60, 0.01); }
    };

    class curve : public FloatParameters<curve> {
    public:
        auto static constexpr ID = "curve";
        auto static constexpr name = "Curve";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class attack : public FloatParameters<attack> {
    public:
        auto static constexpr ID = "attack";
        auto static constexpr name = "Attack (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 500.f, 0.1f, 0.3010299956639812f);
        auto static constexpr defaultV = 50.f;
    };

    class release : public FloatParameters<release> {
    public:
        auto static constexpr ID = "release";
        auto static constexpr name = "Release (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 5000.f, 0.1f, 0.3010299956639812f);
        auto static constexpr defaultV = 500.f;
    };

    class sideSolo : public BoolParameters<sideSolo> {
    public:
        auto static constexpr ID = "side_solo";
        auto static constexpr name = "Side Solo";
        auto static constexpr defaultV = false;
    };

    class lookahead : public FloatParameters<lookahead> {
    public:
        auto static constexpr ID = "llookahead";
        auto static constexpr name = "Lookahead";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 20.f, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class rms : public FloatParameters<rms> {
    public:
        auto static constexpr ID = "rms";
        auto static constexpr name = "RMS";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 40.f, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class effectON : public ChoiceParameters<effectON> {
    public:
        auto static constexpr ID = "effect_on";
        auto static constexpr name = "Effect ON";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 1;
    };

    class staticAutoGain : public ChoiceParameters<staticAutoGain> {
    public:
        auto static constexpr ID = "static_auto_gain";
        auto static constexpr name = "Auto Gain";
        inline auto static const choices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr defaultI = 0;
    };

    class smooth : public FloatParameters<smooth> {
    public:
        auto static constexpr ID = "smooth";
        auto static constexpr name = "Smooth";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, .01f);
        auto static constexpr defaultV = 0.f;
    };

    class outputGain : public FloatParameters<outputGain> {
    public:
        auto static constexpr ID = "output_gain";
        auto static constexpr name = "Output Gain";
        inline auto static const range =
                juce::NormalisableRange<float>(-16.f, 16.f, .01f, 0.5, true);
        auto static constexpr defaultV = 0.f;
    };

    class sideChain : public BoolParameters<sideChain> {
    public:
        auto static constexpr ID = "side_chain";
        auto static constexpr name = "Side Chain";
        auto static constexpr defaultV = false;
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
