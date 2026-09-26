// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "fft_analyzer_panel.hpp"

namespace zlpanel {
    FFTAnalyzerPanel::FFTAnalyzerPanel(PluginProcessor& processor, zlgui::UIBase& base) :
        p_ref_(processor),
        base_(base) {
        constexpr auto preallocate_space = static_cast<int>(zlp::EqualizeController::kAnalyzerPointNum) * 3 + 1;
        for (auto& path : out_path_.get_buffer()) {
            path.preallocateSpace(preallocate_space);
        }
        receiver_.setON(true);
        setInterceptsMouseClicks(false, false);

        base_.getPanelValueTree().addListener(this);
    }

    FFTAnalyzerPanel::~FFTAnalyzerPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void FFTAnalyzerPanel::paint(juce::Graphics& g) {
        if (skip_next_repaint_) {
            skip_next_repaint_ = false;
            return;
        }
        out_path_.pull();
        g.setColour(base_.getTextColour().withAlpha(.375f));
        g.fillPath(out_path_.get_reader());
    }

    void FFTAnalyzerPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound);
        font_size_.store(base_.getFontSize(), std::memory_order::relaxed);
        skip_next_repaint_ = true;
        to_update_xs_.signal();
        to_update_ys_.signal();
    }

    void FFTAnalyzerPanel::run(const juce::Thread& thread) {
        juce::ScopedNoDenormals no_denormals;
        const auto bound = atomic_bound_.load();
        auto& sender{p_ref_.getEqualizeController().getFFTAnalyzerSender()};
        if (!sender.getLock().try_lock()) {
            return;
        }

        const auto sample_rate = sender.getSampleRate();
        const auto high_quality = base_.getFFTQuality() == zlstate::PFFTQuality::kHigh;
        const auto first_resolution = high_quality ? kLowResolution : kMiddleResolution;
        const auto end_resolution = high_quality ? kNumResolutions : kMiddleResolution + 1;
        if (std::abs(c_sample_rate_ - sample_rate) > 0.1 || high_quality != c_high_quality_) {
            c_sample_rate_ = sample_rate;
            c_high_quality_ = high_quality;
            to_update_tilt_.signal();
            to_update_decay_.signal();

            int middle_fft_order;
            if (sample_rate <= 50000.0) {
                middle_fft_order = 12;
            } else if (sample_rate <= 100000.0) {
                middle_fft_order = 13;
            } else if (sample_rate <= 200000.0) {
                middle_fft_order = 14;
            } else {
                middle_fft_order = 15;
            }
            const std::array fft_orders{
                middle_fft_order + 2, middle_fft_order, middle_fft_order - 2
            };
            for (size_t i = first_resolution; i < end_resolution; ++i) {
                processors_[i].prepare(fft_orders[i]);
            }

            const auto reference_window_power = processors_[kMiddleResolution].getWindowSqrSum();
            for (size_t i = first_resolution; i < end_resolution; ++i) {
                noise_power_scales_[i] = static_cast<float>(
                    reference_window_power / processors_[i].getWindowSqrSum());
            }

            history_size_ = static_cast<int>(processors_[first_resolution].getFFTSize());
            receiver_.prepare(2, static_cast<size_t>(history_size_));
            for (size_t i = first_resolution; i < end_resolution; ++i) {
                spectrum_smoothers_[i].prepare(processors_[i].getFFTSize());
                spectrum_smoothers_[i].setSmooth(
                    0.5, sample_rate, zldsp::analyzer::SpectrumSmoother::SmoothMethod::kERB);
                resolution_spectra_[i].resize(processors_[i].getFFTSize() / 2 + 1);
            }

            if (high_quality) {
                frequencies_ = zldsp::analyzer::SpectrumBlender::createFrequencyGrid(
                    processors_[kLowResolution].getFFTSize(),
                    processors_[kMiddleResolution].getFFTSize(),
                    processors_[kHighResolution].getFFTSize(), sample_rate);
            } else {
                const auto fft_size = processors_[kMiddleResolution].getFFTSize();
                const auto delta_freq = static_cast<float>(sample_rate / static_cast<double>(fft_size));
                frequencies_.resize(fft_size / 2 + 1);
                for (size_t i = 0; i < frequencies_.size(); ++i) {
                    frequencies_[i] = static_cast<float>(i) * delta_freq;
                }
            }
            spectrum_tilter_.prepareSpectrum(frequencies_.size());
            spectrum_decayer_.prepareSpectrum(frequencies_.size());
            spectrum_.resize(frequencies_.size());
            xs_.resize(frequencies_.size());
            ys_.resize(frequencies_.size());
            if (high_quality) {
                inter_.reset();
            } else {
                inter_ = std::make_unique<zldsp::interpolation::SeqMakima<float>>(
                    xs_.data(), ys_.data(), kInterSize / 2 + 2, 0.f, 0.f);
            }

            to_update_xs_.signal();
            to_update_ys_.signal();
        }

        auto& fifo{sender.getAbstractFIFO()};
        auto num_read = fifo.getNumReady() / 4 * 3;
        if (num_read > history_size_) {
            (void)fifo.prepareToRead(num_read - history_size_);
            fifo.finishRead(num_read - history_size_);
            num_read = history_size_;
        }
        const auto range = fifo.prepareToRead(num_read);
        receiver_.pull(range, sender.getSampleFIFOs()[0]);
        fifo.finishRead(num_read);
        sender.getLock().unlock();

        if (thread.threadShouldExit() || history_size_ <= 0
            || bound.getWidth() <= 0.f || bound.getHeight() <= 0.f) {
            return;
        }

        if (to_update_tilt_.check()) {
            const auto slope = spectrum_tilt_slope_.load(std::memory_order::relaxed);
            if (high_quality) {
                spectrum_tilter_.setTiltSlope(frequencies_, slope);
            } else {
                spectrum_tilter_.setTiltSlope(sample_rate, slope);
            }
        }
        if (to_update_decay_.check()) {
            const auto decay_speed = spectrum_extra_decay_speed_.load(std::memory_order::relaxed);
            spectrum_decayer_.setDecaySpeed(refresh_rate_.load(std::memory_order::relaxed),
                                             -72.f, 0.15f / decay_speed);
        }
        if (to_update_xs_.check()) {
            c_width_ = bound.getWidth();
            const auto fft_max = static_cast<float>(zlp::getEQFFTMax(sample_rate));
            const auto temp_scale = static_cast<float>(
                1.0 / std::log(fft_max / zlp::kEQMinFreq)) * c_width_;
            const auto temp_bias = std::log(zlp::kEQMinFreq) * temp_scale;
            num_point_ = xs_.size();
            xs_[0] = std::log(frequencies_[1] * .5f) * temp_scale - temp_bias;
            for (size_t i = 1; i < xs_.size(); ++i) {
                xs_[i] = std::log(frequencies_[i]) * temp_scale - temp_bias;
                if (xs_[i] > c_width_) {
                    num_point_ = i + 1;
                    break;
                }
            }
            if (!high_quality) {
                inter_xs_[0] = xs_[0];
                inter_xs_[kInterSize] = xs_[kInterSize / 2];
                inter_xs_[kInterSize + 1] = xs_[kInterSize / 2 + 1];
                const auto delta_inter_x = (xs_[kInterSize / 2 - 1] - xs_[0]) / static_cast<float>(kInterSize - 1);
                for (size_t i = 1; i < kInterSize; ++i) {
                    inter_xs_[i] = inter_xs_[i - 1] + delta_inter_x;
                }
            }
        }
        if (to_update_ys_.check()) {
            c_height_ = bound.getHeight();
            const auto inset = font_size_.load(std::memory_order::relaxed);
            const auto plot_height = std::max(c_height_ - 2.f * inset, 0.f);
            y_scale_ = plot_height / -72.f;
            y_bias_ = inset;
        }

        if (num_point_ < 3) {
            return;
        }

        if (high_quality) {
            for (size_t resolution = 0; resolution < kNumResolutions; ++resolution) {
                auto& resolution_spectrum = resolution_spectra_[resolution];
                receiver_.forward(processors_[resolution], zldsp::analyzer::StereoType::kStereoLR,
                                  resolution_spectrum);
                zldsp::vector::multiply(resolution_spectrum.data(), noise_power_scales_[resolution],
                                        resolution_spectrum.size());
                spectrum_smoothers_[resolution].smooth(resolution_spectrum);
            }
            zldsp::analyzer::SpectrumBlender::blend(
                spectrum_, frequencies_,
                resolution_spectra_[kLowResolution],
                resolution_spectra_[kMiddleResolution],
                resolution_spectra_[kHighResolution], sample_rate);
        } else {
            receiver_.forward(processors_[kMiddleResolution], zldsp::analyzer::StereoType::kStereoLR,
                              spectrum_);
            spectrum_smoothers_[kMiddleResolution].smooth(spectrum_);
        }
        zldsp::vector::sqr_mag_to_db(spectrum_.data(), spectrum_.size());
        spectrum_tilter_.tilt(std::span{spectrum_.data(), spectrum_.size()});
        spectrum_decayer_.decay(std::span{spectrum_.data(), spectrum_.size()},
                                is_fft_frozen_.load(std::memory_order::relaxed));
        zldsp::vector::fma(ys_.data(), spectrum_.data(), y_scale_, y_bias_, num_point_);

        auto& next_out_path{out_path_.get_writer()};
        next_out_path.clear();
        PathMinimizer<5> minimizer{next_out_path};
        next_out_path.startNewSubPath(xs_.front() - .1f, c_height_ * 1.5f);
        size_t first_point{0};
        if (!high_quality) {
            inter_->prepare();
            inter_->eval(inter_xs_.data(), inter_ys_.data(), inter_xs_.size());
            next_out_path.lineTo(inter_xs_[0], inter_ys_[0]);
            for (size_t i = 1; i < kInterSize; i += 2) {
                next_out_path.quadraticTo(inter_xs_[i], inter_ys_[i], inter_xs_[i + 1], inter_ys_[i + 1]);
            }
            next_out_path.lineTo(inter_xs_[kInterSize], inter_ys_[kInterSize]);
            first_point = kInterSize / 2;
        }
        minimizer.startNewSubPath<false>(xs_[first_point], ys_[first_point]);
        for (size_t i = first_point + 1; i < num_point_; ++i) {
            minimizer.lineTo(xs_[i], ys_[i]);
        }
        minimizer.finish();
        next_out_path.lineTo(xs_[num_point_ - 1] + .1f, c_height_ * 1.5f);
        next_out_path.closeSubPath();
        if (thread.threadShouldExit()) {
            return;
        }
        out_path_.publish();
    }

    void FFTAnalyzerPanel::setRefreshRate(const double refresh_rate) {
        refresh_rate_.store(static_cast<float>(refresh_rate), std::memory_order::relaxed);
        to_update_decay_.signal();
    }

    void FFTAnalyzerPanel::lookAndFeelChanged() {
        const auto extra_speed = base_.getFFTExtraSpeed();
        spectrum_extra_decay_speed_.store(extra_speed * extra_speed + .1f, std::memory_order::relaxed);
        to_update_decay_.signal();

        spectrum_tilt_slope_.store(4.5f + base_.getFFTExtraTilt(), std::memory_order::relaxed);
        to_update_tilt_.signal();
    }

    void FFTAnalyzerPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kFFTFrozen, property)) {
            is_fft_frozen_.store(
                static_cast<float>(base_.getPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen)) > .5f,
                std::memory_order::relaxed);
        }
    }
}
