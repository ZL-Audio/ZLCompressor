// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/analyzer/fft_analyzer/fft_analyzer_receiver.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_smoother.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_tilter.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_decayer.hpp"
#include "../../../dsp/lock/spin_lock.hpp"

namespace zlpanel {
    class FFTAnalyzerPanel final : public juce::Component,
                                   private juce::ValueTree::Listener {
    public:
        explicit FFTAnalyzerPanel(PluginProcessor& processor, zlgui::UIBase& base);

        ~FFTAnalyzerPanel() override;

        void paint(juce::Graphics& g) override;

        void run();

        void resized() override;

        void setRefreshRate(double refresh_rate);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        bool skip_next_repaint_{false};
        AtomicBound<float> atomic_bound_;

        std::vector<float> xs_{}, ys_{};
        juce::Path out_path_, next_out_path_;
        zldsp::lock::SpinLock mutex_;

        double c_sample_rate_{};
        int fft_size_{0};
        float c_width_{};
        size_t num_point_{0};

        std::atomic<float> refresh_rate_{30.0};
        std::atomic<float> spectrum_decay_speed_{-20.f};
        std::atomic<bool> to_update_decay_{false};

        std::atomic<float> spectrum_tilt_slope_{4.5f};
        std::atomic<bool> to_update_tilt_{false};

        std::atomic<bool> is_fft_frozen_{false};

        zldsp::analyzer::FFTAnalyzerReceiver<1> receiver_;
        zldsp::analyzer::SpectrumSmoother spectrum_smoother_;
        zldsp::analyzer::SpectrumTilter spectrum_tilter_;
        zldsp::analyzer::SpectrumDecayer spectrum_decayer_;

        void lookAndFeelChanged() override;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
