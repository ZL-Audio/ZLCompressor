// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "peak_panel.hpp"

namespace zlpanel {
    PeakPanel::PeakPanel(PluginProcessor &processor)
        : mag_analyzer_ref_(processor.getController().getMagAnalyzer()) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::CompressorController::kAnalyzerPointNum) * 3 + 1;
        for (auto &path: {&in_path_, &out_path_, &reduction_path_}) {
            path->preallocateSpace(preallocateSpace);
        }
        mag_analyzer_ref_.setToReset();
        setTimeLength(6.f);
    }

    void PeakPanel::paint(juce::Graphics &g) {
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
        g.setColour(juce::Colours::mediumvioletred);
        g.strokePath(reduction_path_,
                     juce::PathStrokeType(2.5f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    void PeakPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        constexpr auto pad_p = 1.f / static_cast<float>(zlp::CompressorController::kAnalyzerPointNum - 1);
        const auto pad = std::max(bound.getWidth() * pad_p, 1.f);
        bound = bound.withWidth(bound.getWidth() + pad);
        atomic_bound_.store(bound);
    }

    void PeakPanel::run(const double next_time_stamp) {
        if (is_first_point_) {
            if (mag_analyzer_ref_.run(1) > 0) {
                is_first_point_ = false;
                current_count_ = 0.;
                start_time_ = next_time_stamp; {
                    next_in_path_.clear();
                    next_out_path_.clear();
                    next_reduction_path_.clear();
                }
                mag_analyzer_ref_.template createPath<true, false>(next_in_path_, next_out_path_, next_reduction_path_,
                                       atomic_bound_.load(), 0.f, 0.f);
            }
        } else {
            const auto target_count = (next_time_stamp - start_time_) * num_per_second_.load();
            const auto target_delta = target_count - current_count_;
            const auto actual_delta = static_cast<double>(mag_analyzer_ref_.run(static_cast<int>(std::floor(target_delta))));
            current_count_ += static_cast<double>(actual_delta);
            const auto current_error = std::abs(target_delta - actual_delta);
            if (current_error < smooth_error_) {
                smooth_error_ = current_error;
            } else {
                smooth_error_ = smooth_error_ * 0.95 + current_error * 0.05;
            }
            if (smooth_error_ > 1.0) {
                current_count_ += std::floor(smooth_error_);
                if (cons_error_count_ < 5) {
                    cons_error_count_ += 1;
                }
            }
            if (actual_delta > 0) {
                cons_error_count_ = 0;
            }
            if (cons_error_count_ < 5) {
                const auto shift = target_count - current_count_; {
                    next_in_path_.clear();
                    next_out_path_.clear();
                    next_reduction_path_.clear();
                }
                mag_analyzer_ref_.template createPath<true, false>(next_in_path_, next_out_path_, next_reduction_path_,
                                                             atomic_bound_.load(), static_cast<float>(shift), 0.f);
            }
        } {
            const juce::GenericScopedLock guard{path_lock_};
            in_path_ = next_in_path_;
            out_path_ = next_out_path_;
            reduction_path_ = next_reduction_path_;
        }
    }
} // zlpanel
