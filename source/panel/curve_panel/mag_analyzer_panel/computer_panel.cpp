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
    ComputerPanel::ComputerPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        threshold_ref_(*p.parameters_.getRawParameterValue(zlp::PThreshold::kID)),
        ratio_ref_(*p.parameters_.getRawParameterValue(zlp::PRatio::kID)),
        knee_ref_(*p.parameters_.getRawParameterValue(zlp::PKneeW::kID)),
        curve_ref_(*p.parameters_.getRawParameterValue(zlp::PCurve::kID)),
        min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        comp_path_.preallocateSpace(static_cast<int>(kNumPoint) * 3);
        next_comp_path_.preallocateSpace(static_cast<int>(kNumPoint) * 3);
        for (auto& ID : kComputerIDs) {
            p_ref_.parameters_.addParameterListener(ID, this);
        }
        for (auto& ID : kNAIDs) {
            p_ref_.na_parameters_.addParameterListener(ID, this);
        }

        setInterceptsMouseClicks(false, false);
    }

    ComputerPanel::~ComputerPanel() {
        for (auto& ID : kComputerIDs) {
            p_ref_.parameters_.removeParameterListener(ID, this);
        }
        for (auto& ID : kNAIDs) {
            p_ref_.na_parameters_.removeParameterListener(ID, this);
        }
    }

    void ComputerPanel::paint(juce::Graphics& g) {
        const juce::GenericScopedTryLock guard{path_lock_};
        if (!guard.isLocked()) {
            return;
        }
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kComputerColour));
        g.strokePath(comp_path_,
                     juce::PathStrokeType(curve_thickness_,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void ComputerPanel::run() {
        if (!to_update_.exchange(false, std::memory_order::acquire)) { return; }
        const auto is_downward = comp_direction_ref_.load(std::memory_order::relaxed) < .5f;
        computer_.setThreshold(threshold_ref_.load(std::memory_order::relaxed));
        computer_.setRatio(ratio_ref_.load(std::memory_order::relaxed));
        computer_.setKneeW(knee_ref_.load(std::memory_order::relaxed));
        computer_.setCurve(zlp::PCurve::formatV(curve_ref_.load(std::memory_order::relaxed)));

        const auto current_min_db = zlstate::PAnalyzerMinDB::getMinDBFromIndex(
            min_db_ref_.load(std::memory_order::relaxed));
        computer_.prepareBuffer();
        const auto bound = atomic_bound_.load();
        auto db_in = current_min_db;
        const auto delta_db_in = -current_min_db / static_cast<float>(kNumPoint - 1);
        const auto delta_y = bound.getHeight() / static_cast<float>(kNumPoint - 1);
        auto x = bound.getX();
        const auto delta_x = delta_y;
        next_comp_path_.clear();
        const auto mul = bound.getHeight() / current_min_db;
        PathMinimizer minimizer{next_comp_path_};
        minimizer.startNewSubPath(x - bound.getHeight(),
                                  computer_.eval(current_min_db * 2.f) * mul + bound.getY());
        for (size_t i = 0; i < kNumPoint; ++i) {
            const auto db_out = is_downward ? computer_.eval(db_in) : db_in + (db_in - computer_.eval(db_in));
            const auto y = db_out * mul + bound.getY();
            minimizer.lineTo(x, y);
            x += delta_x;
            db_in += delta_db_in;
        }
        minimizer.finish();
        const juce::GenericScopedLock guard{path_lock_};
        comp_path_ = next_comp_path_;
    }

    void ComputerPanel::resized() {
        atomic_bound_.store(getLocalBounds().toFloat());
        to_update_.store(true, std::memory_order::release);
        lookAndFeelChanged();
    }

    void ComputerPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .25f * base_.getMagCurveThickness();
    }

    void ComputerPanel::parameterChanged(const juce::String&, float) {
        to_update_.store(true, std::memory_order::release);
    }
}
