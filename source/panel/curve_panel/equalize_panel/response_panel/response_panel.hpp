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
#include <span>

#include "../../../../PluginProcessor.hpp"
#include "../../../../chore/thread/notifier.hpp"
#include "../../../helper/helper.hpp"
#include "single_panel.hpp"
#include "sum_panel.hpp"

namespace zlpanel {
    class ResponsePanel final : public juce::Component,
                                private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ResponsePanel(PluginProcessor& processor, zlgui::UIBase& base);

        ~ResponsePanel() override;

        void resized() override;

        void run(juce::Thread& thread);

        void updateSampleRate(double sample_rate);

        void setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum>& status);

        juce::Point<float> getBandButtonPos(const size_t band) const {
            return single_panels_[band]->getButtonPos();
        }

        void setMouseOver(const bool f) {
            dummy_component_.setVisible(f);
        }

        void updateBand(size_t band);

    private:
        static constexpr size_t kNumPoints = 400;
        static constexpr std::array kBandIDs{
            zlp::PFilterStatus::kID, zlp::PFilterType::kID, zlp::POrder::kID,
            zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID
        };

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& eq_max_db_id_ref_;
        float eq_max_db_id_{-1.f}, eq_max_db_{0.f};

        std::array<zldsp::filter::Ideal<float, 16>, zlp::kBandNum> filters_;
        std::array<zldsp::filter::Empty, zlp::kBandNum> empty_filters_;
        std::array<zlchore::thread::Notifier, zlp::kBandNum> empty_update_flags_;
        std::array<std::atomic<zlp::EqualizeController::FilterStatus>, zlp::kBandNum> filter_status_{};
        std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> c_filter_status_{};
        zlchore::thread::Notifier to_update_filter_status_{true};

        juce::Component dummy_component_;
        std::array<std::unique_ptr<SinglePanel>, zlp::kBandNum> single_panels_;
        SumPanel sum_panel_;

        zldsp::vector::aligned_vector<float> xs_, ws_;
        std::array<zldsp::vector::aligned_vector<float>, zlp::kBandNum> mags_;

        AtomicBound<float> bound_;
        juce::Rectangle<float> c_bound_;
        std::atomic<double> sample_rate_{48000.0};
        double c_sample_rate_{0.0}, slider_max_{20000.0}, fft_max_{22000.0};
        std::array<bool, zlp::kBandNum> to_update_curve_flags_{};
        bool to_update_sum_{true};

        void parameterChanged(const juce::String& parameter_ID, float new_value) override;

        void updateCurveParameters();
    };
}
