// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.hpp"

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p),
      p_ref_(p),
      property_(p.property_),
      base_(p.state_),
      main_panel_(p, base_),
      equalize_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PSideEQDisplay::kID)) {
    // set font
    const auto font_face = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(font_face);

    // add the main panel
    addAndMakeVisible(main_panel_);

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

    startTimerHz(2);
    updateIsShowing();

    base_.setPanelProperty(zlgui::kUISettingChanged, true);
    base_.getPanelValueTree().addListener(this);
}

PluginEditor::~PluginEditor() {
    base_.getPanelValueTree().removeListener(this);
    vblank_.reset();
    stopTimer();
    p_ref_.getCompressController().setMagAnalyzerOn(false);
}

void PluginEditor::paint(juce::Graphics &g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    main_panel_.setBounds(getLocalBounds());
    last_ui_width_ = getWidth();
    last_ui_height_ = getHeight();
    triggerAsyncUpdate();
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

void PluginEditor::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) {
    if (base_.isPanelIdentifier(zlgui::kUISettingChanged, property)) {
        sendLookAndFeelChange();
        auto &fft{p_ref_.getEqualizeController().getFFTAnalyzer()};
        fft.setExtraTilt(base_.getFFTExtraTilt());
        fft.setExtraSpeed(base_.getFFTExtraSpeed());
        triggerAsyncUpdate();
    }
}

void PluginEditor::handleAsyncUpdate() {
    property_.saveAPVTS(p_ref_.state_);
}

void PluginEditor::timerCallback() {
    updateIsShowing();
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
