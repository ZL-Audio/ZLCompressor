// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "sum_panel.hpp"

namespace zlpanel {
    SumPanel::SumPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : p_ref_(processor), base_{base} {
        juce::ignoreUnused(p_ref_);
        path_.preallocateSpace(kWsFloat.size() * 3 + 12);
        next_path_.preallocateSpace(kWsFloat.size() * 3 + 12);

        setInterceptsMouseClicks(false, false);
    }

    SumPanel::~SumPanel() = default;

    void SumPanel::paint(juce::Graphics &g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getColourMap2(0));
        g.strokePath(path_, juce::PathStrokeType(curve_thickness_,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    void SumPanel::resized() {
        lookAndFeelChanged();
    }

    void SumPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.getFontSize() * .2f * base_.getEQCurveThickness();
    }

    bool SumPanel::run(std::array<float, kWsFloat.size()> &xs,
                       std::array<std::array<float, kWsFloat.size()>, 8> &yss,
                       std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &filter_status,
                       const juce::Rectangle<float> &bound) {
        int band_count{0};
        for (size_t band = 0; band < zlp::kBandNum; ++band) {
            if (filter_status[band] == zlp::EqualizeController::FilterStatus::kOn) {
                auto ys_vector = kfr::make_univector<float>(yss[band].data(), yss[band].size());
                if (band_count == 0) {
                    ys = ys_vector;
                } else {
                    ys = ys + ys_vector;
                }
                band_count += 1;
            }
        }

        if (band_count == 0) {
            std::fill(ys.begin(), ys.end(), bound.getCentreY());
        } else if (band_count > 1) {
            const auto total_shift = -bound.getCentreY() * static_cast<float>(band_count - 1);
            ys = ys + total_shift;
        }

        next_path_.clear();
        PathMinimizer minimizer(next_path_);
        minimizer.startNewSubPath(xs[0], ys[0]);
        for (size_t i = 1; i < std::min(xs.size(), ys.size()); ++i) {
            minimizer.lineTo(xs[i], ys[i]);
        }
        minimizer.finish();

        std::lock_guard<std::mutex> lock{mutex_};
        path_.swapWithPath(next_path_);

        return true;
    }
} // zlpanel
