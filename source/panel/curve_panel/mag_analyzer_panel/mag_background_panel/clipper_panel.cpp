// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "clipper_panel.hpp"
#include "../../mag_db_range.hpp"

namespace zlpanel {
    ClipperPanel::ClipperPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        comp_direction_ref_(*p.parameters_.getRawParameterValue(zlp::PCompDirection::kID)),
        mag_max_db_id_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMaxDB::kID)),
        mag_min_db_id_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerMinDB::kID)) {
        for (auto& ID : kClipperIDs) {
            p_ref_.parameters_.addParameterListener(ID, this);
        }
        for (auto& ID : kClipperIDs) {
            parameterChanged(ID, p_ref_.parameters_.getRawParameterValue(ID)->load(std::memory_order::relaxed));
        }
        setInterceptsMouseClicks(false, false);
    }

    ClipperPanel::~ClipperPanel() {
        for (auto& ID : kClipperIDs) {
            p_ref_.parameters_.removeParameterListener(ID, this);
        }
    }

    void ClipperPanel::paint(juce::Graphics& g) {
        const auto bound = getLocalBounds().toFloat();
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kComputerColour).withAlpha(.5f));
        const auto height = base_.getFontSize() * .15f * base_.getMagCurveThickness();
        g.fillRect(bound.getX(), bound.getY() + bound.getHeight() * rel_position_ - height,
                   bound.getWidth(), height);
    }

    void ClipperPanel::repaintCallBackSlow() {
        const auto mag_max_db_id = mag_max_db_id_ref_.load(std::memory_order::relaxed);
        const auto mag_min_db_id = mag_min_db_id_ref_.load(std::memory_order::relaxed);
        if (std::abs(mag_max_db_id_ - mag_max_db_id) > 1e-3f ||
            std::abs(mag_min_db_id_ - mag_min_db_id) > 1e-3f) {
            mag_max_db_id_ = mag_max_db_id;
            mag_min_db_id_ = mag_min_db_id;
            mag_max_db_ = zlstate::PAnalyzerMaxDB::getDBFromIndex(mag_max_db_id);
            mag_range_db_ = zlstate::PAnalyzerMinDB::getDBFromIndex(mag_min_db_id);
            to_update_path_.store(true, std::memory_order::release);
        }
        if (!to_update_path_.exchange(false, std::memory_order::acquire)) {
            return;
        }

        const auto direction = static_cast<zlp::PCompDirection::Direction>(std::round(
            comp_direction_ref_.load(std::memory_order::relaxed)));
        if (direction == zlp::PCompDirection::kCompress) {
            computer_.prepareBuffer();
            clipper_.setReductionAtUnit(computer_.eval(0.f));
        } else {
            clipper_.setReductionAtUnit(-2.360900573208f);
        }
        clipper_.prepareBuffer();
        if (clipper_.getIsON()) {
            setVisible(true);
        } else {
            setVisible(false);
            return;
        }

        const MagDBRange db_range{mag_max_db_, mag_range_db_};
        const auto clipper_db = zldsp::chore::gainToDecibels(clipper_.processSample(1.f));
        rel_position_ = db_range.getYProportion(clipper_db);

        repaint();
    }

    void ClipperPanel::parameterChanged(const juce::String& parameter_ID, const float new_value) {
        if (parameter_ID == zlp::PThreshold::kID) {
            computer_.setThreshold(new_value);
        } else if (parameter_ID == zlp::PRatio::kID) {
            computer_.setRatio(new_value);
        } else if (parameter_ID == zlp::PKneeW::kID) {
            computer_.setKneeW(new_value);
        } else if (parameter_ID == zlp::PCurve::kID) {
            computer_.setCurve(zlp::PCurve::formatV(new_value));
        } else if (parameter_ID == zlp::PClipperDrive::kID) {
            clipper_.setWet(new_value);
        }
        to_update_path_.store(true, std::memory_order::release);
    }
}
