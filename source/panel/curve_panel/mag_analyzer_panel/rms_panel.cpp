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
        : avg_analyzer_ref_(processor.getCompressController().getMagAvgAnalyzer()),
          min_db_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        avg_analyzer_ref_.setToReset();
        setBufferedToImage(true);
    }

    void RMSPanel::paint(juce::Graphics &g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
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

    void RMSPanel::run(double next_time_stamp) {
        juce::ignoreUnused(next_time_stamp);
        const auto current_min_db = zlstate::PAnalyzerMinDB::getMinDBFromIndex(
            min_db_ref_.load(std::memory_order::relaxed));
        const auto end_idx = static_cast<size_t>(-current_min_db);
        const auto current_bound = atomic_bound_.load(std::memory_order::relaxed);
        avg_analyzer_ref_.run();
        avg_analyzer_ref_.createPath({in_xs_, out_xs}, ys_, end_idx,
                                     current_bound.getWidth(), current_bound.getHeight());
        next_in_path_.clear();
        next_out_path_.clear();
        next_in_path_.startNewSubPath(0.f, 0.f);
        next_in_path_.lineTo(in_xs_[0], ys_[0]);
        next_out_path_.startNewSubPath(out_xs[0], ys_[0]);
        for (size_t i = 0; i < end_idx; ++i) {
            next_in_path_.lineTo(in_xs_[i], ys_[i]);
            next_out_path_.lineTo(out_xs[i], ys_[i]);
        }
        next_in_path_.lineTo(0.f, current_bound.getHeight());
        next_in_path_.closeSubPath(); {
            // update the paths with lock
            std::lock_guard<std::mutex> lock{mutex_};
            in_path_ = next_in_path_;
            out_path_ = next_out_path_;
        }
    }

    void RMSPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound.withWidth(bound.getWidth() - 20.f), std::memory_order::relaxed);
    }

    void RMSPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        avg_analyzer_ref_.setToReset();
    }
} // zlpanel
