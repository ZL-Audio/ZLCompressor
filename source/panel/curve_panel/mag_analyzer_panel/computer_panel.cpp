// Copyright (C) 2026 - zsliu98
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
        min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        comp_path_.preallocateSpace(static_cast<int>(kNumPoint) * 3);
        next_comp_path_.preallocateSpace(static_cast<int>(kNumPoint) * 3);
        for (auto& ID : kComputerIDs) {
            p_ref_.parameters_.addParameterListener(ID, this);
            parameterChanged(ID, p_ref_.parameters_.getRawParameterValue(ID)->load(std::memory_order::relaxed));
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
        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        switch (direction) {
        case zlp::PCompDirection::kCompress: {
            updateComputerPath<zldsp::compressor::CompressionComputer<float>, true>(compression_computer_);
            break;
        }
        case zlp::PCompDirection::kInflate: {
            updateComputerPath<zldsp::compressor::InflationComputer<float>, false>(inflation_computer_);
            break;
        }
        case zlp::PCompDirection::kExpand: {
            updateComputerPath<zldsp::compressor::ExpansionComputer<float>, true>(expansion_computer_);
            break;
        }
        case zlp::PCompDirection::kShape: {
            updateComputerPath<zldsp::compressor::CompressionComputer<float>, false>(compression_computer_);
            break;
        }
        }
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

    void ComputerPanel::parameterChanged(const juce::String& parameter_ID, const float value) {
        if (parameter_ID == zlp::PThreshold::kID) {
            compression_computer_.setThreshold(value);
            expansion_computer_.setThreshold(value);
            inflation_computer_.setThreshold(value);
        } else if (parameter_ID == zlp::PRatio::kID) {
            compression_computer_.setRatio(value);
            expansion_computer_.setRatio(value);
            inflation_computer_.setRatio(value);
        } else if (parameter_ID == zlp::PKneeW::kID) {
            compression_computer_.setKneeW(value);
            expansion_computer_.setKneeW(value);
            inflation_computer_.setKneeW(value);
        } else if (parameter_ID == zlp::PCurve::kID) {
            compression_computer_.setCurve(zlp::PCurve::formatV(value));
        } else if (parameter_ID == zlp::PFloor::kID) {
            expansion_computer_.setFloor(value);
            inflation_computer_.setFloor(value);
        }
        to_update_.store(true, std::memory_order::release);
    }

    template <typename C, bool is_downward>
    void ComputerPanel::updateComputerPath(C& c) {
        const auto current_min_db = zlstate::PAnalyzerMinDB::getMinDBFromIndex(
            min_db_ref_.load(std::memory_order::relaxed));
        c.prepareBuffer();
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
                                  c.eval(current_min_db * 2.f) * mul + bound.getY());
        for (size_t i = 0; i < kNumPoint; ++i) {
            const auto db_out = is_downward ? c.eval(db_in) : db_in + db_in - c.eval(db_in);
            const auto y = db_out * mul + bound.getY();
            minimizer.lineTo(x, y);
            x += delta_x;
            db_in += delta_db_in;
        }
        minimizer.finish();
    }
}
