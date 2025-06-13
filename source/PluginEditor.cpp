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
      main_panel_(p, base_) {
    for (auto &ID: kIDs) {
        p_ref_.state_.addParameterListener(ID, this);
    }
    // set font
    const auto font_face = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MiSansLatinMedium_ttf, BinaryData::MiSansLatinMedium_ttfSize);
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(font_face);

    // set size & size listener
    setResizeLimits(static_cast<int>(zlstate::PWindowW::minV),
                    static_cast<int>(zlstate::PWindowH::minV),
                    static_cast<int>(zlstate::PWindowW::maxV),
                    static_cast<int>(zlstate::PWindowH::maxV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);
    last_ui_width_.referTo(p.state_.getParameterAsValue(zlstate::PWindowW::kID));
    last_ui_height_.referTo(p.state_.getParameterAsValue(zlstate::PWindowH::kID));
    setSize(last_ui_width_.getValue(), last_ui_height_.getValue());

    // add the main panel
    addAndMakeVisible(main_panel_);

    startTimerHz(2);

    updateIsShowing();
}

PluginEditor::~PluginEditor() {
    vblank_.reset();
    for (auto &id: kIDs) {
        p_ref_.state_.removeParameterListener(id, this);
    }
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

void PluginEditor::parameterChanged(const juce::String &parameter_id, float new_value) {
    juce::ignoreUnused(parameter_id, new_value);
    is_size_changed_.store(parameter_id == zlstate::PWindowH::kID || parameter_id == zlstate::PWindowW::kID);
    triggerAsyncUpdate();
}

void PluginEditor::handleAsyncUpdate() {
    property_.saveAPVTS(p_ref_.state_);
    if (!is_size_changed_.exchange(false)) {
        sendLookAndFeelChange();
    }
}

void PluginEditor::timerCallback() {
    updateIsShowing();
}

void PluginEditor::updateIsShowing() {
    if (isShowing() != base_.getIsEditorShowing()) {
        base_.setIsEditorShowing(isShowing());
        p_ref_.getCompressController().setMagAnalyzerOn(base_.getIsEditorShowing());
        if (base_.getIsEditorShowing()) {
            vblank_ = std::make_unique<juce::VBlankAttachment>(
                &main_panel_, [this](const double x) { main_panel_.repaintCallBack(x); });
        } else {
            vblank_.reset();
        }
    }
}
