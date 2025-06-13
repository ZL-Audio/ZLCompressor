// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "fft_analyzer_panel.hpp"

namespace zlpanel {
    FFTAnalyzerPanel::FFTAnalyzerPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : p_ref_(processor),
          base_(base) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::EqualizeController::kAnalyzerPointNum) * 3 + 1;
        for (auto &path: {&out_path_, &next_out_path_}) {
            path->preallocateSpace(preallocateSpace);
        }
    }

    FFTAnalyzerPanel::~FFTAnalyzerPanel() {
    }


    void FFTAnalyzerPanel::paint(juce::Graphics &g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getTextColor());
        g.strokePath(out_path_,
                     juce::PathStrokeType(1.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void FFTAnalyzerPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound, std::memory_order::relaxed);
    }

    void FFTAnalyzerPanel::run() {
        auto &analyzer{p_ref_.getEqualizeController().getFFTAnalyzer()};
        analyzer.run();

        const auto bound = atomic_bound_.load(std::memory_order::relaxed);
        if (std::abs(bound.getWidth() - width_) > 1e-3f) {
            width_ = bound.getWidth();
            analyzer.createPathXs(xs_, width_);
        }
        analyzer.createPathYs({std::span{ys_}}, bound.getHeight());

        next_out_path_.clear();
        next_out_path_.startNewSubPath(xs_[0], ys_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            next_out_path_.lineTo(xs_[i], ys_[i]);
        }

        std::lock_guard<std::mutex> lock{mutex_};
        out_path_ = next_out_path_;
    }
} // zlpanel
