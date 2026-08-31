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
#include "../../../dsp/analyzer/fft_analyzer/spectrum_blender.hpp"
#include "../../../chore/thread/notifier.hpp"

namespace zlpanel {
    class FFTAnalyzerPanel final : public juce::Component,
                                   private juce::ValueTree::Listener {
    public:
        explicit FFTAnalyzerPanel(PluginProcessor& processor, zlgui::UIBase& base);

        ~FFTAnalyzerPanel() override;

        void paint(juce::Graphics& g) override;

        void run(const juce::Thread& thread);

        void resized() override;

        void setRefreshRate(double refresh_rate);

    private:
        static constexpr size_t kNumResolutions = 3;
        static constexpr size_t kLowResolution = 0;
        static constexpr size_t kMiddleResolution = 1;
        static constexpr size_t kHighResolution = 2;

        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        bool skip_next_repaint_{false};
        AtomicBound<float> atomic_bound_;

        std::vector<float> xs_{}, ys_{}, frequencies_{};
        BufferedUI<juce::Path> out_path_;

        double c_sample_rate_{};
        int history_size_{0};
        float c_width_{}, c_height_{}, y_scale_{}, y_bias_{};
        size_t num_point_{0};
        std::atomic<float> font_size_{0.1f};
        zlchore::thread::Notifier to_update_xs_{true};
        zlchore::thread::Notifier to_update_ys_{true};

        std::atomic<float> refresh_rate_{30.0};
        std::atomic<float> spectrum_extra_decay_speed_{1.1f};
        zlchore::thread::Notifier to_update_decay_{true};

        std::atomic<float> spectrum_tilt_slope_{4.5f};
        zlchore::thread::Notifier to_update_tilt_{true};

        std::atomic<bool> is_fft_frozen_{false};

        std::array<zldsp::analyzer::FFTAnalyzerProcessor, kNumResolutions> processors_;
        zldsp::analyzer::FFTAnalyzerReceiver receiver_{processors_[kLowResolution]};
        std::array<zldsp::analyzer::SpectrumSmoother, kNumResolutions> spectrum_smoothers_;
        zldsp::analyzer::SpectrumTilter spectrum_tilter_;
        zldsp::analyzer::SpectrumDecayer spectrum_decayer_;
        std::array<zldsp::vector::aligned_vector<float>, kNumResolutions> resolution_spectra_;
        zldsp::vector::aligned_vector<float> spectrum_;
        std::array<float, kNumResolutions> noise_power_scales_{};

        void lookAndFeelChanged() override;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
