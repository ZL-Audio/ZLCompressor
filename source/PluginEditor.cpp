// Copyright (C) 2024 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.h"

PluginEditor::PluginEditor(PluginProcessor &p)
        : AudioProcessorEditor(&p), processorRef(p) {
    juce::ignoreUnused(processorRef);

    addAndMakeVisible(inspectButton);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
}

PluginEditor::~PluginEditor() {
}

void PluginEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    auto helloWorld = juce::String("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v"
    VERSION + " running in " + CMAKE_BUILD_TYPE;
    g.drawText(helloWorld, area.removeFromTop(150), juce::Justification::centred, false);
}

void PluginEditor::resized() {
    // layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    inspectButton.setBounds(getLocalBounds().withSizeKeepingCentre(100, 50));
}
