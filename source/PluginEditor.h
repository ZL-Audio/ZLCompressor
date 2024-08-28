// Copyright (C) 2024 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor {
public:
    explicit PluginEditor(PluginProcessor &);

    ~PluginEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor &processorRef;
    juce::TextButton inspectButton{"Inspect the UI"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
