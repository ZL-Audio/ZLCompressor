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
        constexpr auto preallocateSpace = static_cast<int>(zlp::EqualizeController::kAnalyzerPointNum) * 3 + 1;
        for (auto& path : {&out_path_, &next_out_path_}) {
            path->preallocateSpace(preallocateSpace);
        }
        receiver_.setON({true});
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
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        g.setColour(base_.getTextColour().withAlpha(.375f));
        g.fillPath(out_path_);
    }

    void FFTAnalyzerPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound);
        skip_next_repaint_ = true;
    }

    void FFTAnalyzerPanel::run() {
        const auto bound = atomic_bound_.load();
        bool to_update_xs_{false};
        auto& sender{p_ref_.getEqualizeController().getFFTAnalyzerSender()};
        if (!sender.getLock().try_lock()) {
            return;
        }
        const auto sample_rate = sender.getSampleRate();
        const auto fft_order = sender.getFFTOrder();
        const auto fft_size = static_cast<size_t>(1) << fft_order;
        if (c_fft_order_ != fft_order) {
            c_fft_order_ = fft_order;
            receiver_.prepare(static_cast<int>(fft_order), {1});
            spectrum_smoother_.prepare(fft_size);
            spectrum_smoother_.setSmooth(0.1);
            spectrum_tilter_.prepare(fft_size);
            spectrum_decayer_.prepare(fft_size);

            xs_.resize(fft_size / 2 + 1);
            ys_.resize(fft_size / 2 + 1);

            to_update_xs_ = true;
        }

        auto& fifo{sender.getAbstractFIFO()};
        const auto num_read = fifo.getNumReady();
        const auto range = fifo.prepareToRead(num_read);
        receiver_.pull(range, sender.getSampleFIFOs());
        fifo.finishRead(num_read);
        sender.getLock().unlock();

        if (fft_size == 0) {
            return;
        }
        receiver_.forward(zldsp::analyzer::StereoType::kStereo);

        auto& spectrum{receiver_.getAbsSqrFFTBuffers()[0]};
        spectrum_smoother_.smooth(spectrum);

        if (std::abs(c_sample_rate_ - sample_rate) > 0.1) {
            c_sample_rate_ = sample_rate;
            to_update_tilt_.store(true, std::memory_order::relaxed);
            to_update_xs_ = true;
        }
        if (to_update_tilt_.exchange(false, std::memory_order::acquire)) {
            spectrum_tilter_.setTiltSlope(sample_rate,
                                          spectrum_tilt_slope_.load(std::memory_order::relaxed));
        }
        if (to_update_xs_ || std::abs(bound.getWidth() - c_width_) > 0.01f) {
            c_width_ = bound.getWidth();
            const auto delta_freq = static_cast<float>(sample_rate / static_cast<double>(fft_size));
            const auto temp_scale = static_cast<float>(1.0 / std::log(22000.0 / 10.0)) * bound.getWidth();
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            for (size_t i = 1; i < xs_.size(); ++i) {
                const auto freq = delta_freq * static_cast<float>(i);
                xs_[i] = std::log(freq) * temp_scale - temp_bias;
                if (xs_[i] > c_width_) {
                    num_point_ = i + 1;
                    break;
                }
            }
            xs_[0] = std::min(0.f, xs_[2] - 2.f * xs_[1]);
        }
        if (to_update_decay_.exchange(false, std::memory_order::acquire)) {
            spectrum_decayer_.setDecaySpeed(refresh_rate_.load(std::memory_order::relaxed),
                                            spectrum_decay_speed_.load(std::memory_order::relaxed));
        }

        if (num_point_ < 3) {
            return;
        }
        auto spectrum_slice = spectrum.slice(0, num_point_);
        spectrum_slice = 10.f * kfr::log10(kfr::max(spectrum_slice, 1e-24f));
        spectrum_tilter_.tilt(spectrum_slice);
        spectrum_decayer_.decay(spectrum_slice, is_fft_frozen_.load(std::memory_order::relaxed));

        auto y_v = kfr::make_univector(ys_.data(), num_point_);
        y_v = spectrum_slice * (bound.getHeight() / -72.f);
        next_out_path_.clear();
        PathMinimizer<200> minimizer{next_out_path_};
        const auto num_accu = static_cast<size_t>(std::sqrt(static_cast<float>(num_point_)));
        next_out_path_.startNewSubPath(xs_.front() - .1f, bound.getBottom() * 1.5f);
        for (size_t i = 0; i < num_accu; ++i) {
            next_out_path_.lineTo(xs_[i], ys_[i]);
        }
        minimizer.startNewSubPath<false>(xs_[num_accu], ys_[num_accu]);
        for (size_t i = num_accu + 1; i < num_point_; ++i) {
            minimizer.lineTo(xs_[i], ys_[i]);
        }
        minimizer.finish();
        next_out_path_.lineTo(xs_[num_point_ - 1] + .1f, bound.getBottom() * 1.5f);
        next_out_path_.closeSubPath();

        std::lock_guard<std::mutex> lock{mutex_};
        out_path_.swapWithPath(next_out_path_);
    }

    void FFTAnalyzerPanel::setRefreshRate(const double refresh_rate) {
        refresh_rate_.store(static_cast<float>(refresh_rate), std::memory_order::relaxed);
        to_update_decay_.store(true, std::memory_order::release);
    }

    void FFTAnalyzerPanel::lookAndFeelChanged() {
        spectrum_decay_speed_.store(base_.getFFTExtraSpeed() * (-20.f), std::memory_order::relaxed);
        to_update_decay_.store(true, std::memory_order::release);

        spectrum_tilt_slope_.store(4.5f + base_.getFFTExtraTilt(), std::memory_order::relaxed);
        to_update_tilt_.store(true, std::memory_order::release);
    }

    void FFTAnalyzerPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kFFTFrozen, property)) {
            is_fft_frozen_.store(
                static_cast<float>(base_.getPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen)) > .5f,
                std::memory_order::relaxed);
        }
    }
}
