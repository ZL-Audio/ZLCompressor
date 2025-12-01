// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "compress_controller.hpp"

namespace zlp {
    inline static constexpr int kVersionHint = 1;

    inline static constexpr size_t kBandNum = 8;

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRange(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid, const FloatType x_interval) {
        const FloatType rng1{std::log(x_mid / x_min) * FloatType(2)};
        const FloatType rng2{std::log(x_max / x_mid) * FloatType(2)};
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min, x_max,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5) ? std::exp(v * rng1) * x_min : std::exp((v - FloatType(.5)) * rng2) * x_mid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < x_mid ? std::log(v / x_min) / rng1 : FloatType(.5) + std::log(v / x_mid) / rng2;
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = x_min + x_interval * std::round((v - x_min) / x_interval);
                return x <= x_min ? x_min : (x >= x_max ? x_max : x);
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRangeShift(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid,
        const FloatType x_interval, const FloatType shift) {
        const auto range = getLogMidRange<FloatType>(x_min, x_max, x_mid, x_interval);
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min + shift, x_max + shift,
            [=](FloatType, FloatType, const FloatType v) {
                return range.convertFrom0to1(v) + shift;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return range.convertTo0to1(v - shift);
            },
            [=](FloatType, FloatType, const FloatType v) {
                return range.snapToLegalValue(v - shift) + shift;
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getSymmetricLogMidRangeShift(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid,
        const FloatType x_interval, const FloatType shift) {
        const auto range = getLogMidRangeShift<FloatType>(x_min, x_max, x_mid, x_interval, shift);
        auto result_range = juce::NormalisableRange<FloatType>{
            -(x_max + shift), x_max + shift,
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0.5)) {
                    return range.convertFrom0to1(v * FloatType(2) - FloatType(1));
                }
                else {
                    return -range.convertFrom0to1(FloatType(1) - v * FloatType(2));
                }
            },
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0)) {
                    return range.convertTo0to1(v) * FloatType(0.5) + FloatType(0.5);
                }
                else {
                    return FloatType(0.5) - range.convertTo0to1(-v) * FloatType(0.5);
                }
            },
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0)) {
                    return range.snapToLegalValue(v);
                }
                else {
                    return -range.snapToLegalValue(-v);
                }
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLinearMidRange(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid, const FloatType x_interval) {
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min, x_max,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5)
                           ? FloatType(2) * v * (x_mid - x_min) + x_min
                           : FloatType(2) * (v - FloatType(0.5)) * (x_max - x_mid) + x_mid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < x_mid
                           ? FloatType(.5) * (v - x_min) / (x_mid - x_min)
                           : FloatType(.5) + FloatType(.5) * (v - x_mid) / (x_max - x_mid);
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = x_min + x_interval * std::round((v - x_min) / x_interval);
                return x <= x_min ? x_min : (x >= x_max ? x_max : x);
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

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

    class PCompStyle : public ChoiceParameters<PCompStyle> {
    public:
        auto static constexpr kID = "comp_style";
        auto static constexpr kName = "Compressor Style";
        inline auto static const kChoices = juce::StringArray{
            "Clean", "Classic", "Optical", "Vocal"
        };
        int static constexpr kDefaultI = 0;
    };

    class PCompDirection : public ChoiceParameters<PCompDirection> {
    public:
        auto static constexpr kID = "comp_direction";
        auto static constexpr kName = "Compression Direction";
        inline auto static const kChoices = juce::StringArray{
            "Down", "Up"
        };
        int static constexpr kDefaultI = 0;
    };

    class PThreshold : public FloatParameters<PThreshold> {
    public:
        auto static constexpr kID = "threshold";
        auto static constexpr kName = "Threshold (dB)";
        inline auto static const kRange = getLinearMidRange(-80.f, 0.f, -18.f, 0.1f);
        auto static constexpr kDefaultV = -18.f;
    };

    class PRatio : public FloatParameters<PRatio> {
    public:
        auto static constexpr kID = "ratio";
        auto static constexpr kName = "Ratio";
        auto static constexpr kDefaultV = 3.f;
        inline auto static const kRange = getLogMidRange(1.f, 100.f, 3.f, 0.01f);
    };

    class PKneeW : public FloatParameters<PKneeW> {
    public:
        auto static constexpr kID = "knee_width";
        auto static constexpr kName = "Knee Width";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 32.f, .01f, .5f);
        auto static constexpr kDefaultV = 8.f;
    };

    class PCurve : public FloatParameters<PCurve> {
    public:
        auto static constexpr kID = "curve";
        auto static constexpr kName = "Curve";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;

        template <typename FloatType>
        inline static FloatType formatV(const FloatType x) {
            return x * FloatType(0.02) - FloatType(1);
        }
    };

    class PAttack : public FloatParameters<PAttack> {
    public:
        auto static constexpr kID = "attack";
        auto static constexpr kName = "Attack";
        inline auto static const kRange = getLogMidRangeShift(20.f, 1020.f, 120.f, 0.01f, -20.f);
        auto static constexpr kDefaultV = 100.f;
    };

    class PRelease : public FloatParameters<PRelease> {
    public:
        auto static constexpr kID = "release";
        auto static constexpr kName = "Release";
        inline auto static const kRange = getLogMidRangeShift(100.f, 5100.f, 600.f, 0.01f, -100.f);
        auto static constexpr kDefaultV = 500.f;
    };

    class PPump : public FloatParameters<PPump> {
    public:
        auto static constexpr kID = "pump";
        auto static constexpr kName = "Pump";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 0.f;

        template <typename FloatType>
        inline static FloatType formatV(const FloatType x) {
            return x * FloatType(0.01);
        }
    };

    class PSmooth : public FloatParameters<PSmooth> {
    public:
        auto static constexpr kID = "smooth";
        auto static constexpr kName = "Smooth";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 0.f;

        template <typename FloatType>
        inline static FloatType formatV(const FloatType x) {
            return x * FloatType(0.01);
        }
    };

    class PRange : public FloatParameters<PRange> {
    public:
        auto static constexpr kID = "range";
        auto static constexpr kName = "Range";
        inline auto static const kRange = getLinearMidRange(0.f, 80.f, 18.f, .1f);
        auto static constexpr kDefaultV = 80.f;
    };

    class PHold : public FloatParameters<PHold> {
    public:
        auto static constexpr kID = "hold";
        auto static constexpr kName = "Hold";
        inline auto static const kRange = getLogMidRangeShift(20.f, 1020.f, 120.f, 0.1f, -20.f);
        auto static constexpr kDefaultV = 0.f;
    };

    class POutGain : public FloatParameters<POutGain> {
    public:
        auto static constexpr kID = "makeup";
        auto static constexpr kName = "Makeup Gain";
        inline auto static const kRange = getLogMidRangeShift(2.f, 32.f, 12.f, 0.01f, -2.f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PWet : public FloatParameters<PWet> {
    public:
        auto static constexpr kID = "wet";
        auto static constexpr kName = "Wet";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 100.f;
    };

    class PExtSide : public BoolParameters<PExtSide> {
    public:
        auto static constexpr kID = "external_side";
        auto static constexpr kName = "External Side";
        auto static constexpr kDefaultV = false;
    };

    class PSideOut : public BoolParameters<PSideOut> {
    public:
        auto static constexpr kID = "side_out";
        auto static constexpr kName = "Side Out";
        auto static constexpr kDefaultV = false;
    };

    class PSideGain : public FloatParameters<PSideGain> {
    public:
        auto static constexpr kID = "side gain";
        auto static constexpr kName = "Side Gain";
        inline auto static const kRange = getSymmetricLogMidRangeShift(2.f, 17.f, 12.f, 0.01f, -2.f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PSideStereoMode : public ChoiceParameters<PSideStereoMode> {
    public:
        auto static constexpr kID = "side_stereo_mode";
        auto static constexpr kName = "Side Stereo Mode";
        inline auto static const kChoices = juce::StringArray{
            "Mid Side", "Left Right", "Mis Side Max", "Left Right Max"
        };
        int static constexpr kDefaultI = 0;
    };

    class PSideStereoSwap : public ChoiceParameters<PSideStereoSwap> {
    public:
        auto static constexpr kID = "side_stereo_swap";
        auto static constexpr kName = "Side Stereo Swap";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    class PSideStereoLink : public FloatParameters<PSideStereoLink> {
    public:
        auto static constexpr kID = "side_stereo_link";
        auto static constexpr kName = "Side Stereo Link";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PSideStereoWet1 : public FloatParameters<PSideStereoWet1> {
    public:
        auto static constexpr kID = "side_stereo_wet1";
        auto static constexpr kName = "Side Stereo Wet 1";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 100.f;
    };

    class PSideStereoWet2 : public FloatParameters<PSideStereoWet2> {
    public:
        auto static constexpr kID = "side_stereo_wet2";
        auto static constexpr kName = "Side Stereo Wet 2";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 100.f;
    };

    class PCompON : public ChoiceParameters<PCompON> {
    public:
        auto static constexpr kID = "comp_on";
        auto static constexpr kName = "Compressor ON";
        inline auto static const kChoices = juce::StringArray{
            "Bypass", "ON"
        };
        int static constexpr kDefaultI = 1;
    };

    class PCompDelta : public ChoiceParameters<PCompDelta> {
    public:
        auto static constexpr kID = "comp_delta";
        auto static constexpr kName = "Compressor Delta";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    class PClipperDrive : public FloatParameters<PClipperDrive> {
    public:
        auto static constexpr kID = "clipper_drive";
        auto static constexpr kName = "Clipper Drive";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 0.f;
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

    class PLookAhead : public FloatParameters<PLookAhead> {
    public:
        auto static constexpr kID = "lookahead";
        auto static constexpr kName = "Lookahead";
        inline auto static const kRange = getLogMidRangeShift(2.f, 22.f, 7.f, 0.01f, -2.f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PRMSON : public BoolParameters<PRMSON> {
    public:
        auto static constexpr kID = "rms_on";
        auto static constexpr kName = "RMS ON";
        auto static constexpr kDefaultV = false;
    };

    class PRMSLength : public FloatParameters<PRMSLength> {
    public:
        auto static constexpr kID = "rms_length";
        auto static constexpr kName = "RMS Length";
        inline auto static const kRange = getLogMidRangeShift(4.f, 44.f, 14.f, 0.01f, -4.f);
        auto static constexpr kDefaultV = 10.f;
    };

    class PRMSSpeed : public FloatParameters<PRMSSpeed> {
    public:
        auto static constexpr kID = "rms_speed";
        auto static constexpr kName = "RMS Speed";
        inline auto static const kRange = getLogMidRangeShift(1.f, 9.f, 2.f, 0.01f, -1.f);
        auto static constexpr kDefaultV = 1.f;
    };

    class PRMSMix : public FloatParameters<PRMSMix> {
    public:
        auto static constexpr kID = "rms_mix";
        auto static constexpr kName = "RMS Mix";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;
    };

    class PFilterStatus : public ChoiceParameters<PFilterStatus> {
    public:
        auto static constexpr kID = "filter_status";
        auto static constexpr kName = "Filter Status";
        inline auto static const kChoices = juce::StringArray{
            "OFF", "Bypass", "ON"
        };
        int static constexpr kDefaultI = 0;
    };

    class PFilterType : public ChoiceParameters<PFilterType> {
    public:
        auto static constexpr kID = "filter_type";
        auto static constexpr kName = "Filter Type";
        inline auto static const kChoices = juce::StringArray{
            "Peak", "Low Shelf", "Low Pass",
            "High Shelf", "High Pass", "Notch",
            "Band Pass", "Tilt Shelf"
        };
        int static constexpr kDefaultI = 0;
    };

    class POrder : public ChoiceParameters<POrder> {
    public:
        auto static constexpr kID = "order";
        auto static constexpr kName = "Order";
        inline auto static const kChoices = juce::StringArray{
            "6 dB/oct", "12 dB/oct", "24 dB/oct", "48 dB/oct", "72 dB/oct", "96 dB/oct"
        };
        int static constexpr kDefaultI = 1;
        static constexpr std::array<size_t, 7> kOrderArray{1, 2, 4, 8, 12, 16};

        static size_t convertToIdx(const size_t order) {
            switch (order) {
            case 1: return 0;
            case 2: return 1;
            case 4: return 2;
            case 6: return 3;
            case 8: return 4;
            case 12: return 5;
            case 16: return 6;
            default: return 0;
            }
        }
    };

    class PFreq : public FloatParameters<PFreq> {
    public:
        auto static constexpr kID = "freq";
        auto static constexpr kName = "Freq";
        inline auto static const kRange = getLogMidRange(10.f, 20000.f, 1000.f, 0.1f);
        auto static constexpr kDefaultV = 1000.f;
    };

    class PGain : public FloatParameters<PGain> {
    public:
        auto static constexpr kID = "gain";
        auto static constexpr kName = "Gain";
        inline auto static const kRange = juce::NormalisableRange<float>(-30, 30, .01f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PQ : public FloatParameters<PQ> {
    public:
        auto static constexpr kID = "Q";
        auto static constexpr kName = "Q";
        inline auto static const kRange = getLogMidRange(0.025f, 25.f, 0.707f, 0.001f);
        auto static constexpr kDefaultV = 0.707f;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PCompStyle::get(), PCompDirection::get(),
                   PThreshold::get(), PRatio::get(), PKneeW::get(), PCurve::get(),
                   PAttack::get(), PRelease::get(), PPump::get(), PSmooth::get(),
                   PHold::get(), PRange::get(), PWet::get(), POutGain::get(),
                   PExtSide::get(), PSideOut::get(), PSideGain::get(),
                   PSideStereoMode::get(), PSideStereoSwap::get(),
                   PSideStereoLink::get(), PSideStereoWet1::get(), PSideStereoWet2::get(),
                   PCompON::get(), PCompDelta::get(),
                   PClipperDrive::get(),
                   POversample::get(), PLookAhead::get(),
                   PRMSON::get(), PRMSLength::get(), PRMSSpeed::get(), PRMSMix::get());
        for (size_t i = 0; i < kBandNum; ++i) {
            const auto suffix = std::to_string(i);
            layout.add(PFilterStatus::get(suffix), PFilterType::get(suffix), POrder::get(suffix),
                       PFreq::get(suffix), PGain::get(suffix), PQ::get(suffix));
        }
        return layout;
    }

    inline void updateParaNotifyHost(juce::RangedAudioParameter* para, const float value) {
        para->beginChangeGesture();
        para->setValueNotifyingHost(value);
        para->endChangeGesture();
    }
}
