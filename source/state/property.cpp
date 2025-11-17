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
    }

    Property::Property(juce::AudioProcessorValueTreeState& apvts) {
        loadAPVTS(apvts);
    }

    void Property::loadAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        std::lock_guard<std::mutex> lock_guard{mutex_};
        if (checkCreateDirectory()) {
            if (const auto xml = juce::XmlDocument::parse(kUIPath); xml) {
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
            }
        }
    }

    void Property::saveAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        std::lock_guard<std::mutex> lock{mutex_};
        if (checkCreateDirectory()) {
            if (const auto xml = apvts.copyState().createXml(); xml) {
                if (!xml->writeTo(kUIPath)) {
                    return;
                }
            }
        }
    }

    bool Property::checkCreateDirectory() const {
        // create directory if not exists
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) {
                return false;
            }
        }
        if (kUIPath.existsAsFile()) {
            return true;
        } else if (!kUIPath.existsAsFile()) {
            if (kOldUIPath.existsAsFile()) {
                if (const auto c_res = kOldUIPath.copyFileTo(kUIPath); c_res) {
                    if (const auto d_res = kOldUIPath.deleteFile(); d_res) {
                        return true;
                    }
                }
            }
            const auto res = kUIPath.create();
            return res.wasOk();
        }
        return false;
    }
}
