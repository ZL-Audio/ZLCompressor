// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "computer_panel.hpp"

namespace zlpanel {
    ComputerPanel::ComputerPanel() {
        comp_path_.preallocateSpace(static_cast<int>(numPoint) * 3);
        next_comp_path_.preallocateSpace(static_cast<int>(numPoint) * 3);
        computer_.setThreshold(-18.f);
        computer_.setKneeW(5.f);
        computer_.setCurve(1.f);
    }

    void ComputerPanel::paint(juce::Graphics &g) {
        const juce::GenericScopedTryLock guard{path_lock_};
        if (!guard.isLocked()) {
            return;
        }
        g.setColour(juce::Colours::orange);
        g.strokePath(comp_path_,
                     juce::PathStrokeType(2.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void ComputerPanel::run() {
        if (!to_update_.exchange(false)) { return; }
        const auto current_min_db = min_db_.load();
        computer_.prepareBuffer();
        const auto bound = atomic_bound_.load();
        auto db_in = current_min_db;
        const auto delta_db_in = -current_min_db / static_cast<float>(numPoint - 1);
        const auto delta_y = bound.getHeight() / static_cast<float>(numPoint - 1);
        auto x = bound.getX();
        const auto delta_x = delta_y;
        next_comp_path_.clear();
        PathMinimizer minimizer{next_comp_path_};
        for (size_t i = 0; i < numPoint; ++i) {
            const auto db_out = computer_.eval(db_in);
            const auto y = db_out / current_min_db * bound.getHeight() + bound.getY();
            if (i == 0) {
                minimizer.startNewSubPath(x, y);
            } else {
                minimizer.lineTo(x, y);
            }
            x += delta_x;
            db_in += delta_db_in;
        }
        minimizer.finish();
        const juce::GenericScopedLock guard{path_lock_};
        comp_path_ = next_comp_path_;
    }

    void ComputerPanel::resized() {
        atomic_bound_.store(getLocalBounds().toFloat());
        to_update_.store(true);
    }
} // zlpanel
