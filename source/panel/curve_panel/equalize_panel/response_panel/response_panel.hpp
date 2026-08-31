// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <atomic>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include "../../../../PluginProcessor.hpp"
#include "../../../../chore/thread/notifier.hpp"
#include "single_panel.hpp"
#include "sum_panel.hpp"

namespace zlpanel {
    class ResponsePanel final : public juce::Component,
                                public juce::Thread,
                                private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ResponsePanel(PluginProcessor& processor, zlgui::UIBase& base,
                               size_t& selected_band_idx);

        ~ResponsePanel() override;

        void resized() override;

        void repaintCallBack();

        void updateSampleRate(double sample_rate);

        juce::Point<float> getBandButtonPos(size_t band) const {
            return {
                points_[band][0].load(std::memory_order::relaxed),
                points_[band][4].load(std::memory_order::relaxed)
            };
        }

        std::pair<float, float> getBandSoloRange(size_t band) const {
            return {
                points_[band][1].load(std::memory_order::relaxed),
                points_[band][2].load(std::memory_order::relaxed)
            };
        }

        void updateBand();

        void run() override;

    private:
        static constexpr size_t kNumPoints = 400;
        static constexpr std::array kBandIDs{
            zlp::PFilterStatus::kID, zlp::PFilterType::kID, zlp::POrder::kID,
            zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID
        };

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        std::vector<size_t> message_not_off_indices_{};
        zlchore::thread::Notifier message_to_update_panels_{true};

        SinglePanel single_panel_;
        SumPanel sum_panel_;

        std::atomic<float> width_{0.f}, height_{0.f}, font_size_{1.f};
        float c_width_{0.f}, c_height_{0.f}, c_font_size_{1.f};
        zlchore::thread::Notifier to_update_bound_{};

        std::atomic<float>& eq_max_db_idx_ref_;
        float c_eq_max_db_idx_{-1.f};
        float c_scale_{0.f}, c_bias_{0.f};

        std::atomic<double> sample_rate_{48000.0};
        double c_sample_rate_{0.0}, c_slider_max_{0.0}, fft_max_{0.0};

        zldsp::vector::aligned_vector<float> ws_{};
        zldsp::vector::aligned_vector<float> xs_{};

        std::array<zldsp::filter::Ideal<float, 16>, zlp::kBandNum> ideal_{};
        std::array<zldsp::vector::aligned_vector<float>, zlp::kBandNum> magnitudes_{};

        std::array<zldsp::filter::Empty, zlp::kBandNum> empty_{};
        std::array<zlchore::thread::Notifier, zlp::kBandNum> to_update_empty_flags_{};

        std::array<std::atomic<zlp::EqualizeController::FilterStatus>, zlp::kBandNum> filter_status_{};
        std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> c_filter_status_{};
        zlchore::thread::Notifier to_update_filter_status_{true};

        std::vector<size_t> on_indices_{};
        bool has_not_off_filter_{false};
        std::array<bool, zlp::kBandNum> to_update_curve_flags_{};
        bool to_update_sum_{true};

        // center x, left x, right x, curve center y, button y
        std::array<std::array<std::atomic<float>, 5>, zlp::kBandNum> points_{};

        void parameterChanged(const juce::String& parameter_ID, float value) override;

        bool updateCurveParameters();

        bool updateCurveMagnitudes();

        static float getButtonMagnitude(const zldsp::filter::FilterParameters& parameters);

        std::tuple<float, float, float> getLeftCenterRightX(
            zldsp::filter::FilterParameters parameters) const;
    };
}
