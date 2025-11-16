// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "property.hpp"

namespace zlstate {
    Property::Property() {
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) {
                return;
            }
        }
        if (!kUIPath.existsAsFile()) {
            if (const auto res = kUIPath.create(); !res.wasOk()) {
                return;
            }
        }
        is_directory_created_ = true;
    }

    Property::Property(juce::AudioProcessorValueTreeState& apvts) {
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) {
                return;
            }
        }
        if (!kUIPath.existsAsFile()) {
            if (const auto res = kUIPath.create(); !res.wasOk()) {
                return;
            }
        }
        is_directory_created_ = true;
        loadAPVTS(apvts);
    }

    void Property::loadAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        if (is_directory_created_) {
            std::lock_guard<std::mutex> lock_guard{mutex_};
            if (kUIPath.existsAsFile()) {
                if (const auto xml = juce::XmlDocument::parse(kUIPath); xml) {
                    apvts.replaceState(juce::ValueTree::fromXml(*xml));
                }
            }
        }
    }

    void Property::saveAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        if (is_directory_created_) {
            std::lock_guard<std::mutex> lock{mutex_};
            if (kUIPath.existsAsFile()) {
                if (const auto xml = apvts.copyState().createXml(); xml) {
                    if (!xml->writeTo(kUIPath)) {
                        return;
                    }
                }
            }
        }
    }
}
