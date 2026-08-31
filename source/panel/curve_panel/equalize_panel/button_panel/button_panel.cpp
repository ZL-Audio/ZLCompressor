// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "button_panel.hpp"

namespace zlpanel {
    ButtonPanel::ButtonPanel(PluginProcessor& p, zlgui::UIBase& base,
                             size_t& selected_band_idx) :
        p_ref_(p), base_(base), selected_band_idx_(selected_band_idx),
        eq_max_db_id_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PEQMaxDB::kID)),
        q_slider_(base), slope_slider_(base),
        eq_db_box_(zlstate::PEQMaxDB::kChoices, base),
        eq_db_box_attachment_(eq_db_box_.getBox(), p.na_parameters_, zlstate::PEQMaxDB::kID, updater_),
        para_panel_(p, base, selected_band_idx),
        popup_panel_(p, base, selected_band_idx),
        right_click_panel_(p, base, selected_band_idx),
        solo_panel_(base),
        items_set_(base.getSelectedBandSet()) {
        addChildComponent(q_slider_);
        slope_slider_.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        addChildComponent(slope_slider_);

        const auto popup_option = juce::PopupMenu::Options()
            .withParentComponent(this)
            .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::downwards);
        for (auto& box : {&eq_db_box_}) {
            box->setScrollEnabled(true);
            box->getLAF().setFontScale(1.f);
            box->getLAF().setOption(popup_option);
            box->setAlpha(.5f);
            box->setBufferedToImage(true);
            addAndMakeVisible(box);
        }

        addChildComponent(solo_panel_);

        addChildComponent(para_panel_);

        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band] = std::make_unique<DraggerPanel>(
                p_ref_, base_, band, selected_band_idx);
            addChildComponent(*dragger_panels_[band]);
            dragger_panels_[band]->addMouseListener(this, true);
        }
        addChildComponent(popup_panel_);

        right_click_panel_.setBufferedToImage(true);
        addChildComponent(right_click_panel_);

        addChildComponent(lasso_component_);
        items_set_.addChangeListener(this);
        for (size_t i = 0; i < zlp::kBandNum; ++i) {
            status_updaters_[i] = std::make_unique<zlp::juce_helper::ParaUpdater>(
                p.parameters_, zlp::PFilterStatus::kID + std::to_string(i));
            freq_updaters_[i] = std::make_unique<zlp::juce_helper::ParaUpdater>(
                p.parameters_, zlp::PFreq::kID + std::to_string(i));
            gain_updaters_[i] = std::make_unique<zlp::juce_helper::ParaUpdater>(
                p.parameters_, zlp::PGain::kID + std::to_string(i));
            q_updaters_[i] = std::make_unique<zlp::juce_helper::ParaUpdater>(
                p.parameters_, zlp::PQ::kID + std::to_string(i));
        }
    }

    ButtonPanel::~ButtonPanel() {
        turnOffSolo();
    }

    void ButtonPanel::resized() {
        {
            const auto bound = getLocalBounds();
            q_slider_.setBounds(bound);
            slope_slider_.setBounds(bound);
            solo_panel_.setBounds(bound);
            para_panel_.setBounds({
                bound.getX(), bound.getY(),
                para_panel_.getIdealWidth(), para_panel_.getIdealHeight()
            });
            right_click_panel_.setBounds({
                bound.getX(), bound.getY(),
                juce::roundToInt(base_.getFontSize() * 7.5f),
                juce::roundToInt(base_.getFontSize() * 4.5f)
            });
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                dragger_panels_[band]->setBounds(bound);
            }
            popup_panel_.setBounds({
                bound.getX(), bound.getY(),
                popup_panel_.getIdealWidth(), popup_panel_.getIdealHeight()
            });
        }
        {
            auto bound = getLocalBounds();
            const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
            bound = bound.removeFromTop(juce::roundToInt(base_.getFontSize() * 1.25f));
            bound.removeFromRight(padding);
            bound = bound.removeFromRight(juce::roundToInt(base_.getFontSize() * 2.5f));
            eq_db_box_.setBounds(bound);
        }
    }

    void ButtonPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        // update eq max dB scale
        const auto c_eq_max_db_id = eq_max_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(c_eq_max_db_id - eq_max_db_id_) > 1e-3f) {
            eq_max_db_id_ = std::round(c_eq_max_db_id);
            const auto eq_max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(eq_max_db_id_)];
            for (size_t band = 0; band < zlp::kBandNum; ++band) {
                dragger_panels_[band]->setEQMaxDB(eq_max_db);
            }
        }
        // update draggers
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            dragger_panels_[band]->repaintCallBackSlow();
        }
        para_panel_.repaintCallBackSlow();
        popup_panel_.repaintCallBackSlow();
        if (filter_type_ref_ && selected_band_idx_ != zlp::kBandNum) {
            if (std::abs(filter_type_ref_->load(std::memory_order::relaxed) - c_filter_type_) > .1f) {
                c_filter_type_ = filter_type_ref_->load(std::memory_order::relaxed);
                const auto filter_type = static_cast<int>(std::round(c_filter_type_));
                const auto flag = (filter_type == 0) || (filter_type == 5) || (filter_type == 6);
                slope_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                    slope_slider_,
                    p_ref_.parameters_, zlp::POrder::kID + std::to_string(selected_band_idx_),
                    juce::NormalisableRange<double>(flag ? 1.0 : 0.0, 5.0, 0.033),
                    updater_);
            }
        }
    }

    void ButtonPanel::updateSampleRate(const double sample_rate) {
        slider_max_ = static_cast<float>(zlp::getEQFreqMax(sample_rate));
        fft_max_ = static_cast<float>(zlp::getEQFFTMax(sample_rate));
        para_panel_.updateFreqMax(slider_max_);
        for (auto& dragger_panel : dragger_panels_) {
            dragger_panel->updateSampleRate(sample_rate);
        }
    }

    void ButtonPanel::updateBand() {
        if (previous_band_idx_ != selected_band_idx_) {
            if (solo_band_ != selected_band_idx_) {
                turnOffSolo();
            }
            if (previous_band_idx_ != zlp::kBandNum) {
                dragger_panels_[previous_band_idx_]->getDragger().getButton().setToggleState(
                    false, juce::sendNotificationSync);
            }
            previous_band_idx_ = selected_band_idx_;
            q_attachment_.reset();
            slope_attachment_.reset();
            if (selected_band_idx_ != zlp::kBandNum) {
                filter_type_ref_ = p_ref_.parameters_.getRawParameterValue(
                    zlp::PFilterType::kID + std::to_string(selected_band_idx_));
                q_attachment_ = std::make_unique<zlgui::attachment::SliderAttachment<true>>(
                    q_slider_,
                    p_ref_.parameters_, zlp::PQ::kID + std::to_string(selected_band_idx_),
                    updater_);
            } else {
                filter_type_ref_ = nullptr;
            }
            para_panel_.updateBand();
            popup_panel_.updateBand();

            if (!items_set_.isSelected(selected_band_idx_)) {
                items_set_.deselectAll();
            } else {
                previous_paras_ = {-99.f, -99.f, -99.f};
                loadSelectedParas();
            }
        }
    }

    void ButtonPanel::setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum>& status) {
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            const auto f = status[band] != zlp::EqualizeController::FilterStatus::kOff;
            dragger_panels_[band]->setVisible(f);
            dragger_panels_[band]->setAlpha(
                status[band] == zlp::EqualizeController::FilterStatus::kBypass ? .75f : 1.f);
            if (f) {
                dragger_panels_[band]->getDragger().getButton().repaint();
            }
        }
    }

    void ButtonPanel::mouseDown(const juce::MouseEvent& event) {
        solo_gain_drag_active_ = false;
        exit_solo_on_mouse_up_ = false;

        const auto event_band = findEventBand(event.originalComponent);
        const auto action_type = event.mods.isRightButtonDown()
                                     ? zlgui::MouseActionType::kRightClick
                                     : zlgui::MouseActionType::kLeftClick;
        const auto solo_action_handled = event_band < zlp::kBandNum
                                             && handleSoloAction(action_type, event.mods, event_band);
        const auto context_menu_triggered = event.mods.isRightButtonDown()
                                            && event.mods.isAltDown()
                                            && !solo_action_handled;

        if (context_menu_triggered) {
            const auto bound = getLocalBounds().toFloat();
            auto target_point = bound.getTopLeft();
            if (event.originalComponent == this) {
                target_point.x += event.position.x;
                target_point.y += event.position.y;
                right_click_panel_.updateCopyVisibility(false);
            } else {
                for (size_t i = 0; i < zlp::kBandNum; ++i) {
                    if (dragger_panels_[i]->isParentOf(event.originalComponent)) {
                        target_point.applyTransform(dragger_panels_[i]->getDragger().getButton().getTransform());
                        right_click_panel_.updateCopyVisibility(true);
                        break;
                    }
                }
            }
            if (target_point.x >= bound.getRight() - static_cast<float>(right_click_panel_.getWidth())
                || target_point.y >= bound.getBottom() - static_cast<float>(right_click_panel_.getHeight())) {
                target_point.x -= static_cast<float>(right_click_panel_.getWidth());
                target_point.y -= static_cast<float>(right_click_panel_.getHeight());
            }
            right_click_panel_.setTransform(juce::AffineTransform::translation(target_point.x, target_point.y));
            right_click_panel_.setVisible(true);
        } else {
            right_click_panel_.setVisible(false);
            if (event.originalComponent == this) {
                turnOffSolo();
                selected_band_idx_ = zlp::kBandNum;
            }
        }
        if (event.originalComponent == this) {
            items_set_.deselectAll();
            lasso_component_.setColour(juce::LassoComponent<size_t>::lassoFillColourId,
                                       base_.getTextColour().withMultipliedAlpha(.125f));
            lasso_component_.setColour(juce::LassoComponent<size_t>::lassoOutlineColourId,
                                       base_.getTextColour().withMultipliedAlpha(.15f));
            lasso_component_.setVisible(true);
            lasso_component_.beginLasso(event, this);
        }
    }

    void ButtonPanel::mouseUp(const juce::MouseEvent& event) {
        if (solo_gain_drag_active_) {
            p_ref_.getEqualizeController().resetSoloGain();
            solo_gain_drag_active_ = false;
        }
        if (event.originalComponent == this) {
            lasso_component_.endLasso();
            lasso_component_.setVisible(false);
        } else if (exit_solo_on_mouse_up_) {
            turnOffSolo();
        }
        exit_solo_on_mouse_up_ = false;
    }

    void ButtonPanel::mouseDrag(const juce::MouseEvent& event) {
        if (event.originalComponent == this) {
            lasso_component_.dragLasso(event);
        }
    }

    void ButtonPanel::mouseDoubleClick(const juce::MouseEvent& event) {
        if (event.originalComponent != this) {
            const auto event_band = findEventBand(event.originalComponent);
            if (event_band < zlp::kBandNum) {
                const auto action_type = event.mods.isRightButtonDown()
                                             ? zlgui::MouseActionType::kRightDoubleClick
                                             : zlgui::MouseActionType::kLeftDoubleClick;
                handleSoloAction(action_type, event.mods, event_band);
            }
            return;
        }
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
        const auto x_portion = std::clamp(x / bound.getWidth(), 0.f, 1.f);
        const auto freq = std::clamp(std::exp(x_portion * std::log(fft_max_ / zlp::kEQMinFreq)) * zlp::kEQMinFreq,
                                     zlp::kEQMinFreq, slider_max_);
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
            auto* para = p_ref_.parameters_.getParameter(init_IDs[i] + suffix);
            para->beginChangeGesture();
            para->setValueNotifyingHost(init_values[i]);
            para->endChangeGesture();
        }

        dragger_panels_[band_idx]->getDragger().getButton().setToggleState(true, juce::sendNotificationSync);
    }

    void ButtonPanel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        const juce::MouseEvent e{
            event.source, event.position,
            event.mods.withoutMouseButtons(),
            event.pressure, event.orientation, event.rotation,
            event.tiltX, event.tiltY,
            event.eventComponent, event.originalComponent,
            event.eventTime, event.mouseDownPosition, event.mouseDownTime,
            event.getNumberOfClicks(), false
        };

        if (e.mods.isCommandDown()) {
            slope_slider_.mouseWheelMove(e, wheel);
        } else {
            q_slider_.mouseWheelMove(e, wheel);
        }
    }

    void ButtonPanel::turnOnSolo(const size_t band) {
        if (solo_band_ < zlp::kBandNum && solo_band_ != band) {
            dragger_panels_[solo_band_]->setSoloActive(false);
        }
        solo_band_ = band;
        dragger_panels_[solo_band_]->setSoloActive(true);
        p_ref_.getEqualizeController().setSoloBand(band);
        solo_panel_.setVisible(true);
    }

    void ButtonPanel::turnOffSolo() {
        if (solo_band_ < zlp::kBandNum) {
            dragger_panels_[solo_band_]->setSoloActive(false);
        }
        solo_band_ = zlp::kBandNum;
        solo_panel_.setVisible(false);
        p_ref_.getEqualizeController().resetSoloGain();
        p_ref_.getEqualizeController().setSoloBand(zlp::kBandNum);
    }

    size_t ButtonPanel::findEventBand(const juce::Component* component) const {
        if (component == nullptr) {
            return zlp::kBandNum;
        }
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (component == dragger_panels_[band].get() || dragger_panels_[band]->isParentOf(component)) {
                return band;
            }
        }
        return zlp::kBandNum;
    }

    bool ButtonPanel::isEnterSoloTriggered(const zlgui::MouseActionType type,
                                           const juce::ModifierKeys& mods) const {
        if (base_.isEnterSoloTriggered(type, mods)) {
            return true;
        }
        constexpr auto drag_modifiers = juce::ModifierKeys::commandModifier
                                        | juce::ModifierKeys::shiftModifier;
        return base_.getEnterSoloKey() == zlgui::KeyActionType::kNone
               && base_.isEnterSoloTriggered(type, mods.withoutFlags(drag_modifiers));
    }

    bool ButtonPanel::isExitSoloTriggered(const zlgui::MouseActionType type,
                                          const juce::ModifierKeys& mods) const {
        if (base_.isExitSoloTriggered(type, mods)) {
            return true;
        }
        constexpr auto drag_modifiers = juce::ModifierKeys::commandModifier
                                        | juce::ModifierKeys::shiftModifier;
        return base_.getExitSoloKey() == zlgui::KeyActionType::kNone
               && base_.isExitSoloTriggered(type, mods.withoutFlags(drag_modifiers));
    }

    bool ButtonPanel::handleSoloAction(const zlgui::MouseActionType type,
                                       const juce::ModifierKeys& mods,
                                       const size_t band) {
        const auto enter_solo_triggered = isEnterSoloTriggered(type, mods);
        const auto exit_solo_triggered = isExitSoloTriggered(type, mods);
        exit_solo_on_mouse_up_ = exit_solo_triggered;

        if (enter_solo_triggered) {
            turnOnSolo(band);
        } else if (exit_solo_triggered) {
            turnOffSolo();
        }

        if (p_ref_.getEqualizeController().getSoloBand() == band) {
            dragger_panels_[band]->startSoloGainDrag();
            solo_gain_drag_active_ = true;
        }
        return enter_solo_triggered || exit_solo_triggered;
    }

    void ButtonPanel::mouseEnter(const juce::MouseEvent& event) {
        if (event.originalComponent == this) {
            startTimer(2000);
        }
    }

    void ButtonPanel::mouseMove(const juce::MouseEvent& event) {
        if (event.originalComponent == this) {
            stopTimer();
            base_.setPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen, 0.f);
            startTimer(2000);
        }
    }

    void ButtonPanel::mouseExit(const juce::MouseEvent& event) {
        if (event.originalComponent == this) {
            stopTimer();
            base_.setPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen, 0.f);
        }
    }

    void ButtonPanel::timerCallback() {
        base_.setPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen, 1.f);
        stopTimer();
    }

    void ButtonPanel::findLassoItemsInArea(juce::Array<size_t>& items_found, const juce::Rectangle<int>& area) {
        const auto float_area = area.toFloat();
        for (size_t idx = 0; idx < dragger_panels_.size(); ++idx) {
            if (dragger_panels_[idx]->isVisible()) {
                if (float_area.contains(dragger_panels_[idx]->getDragger().getButtonPos())) {
                    items_found.add(idx);
                }
            }
        }
    }

    juce::SelectedItemSet<size_t>& ButtonPanel::getLassoSelection() {
        return items_set_;
    }

    void ButtonPanel::changeListenerCallback(juce::ChangeBroadcaster*) {
        if (items_set_.getNumSelected() == 1) {
            selected_band_idx_ = items_set_.getSelectedItem(0);
        }
        for (size_t i = 0; i < dragger_panels_.size(); ++i) {
            const auto f1 = items_set_.isSelected(i);
            if (f1 != dragger_panels_[i]->getDragger().getLAF().getIsSelected()) {
                dragger_panels_[i]->getDragger().getLAF().setIsSelected(f1);
                dragger_panels_[i]->getDragger().getButton().repaint();
            }
        }
        loadSelectedParas();
    }

    void ButtonPanel::loadSelectedParas() {
        if (items_set_.getNumSelected() == 0) {
            previous_paras_ = {-99.f, -99.f, -99.f};
            return;
        }
        for (size_t i : items_set_.getItemArray()) {
            selected_freq_[i] = freq_updaters_[i]->getRawParaValue();
            selected_gain_[i] = gain_updaters_[i]->getRawParaValue();
            selected_q_[i] = q_updaters_[i]->getRawParaValue();
        }
    }

    void ButtonPanel::repaintCallBackAfter() {
        if (items_set_.getNumSelected() == 0 || !items_set_.isSelected(selected_band_idx_)) {
            return;
        }
        const std::array<float, 4> new_paras{
            status_updaters_[selected_band_idx_]->getRawParaValue(),
            freq_updaters_[selected_band_idx_]->getRawParaValue(),
            gain_updaters_[selected_band_idx_]->getRawParaValue(),
            q_updaters_[selected_band_idx_]->getRawParaValue(),
        };
        std::array<bool, 4> to_update;
        for (size_t i = 0; i < 4; ++i) {
            to_update[i] = std::abs(new_paras[i] - previous_paras_[i]) > 1e-5;
        }
        previous_paras_ = new_paras;
        const std::array<float, 3> portion{
            new_paras[1] / selected_freq_[selected_band_idx_],
            new_paras[2] / selected_gain_[selected_band_idx_],
            new_paras[3] / selected_q_[selected_band_idx_]
        };
        if (to_update[0]) {
            for (const size_t& i : items_set_.getItemArray()) {
                if (i != selected_band_idx_) {
                    status_updaters_[i]->updateSync(
                        status_updaters_[i]->getPara()->convertTo0to1(new_paras[0]));
                }
            }
        }
        if (to_update[1]) {
            for (const size_t& i : items_set_.getItemArray()) {
                if (i != selected_band_idx_) {
                    freq_updaters_[i]->updateSync(
                        freq_updaters_[i]->getPara()->convertTo0to1(selected_freq_[i] * portion[0]));
                }
            }
        }
        if (to_update[2]) {
            for (const size_t& i : items_set_.getItemArray()) {
                if (i != selected_band_idx_) {
                    gain_updaters_[i]->updateSync(
                        gain_updaters_[i]->getPara()->convertTo0to1(selected_gain_[i] * portion[1]));
                }
            }
        }
        if (to_update[3]) {
            for (const size_t& i : items_set_.getItemArray()) {
                if (i != selected_band_idx_) {
                    q_updaters_[i]->updateSync(
                        q_updaters_[i]->getPara()->convertTo0to1(selected_q_[i] * portion[2]));
                }
            }
        }
    }
}
