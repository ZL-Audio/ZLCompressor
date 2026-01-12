// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "PluginProcessor.hpp"
#include "BinaryData.h"

#include "panel/main_panel.hpp"
#include "gui/gui.hpp"
#include "state/state.hpp"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer,
                     private juce::ValueTree::Listener,
                     private juce::AsyncUpdater {
public:
    explicit PluginEditor(PluginProcessor&);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;

    void resized() override;

    void visibilityChanged() override;

    void parentHierarchyChanged() override;

    void minimisationStateChanged(bool isNowMinimised) override;

    int getControlParameterIndex(Component& c) override;

    void mouseDown(const juce::MouseEvent& event) override;

private:
    PluginProcessor& p_ref_;
    zlstate::Property& property_;
    juce::Value last_ui_width_, last_ui_height_;

    zlgui::UIBase base_;
    zlpanel::MainPanel main_panel_;

    std::unique_ptr<juce::VBlankAttachment> vblank_;

    std::atomic<float>& equalize_show_ref_;

    void timerCallback() override;

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;

    void handleAsyncUpdate() override;

    void updateIsShowing();

    static zlstate::Property& initProperty(PluginProcessor& p);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
