// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "rms_panel.hpp"

namespace zlpanel {
    RMSPanel::RMSPanel(PluginProcessor &processor)
        : avg_analyzer_ref_(processor.getController().getMagAvgAnalyzer()) {
        avg_analyzer_ref_.setToReset();
        setBufferedToImage(true);
    }

    void RMSPanel::paint(juce::Graphics &g) {
        const juce::GenericScopedTryLock guard{path_lock_};
        if (!guard.isLocked()) {
            return;
        }
        g.setColour(juce::Colours::white.withAlpha(.25f));
        g.fillPath(in_path_);
        g.setColour(juce::Colours::white.withAlpha(.9f));
        g.strokePath(out_path_,
                     juce::PathStrokeType(1.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void RMSPanel::run(double nextTimeStamp) {
        juce::ignoreUnused(nextTimeStamp);
        const auto currentBound = atomic_bound_.load();
        avg_analyzer_ref_.run();
        next_in_path_.clear();
        next_out_path_.clear();
        avg_analyzer_ref_.createPath({next_in_path_, next_out_path_}, {true, false},
                               currentBound, 72); {
            const juce::GenericScopedLock guard{path_lock_};
            in_path_ = next_in_path_;
            out_path_ = next_out_path_;
        }
    }

    void RMSPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound.withWidth(bound.getWidth() - 20.f));
    }

    void RMSPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        avg_analyzer_ref_.setToReset();
    }
} // zlpanel
