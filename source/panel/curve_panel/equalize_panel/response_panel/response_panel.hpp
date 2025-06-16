// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "single_panel.hpp"
#include "sum_panel.hpp"
#include "static_freq_array.hpp"

namespace zlpanel {
    class ResponsePanel final : public juce::Component {
    public:
        explicit ResponsePanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~ResponsePanel() override;

        void resized() override;

        void run(std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &filter_status,
                 bool to_update_sum);

        void setBandStatus(const std::array<zlp::EqualizeController::FilterStatus, zlp::kBandNum> &status);

        juce::Point<float> getBandButtonPos(const size_t band) const {
            return single_panels_[band]->getButtonPos();
        }

        void setMouseOver(const bool f) {
            dummy_component_.setVisible(f);
        }

    private:
        std::array<zldsp::filter::Ideal<float, 16>, zlp::kBandNum> filters_;
        juce::Component dummy_component_;
        std::array<std::unique_ptr<SinglePanel>, zlp::kBandNum> single_panels_;
        SumPanel sum_panel_;

        std::array<float, kWsFloat.size()> xs_{};
        std::array<std::array<float, kWsFloat.size()>, 8> yss_{};

        AtomicBound<float> bound_;
        juce::Rectangle<float> c_bound_;
    };
} // zlpanel
