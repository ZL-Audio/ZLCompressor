// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "compressor_controller.hpp"

namespace zlp {
    inline auto static constexpr kVersionHint = 1;

    template<typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRange(
        const FloatType xMin, const FloatType xMax, const FloatType xMid, const FloatType xInterval) {
        const FloatType rng1{std::log(xMid / xMin) * FloatType(2)};
        const FloatType rng2{std::log(xMax / xMid) * FloatType(2)};
        return {
            xMin, xMax,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5) ? std::exp(v * rng1) * xMin : std::exp((v - FloatType(.5)) * rng2) * xMid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < xMid ? std::log(v / xMin) / rng1 : FloatType(.5) + std::log(v / xMid) / rng2;
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = xMin + xInterval * std::round((v - xMin) / xInterval);
                return x <= xMin ? xMin : (x >= xMax ? xMax : x);
            }
        };
    }

    inline juce::NormalisableRange<double> logMidRange(
        const double xMin, const double xMax, const double xMid, const double xInterval) {
        return getLogMidRange<double>(xMin, xMax, xMid, xInterval);
    }

    inline juce::NormalisableRange<float> logMidRange(
        const float xMin, const float xMax, const float xMid, const float xInterval) {
        return getLogMidRange<float>(xMin, xMax, xMid, xInterval);
    }

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
            return static_cast<float>(x) / static_cast<float>(T::choices.size());
        }
    };

    class PCompStyle : public ChoiceParameters<PCompStyle> {
    public:
        auto static constexpr kID = "comp_style";
        auto static constexpr kName = "Compressor Style";
        inline auto static const kChoices = juce::StringArray{
            "Clean", "Classic", "Optical"
        };
        int static constexpr kDefaultI = 0;
    };

    class PThreshold : public FloatParameters<PThreshold> {
    public:
        auto static constexpr kID = "threshold";
        auto static constexpr kName = "Threshold (dB)";
        inline auto static const kRange =
                juce::NormalisableRange<float>(-80.f, 0.f, .1f);
        auto static constexpr kDefaultV = -18.f;
    };

    class PRatio : public FloatParameters<PRatio> {
    public:
        auto static constexpr kID = "ratio";
        auto static constexpr kName = "Ratio";
        auto static constexpr kDefaultV = 4.f;
        inline auto static const kRange = getLogMidRange(1.f, 100.f, 4.f, 0.01f);
                // juce::NormalisableRange<float>(1.f, 100.f, 0.01f, 0.2160127f);
    };

    class PKneeW : public FloatParameters<PKneeW> {
    public:
        auto static constexpr kID = "knee_width";
        auto static constexpr kName = "Knee Width";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 32.f, .01f, .5f);
        auto static constexpr kDefaultV = 8.f;
    };

    class PCurve : public FloatParameters<PCurve> {
    public:
        auto static constexpr kID = "curve";
        auto static constexpr kName = "Curve";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;

        template<typename FloatType>
        inline static FloatType formatV(const FloatType x) {
            return x * FloatType(0.02) - FloatType(1);
        }
    };

    class PAttack : public FloatParameters<PAttack> {
    public:
        auto static constexpr kID = "attack";
        auto static constexpr kName = "Attack (ms)";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 500.f, 0.01f, 0.3010299956639812f);
        auto static constexpr kDefaultV = 50.f;
    };

    class PRelease : public FloatParameters<PRelease> {
    public:
        auto static constexpr kID = "release";
        auto static constexpr kName = "Release (ms)";
        inline auto static const kRange =
                juce::NormalisableRange<float>(0.f, 5000.f, 0.01f, 0.3010299956639812f);
        auto static constexpr kDefaultV = 500.f;
    };

    class PPump : public FloatParameters<PPump> {
    public:
        auto static constexpr kID = "pump";
        auto static constexpr kName = "Pump";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .01f);
        auto static constexpr kDefaultV = 0.f;

        template<typename FloatType>
        inline static FloatType formatV(const FloatType x) {
            return x * FloatType(0.01);
        }
    };

    class PSmooth : public FloatParameters<PSmooth> {
    public:
        auto static constexpr kID = "smooth";
        auto static constexpr kName = "Smooth";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .01f);
        auto static constexpr kDefaultV = 0.f;

        template<typename FloatType>
        inline static FloatType formatV(const FloatType x) {
            return x * FloatType(0.01);
        }
    };

    class POversample : public ChoiceParameters<POversample> {
    public:
        auto static constexpr kID = "oversample";
        auto static constexpr kName = "Oversample";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "2x", "4x", "8x"
        };
        int static constexpr kDefaultI = 0;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PCompStyle::get(),
                   PThreshold::get(), PRatio::get(), PKneeW::get(), PCurve::get(),
                   PAttack::get(), PRelease::get(), PPump::get(), PSmooth::get(),
                   POversample::get());
        return layout;
    }

    inline void updateParaNotifyHost(juce::RangedAudioParameter *para, float value) {
        para->beginChangeGesture();
        para->setValueNotifyingHost(value);
        para->endChangeGesture();
    }
}
