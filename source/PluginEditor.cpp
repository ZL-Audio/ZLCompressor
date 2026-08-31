// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor& p) :
    AudioProcessorEditor(&p),
    p_ref_(p),
    property_(initProperty(p)),
    base_(p.state_),
    main_panel_(p, base_),
    equalize_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideEQDisplay::kID)) {
    // set font
#if defined(JUCE_WINDOWS)
    base_.font_ = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterSubsetMediumNoHinting_ttf, BinaryData::InterSubsetMediumNoHinting_ttfSize);
#else
    base_.font_ = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterSubsetMedium_ttf, BinaryData::InterSubsetMedium_ttfSize);
#endif
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(base_.font_);

    // add the main panel
    addAndMakeVisible(main_panel_);
    main_panel_.getControlPanel().addMouseListener(this, true);
    main_panel_.getSideControlPanel().addMouseListener(this, true);

    // set size & size listener
    setResizeLimits(static_cast<int>(zlstate::PWindowW::minV),
                    static_cast<int>(zlstate::PWindowH::minV),
                    static_cast<int>(zlstate::PWindowW::maxV),
                    static_cast<int>(zlstate::PWindowH::maxV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);

    this->resizableCorner = std::make_unique<zlgui::ResizeCorner>(base_, this, getConstrainer(),
                                                                  zlgui::ResizeCorner::kScaleWithWidth, 0.025f);
    addChildComponent(this->resizableCorner.get());
    this->resizableCorner->setAlwaysOnTop(true);
    this->resizableCorner->resized();

    last_ui_width_.referTo(p.state_.getParameterAsValue(zlstate::PWindowW::kID));
    last_ui_height_.referTo(p.state_.getParameterAsValue(zlstate::PWindowH::kID));
    setSize(last_ui_width_.getValue(), last_ui_height_.getValue());

    startTimer(kVisibilityTimer, 500);
    updateIsShowing();

    base_.setPanelProperty(zlgui::kUISettingChanged, true);
    base_.getPanelValueTree().addListener(this);
}

PluginEditor::~PluginEditor() {
    base_.getPanelValueTree().removeListener(this);
    flushPendingPropertySave();
    vblank_.reset();
    stopTimer(kVisibilityTimer);
    p_ref_.getCompressController().setMagAnalyzerOn(false);
}

void PluginEditor::paint(juce::Graphics& g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    main_panel_.setBounds(getLocalBounds());
    if (!base_.getWindowSizeFix()) {
        const auto width = getWidth();
        const auto height = getHeight();
        const auto size_changed = width != static_cast<int>(last_ui_width_.getValue()) ||
            height != static_cast<int>(last_ui_height_.getValue());
        last_ui_width_ = width;
        last_ui_height_ = height;
        triggerAsyncUpdate();
        if (size_changed) {
            schedulePropertySave();
        }
    }
}

void PluginEditor::visibilityChanged() {
    updateIsShowing();
}

void PluginEditor::parentHierarchyChanged() {
    updateIsShowing();
}

void PluginEditor::minimisationStateChanged(bool) {
    updateIsShowing();
}

void PluginEditor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
    if (base_.isPanelIdentifier(zlgui::kUISettingChanged, property)) {
        triggerAsyncUpdate();
        schedulePropertySave();
    }
}

void PluginEditor::handleAsyncUpdate() {
    sendLookAndFeelChange();
}

void PluginEditor::timerCallback(const int timer_id) {
    if (timer_id == kVisibilityTimer) {
        updateIsShowing();
    } else if (timer_id == kPropertySaveTimer) {
        flushPendingPropertySave();
    }
}

void PluginEditor::schedulePropertySave() {
    startTimer(kPropertySaveTimer, kPropertySaveDelayMS);
}

void PluginEditor::flushPendingPropertySave() {
    if (isTimerRunning(kPropertySaveTimer)) {
        stopTimer(kPropertySaveTimer);
        property_.saveAPVTS(p_ref_.state_);
    }
}

void PluginEditor::updateIsShowing() {
    if (isShowing() != base_.getIsEditorShowing()) {
        base_.setIsEditorShowing(isShowing());
        p_ref_.getCompressController().setMagAnalyzerOn(base_.getIsEditorShowing());
        p_ref_.getEqualizeController().setFFTAnalyzerON(
            base_.getIsEditorShowing() && equalize_show_ref_.load(std::memory_order::relaxed) > .5f);
        if (base_.getIsEditorShowing()) {
            vblank_ = std::make_unique<juce::VBlankAttachment>(
                &main_panel_, [this](const double x) { main_panel_.repaintCallBack(x); });
        } else {
            vblank_.reset();
        }
    }
}

int PluginEditor::getControlParameterIndex(Component& c) {
    const auto id = c.getComponentID();
    if (id.isEmpty()) {
        return -1;
    }
    if (const auto para = p_ref_.parameters_.getParameter(id); para == nullptr) {
        return -1;
    } else {
        return para->getParameterIndex();
    }
}

void PluginEditor::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isRightButtonDown() && event.getNumberOfClicks() == 1) {
        if (event.originalComponent != nullptr) {
            if (const auto id = event.originalComponent->getComponentID(); !id.isEmpty()) {
                if (const auto para = p_ref_.parameters_.getParameter(id); para != nullptr) {
                    if (const auto* context = getHostContext(); context != nullptr) {
                        if (const auto menu = context->getContextMenuForParameter(para)) {
                            menu->showNativeMenu(juce::Component::getMouseXYRelative());
                        }
                    }
                }
            }
        }
    }
}

zlstate::Property& PluginEditor::initProperty(PluginProcessor& p) {
    p.property_.loadAPVTS(p.state_);
    return p.property_;
}
