// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "filter_para_panel.hpp"

namespace zlpanel {
    FilterParaPanel::FilterParaPanel(PluginProcessor &processor, zlgui::UIBase &base, size_t &selected_band_idx)
        : p_ref_(processor), base_(base), selected_band_idx_(selected_band_idx),
          label_laf_(base),
          freq_label_("", "Freq "),
          gain_label_("", "Gain "),
          freq_slider_("", base, ""),
          gain_slider_("", base, "") {
        label_laf_.setFontScale(1.25f);
        for (auto &l: {&freq_label_, &gain_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centredRight);
            l->setJustificationType(juce::Justification::centred);
            l->setBufferedToImage(true);
        }
        addAndMakeVisible(freq_label_);
        addChildComponent(gain_label_);

        for (auto &s: {&freq_slider_, &gain_slider_}) {
            s->setFontScale(1.25f);
            s->setJustification(juce::Justification::centredLeft);
            s->getSlider().setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s->getSlider().setSliderSnapsToMousePosition(false);
            s->setBufferedToImage(true);
        }
        addAndMakeVisible(freq_slider_);
        addChildComponent(gain_slider_);

        setInterceptsMouseClicks(false, true);
    }

    int FilterParaPanel::getIdealWidth() const {
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale * .5f);
        return 2 * slider_width;
    }

    int FilterParaPanel::getIdealHeight() const {
        const auto button_height = juce::roundToInt(base_.getFontSize() * kButtonScale * .66f);
        return 2 * button_height;
    }

    void FilterParaPanel::resized() {
        auto bound = getLocalBounds();
        auto t_bound = bound.removeFromTop(bound.getHeight() / 2);
        freq_label_.setBounds(t_bound.removeFromLeft(t_bound.getWidth() / 2));
        freq_slider_.setBounds(t_bound);
        gain_label_.setBounds(bound.removeFromLeft(bound.getWidth() / 2));
        gain_slider_.setBounds(bound);
    }

    void FilterParaPanel::updateBand() {
        c_ftype_ = -1.f;
        freq_attachment_.reset();
        gain_attachment_.reset();
        if (selected_band_idx_ != zlp::kBandNum) {
            freq_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true> >(
                freq_slider_.getSlider(), p_ref_.parameters_,
                zlp::PFreq::kID + std::to_string(selected_band_idx_), updater_);
            gain_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true> >(
                gain_slider_.getSlider(), p_ref_.parameters_,
                zlp::PGain::kID + std::to_string(selected_band_idx_), updater_);
            updater_.updateComponents();
            ftype_ref_ = p_ref_.parameters_.getRawParameterValue(
                zlp::PFilterType::kID + std::to_string(selected_band_idx_));
            setVisible(true);
        } else {
            ftype_ref_ = nullptr;
            setVisible(false);
        }
    }

    void FilterParaPanel::repaintCallBackSlow() {
        if (ftype_ref_ != nullptr) {
            if (std::abs(c_ftype_ - ftype_ref_->load(std::memory_order::relaxed)) > 0.01f) {
                c_ftype_ = ftype_ref_->load(std::memory_order::relaxed);
                switch (static_cast<zldsp::filter::FilterType>(std::round(c_ftype_))) {
                    case zldsp::filter::FilterType::kPeak:
                    case zldsp::filter::FilterType::kLowShelf:
                    case zldsp::filter::FilterType::kHighShelf:
                    case zldsp::filter::FilterType::kTiltShelf:
                    case zldsp::filter::FilterType::kBandShelf: {
                        gain_label_.setVisible(true);
                        gain_slider_.setVisible(true);
                        break;
                    }
                    case zldsp::filter::FilterType::kLowPass:
                    case zldsp::filter::FilterType::kHighPass:
                    case zldsp::filter::FilterType::kNotch:
                    case zldsp::filter::FilterType::kBandPass: {
                        gain_label_.setVisible(false);
                        gain_slider_.setVisible(false);
                        break;
                    }
                }
            }
        }
        updater_.updateComponents();
    }
} // zlpanel
