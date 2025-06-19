// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "button_panel.hpp"

namespace zlpanel {
    ButtonPanel::ButtonPanel(PluginProcessor &processor, zlgui::UIBase &base,
                             size_t &selected_band_idx)
        : p_ref_(processor), base_(base), selected_band_idx_(selected_band_idx),
          eq_max_db_id_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PEQMaxDB::kID)),
          q_slider_(base),
          eq_db_box_(zlstate::PEQMaxDB::kChoices, base),
          eq_db_box_attachment_(eq_db_box_.getBox(), processor.na_parameters_, zlstate::PEQMaxDB::kID, updater_),
          para_panel_(processor, base, selected_band_idx),
          popup_panel_(processor, base, selected_band_idx) {
        addChildComponent(q_slider_);

        const auto popup_option = juce::PopupMenu::Options()
                .withParentComponent(this)
                .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::downwards);
        for (auto &box: {&eq_db_box_}) {
            box->getLAF().setFontScale(1.f);
            box->getLAF().setOption(popup_option);
            box->setAlpha(.5f);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        addChildComponent(para_panel_);

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band] = std::make_unique<DraggerPanel>(
                p_ref_, base_, band, selected_band_idx);
            addChildComponent(*dragger_panels_[band]);
            dragger_panels_[band]->addMouseListener(this, true);
        }
        addChildComponent(popup_panel_);
    }

    void ButtonPanel::resized() { {
            const auto bound = getLocalBounds();
            q_slider_.setBounds(bound);
            para_panel_.setBounds({
                bound.getX(), bound.getY(),
                para_panel_.getIdealWidth(), para_panel_.getIdealHeight()
            });
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                dragger_panels_[band]->setBounds(bound);
            }
            popup_panel_.setBounds({
                bound.getX(), bound.getY(),
                popup_panel_.getIdealWidth(), popup_panel_.getIdealHeight()
            });
        } {
            auto bound = getLocalBounds();
            const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
            bound = bound.removeFromTop(juce::roundToInt(base_.getFontSize() * 1.75f));
            bound.removeFromRight(padding);
            bound = bound.removeFromRight(juce::roundToInt(base_.getFontSize() * 2.5f));
            eq_db_box_.setBounds(bound);
        }
    }

    void ButtonPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        const auto c_eq_max_db_id = eq_max_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(c_eq_max_db_id - eq_max_db_id_) > 1e-3f) {
            eq_max_db_id_ = std::round(c_eq_max_db_id);

            const auto eq_max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(eq_max_db_id_)];
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                dragger_panels_[band]->setEQMaxDB(eq_max_db);
            }
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band]->repaintCallBackSlow();
        }
        para_panel_.repaintCallBackSlow();
        popup_panel_.repaintCallBackSlow();
    }

    void ButtonPanel::updateBand() {
        if (previous_band_idx_ != selected_band_idx_) {
            if (previous_band_idx_ != zlp::kBandNum) {
                dragger_panels_[previous_band_idx_]->getDragger().getButton().setToggleState(
                    false, juce::sendNotificationSync);
            }
            previous_band_idx_ = selected_band_idx_;
            q_attachment_.reset();
            if (selected_band_idx_ != zlp::kBandNum) {
                q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true> >(
                    q_slider_,
                    p_ref_.parameters_, zlp::PQ::kID + std::to_string(selected_band_idx_),
                    updater_);
            }
            para_panel_.updateBand();
            popup_panel_.updateBand();
        }
    }

    void ButtonPanel::setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &status) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto f = status[band] != zlp::EqualizeController::FilterStatus::kOff;
            dragger_panels_[band]->setVisible(f);
            if (f) {
                dragger_panels_[band]->getDragger().getButton().repaint();
            }
        }
    }

    void ButtonPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent != this) { return; }
        selected_band_idx_ = zlp::kBandNum;
    }

    void ButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.originalComponent != this) { return; }
        // find an off band
        size_t band_idx = zlp::kBandNum;
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (p_ref_.parameters_.getRawParameterValue(
                    zlp::PFilterStatus::kID + std::to_string(band))->load(std::memory_order::relaxed) < .1f) {
                band_idx = band;
                break;
            }
        }
        if (band_idx == zlp::kBandNum) {
            return;
        }

        auto bound = getLocalBounds().toFloat();
        bound.reduce(0.f, base_.getFontSize());

        const auto point = event.getPosition().toFloat();
        const auto x = point.getX(), y = point.getY();

        const auto eq_max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(eq_max_db_id_)];
        const auto freq = std::exp(x / bound.getWidth() * std::log(2200.f)) * 10.f;
        const auto gain = -(y - bound.getCentreY()) / bound.getHeight() * 2.f * eq_max_db;

        std::vector<float> init_values;
        init_values.reserve(init_IDs.size());

        if (freq < 20.f) {
            init_values.emplace_back(zlp::PFilterType::convertTo01(zldsp::filter::FilterType::kHighPass));
            init_values.emplace_back(zlp::PGain::convertTo01(0.f));
        } else if (freq < 50.f) {
            init_values.emplace_back(zlp::PFilterType::convertTo01(zldsp::filter::FilterType::kLowShelf));
            init_values.emplace_back(zlp::PGain::convertTo01(2 * gain));
        } else if (freq < 5000.f) {
            init_values.emplace_back(zlp::PFilterType::convertTo01(zldsp::filter::FilterType::kPeak));
            init_values.emplace_back(zlp::PGain::convertTo01(gain));
        } else if (freq < 15000.f) {
            init_values.emplace_back(zlp::PFilterType::convertTo01(zldsp::filter::FilterType::kHighShelf));
            init_values.emplace_back(zlp::PGain::convertTo01(2 * gain));
        } else {
            init_values.emplace_back(zlp::PFilterType::convertTo01(zldsp::filter::FilterType::kLowPass));
            init_values.emplace_back(zlp::PGain::convertTo01(0.f));
        }
        init_values.emplace_back(zlp::PFreq::convertTo01(freq));
        init_values.emplace_back(zlp::PQ::convertTo01(zlp::PQ::kDefaultV));
        init_values.emplace_back(zlp::POrder::convertTo01(zlp::POrder::kDefaultI));
        init_values.emplace_back(zlp::PFilterStatus::convertTo01(zlp::EqualizeController::FilterStatus::kOn));

        const auto suffix = std::to_string(band_idx);

        for (size_t i = 0; i < init_values.size(); ++i) {
            auto *para = p_ref_.parameters_.getParameter(init_IDs[i] + suffix);
            para->beginChangeGesture();
            para->setValueNotifyingHost(init_values[i]);
            para->endChangeGesture();
        }

        dragger_panels_[band_idx]->getDragger().getButton().setToggleState(true, juce::sendNotificationSync);
    }

    void ButtonPanel::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        const juce::MouseEvent e{
            event.source, event.position,
            event.mods.withoutMouseButtons(),
            event.pressure, event.orientation, event.rotation,
            event.tiltX, event.tiltY,
            event.eventComponent, event.originalComponent,
            event.eventTime, event.mouseDownPosition, event.mouseDownTime,
            event.getNumberOfClicks(), false
        };
        q_slider_.mouseWheelMove(e, wheel);
    }
} // zlpanel
