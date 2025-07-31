// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <span>

#include "../container/container.hpp"
#include "../interpolation/interpolation.hpp"
#include "../fft/fft.hpp"
#include "../chore/decibels.hpp"
#include "../lock/spin_lock.hpp"

namespace zldsp::analyzer {
    /**
     * a fft analyzer which make sure that multiple FFTs are synchronized in time
     * @tparam FloatType the float type of input audio buffers
     * @tparam FFTNum the number of FFTs
     * @tparam PointNum the number of output points
     */
    template<typename FloatType, size_t FFTNum, size_t PointNum>
    class MultipleFFTBase {
    private:
        static constexpr float kMinDB = -256.f;

    public:
        explicit MultipleFFTBase(const size_t fft_order = 12) {
            default_fft_order_ = fft_order;

            reset();

            for (auto &d: decay_rates_) {
                d.store(0.95f);
            }
            for (auto &d: actual_decay_rates_) {
                d.store(0.95f);
            }
            updateActualDecayRate();
        }

        ~MultipleFFTBase() = default;

        void prepare(const double sample_rate) {
            lock_.lock();
            sample_rate_.store(static_cast<float>(sample_rate), std::memory_order::relaxed);
            if (sample_rate <= 50000) {
                setOrder(static_cast<int>(default_fft_order_));
            } else if (sample_rate <= 100000) {
                setOrder(static_cast<int>(default_fft_order_) + 1);
            } else if (sample_rate <= 200000) {
                setOrder(static_cast<int>(default_fft_order_) + 2);
            } else {
                setOrder(static_cast<int>(default_fft_order_) + 3);
            }
            reset();
            to_update_akima_.store(true, std::memory_order::release);
            lock_.unlock();
        }

        void reset() {
            for (auto &f: to_reset_) {
                f.store(true, std::memory_order::release);
            }
        }

        /**
         * put input samples into FIFOs
         * @param buffers
         * @param num_samples
         */
        void process(std::array<std::span<FloatType *>, FFTNum> buffers, const size_t num_samples) {
            int free_space = abstract_fifo_.getNumFree();
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!is_on_[i].load(std::memory_order::relaxed)) continue;
                free_space = std::min(free_space, static_cast<int>(num_samples));
            }
            if (free_space == 0) { return; }
            const auto range = abstract_fifo_.prepareToWrite(free_space);
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!is_on_[i].load()) continue;
                int j = 0;
                const auto buffer = buffers[i];
                int shift = 0;
                for (; j < range.block_size1; ++j) {
                    FloatType sample{0};
                    for (size_t channel = 0; channel < buffer.size(); ++channel) {
                        sample += buffer[channel][static_cast<size_t>(j)];
                    }
                    sample_fifos_[i][static_cast<size_t>(shift + range.start_index1)] = static_cast<float>(sample);
                    shift += 1;
                }
                shift = 0;
                for (; j < range.block_size1 + range.block_size2; ++j) {
                    FloatType sample{0};
                    for (size_t channel = 0; channel < buffer.size(); ++channel) {
                        sample += buffer[channel][static_cast<size_t>(j)];
                    }
                    sample_fifos_[i][static_cast<size_t>(shift + range.start_index2)] = static_cast<float>(sample);
                    shift += 1;
                }
            }
            abstract_fifo_.finishWrite(free_space);
        }

        size_t getInterplotSize() const {
            return interplot_freqs_.size();
        }

        /**
         * run the forward FFT and calculate the interpolated DBs
         * @return whether to update
         */
        bool run() {
            bool to_update{false};
            // cache on indices
            std::vector<size_t> is_on_vector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (is_on_[i].load()) is_on_vector.push_back(i);
            }
            // pull data from FIFO
            const int num_ready = abstract_fifo_.getNumReady();
            const auto range = abstract_fifo_.prepareToRead(num_ready);
            const auto num_replace = static_cast<int>(circular_buffers_[0].size()) - num_ready;
            for (const auto &i: is_on_vector) {
                auto &circular_buffer{circular_buffers_[i]};
                auto &sample_fifo{sample_fifos_[i]};
                std::memmove(circular_buffer.data(),
                             circular_buffer.data() + static_cast<std::ptrdiff_t>(num_ready),
                             sizeof(float) * static_cast<size_t>(num_replace));
                if (range.block_size1 > 0) {
                    std::copy(sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index1),
                              sample_fifo.begin() + static_cast<std::ptrdiff_t>(
                                  range.start_index1 + range.block_size1),
                              circular_buffer.begin() + static_cast<std::ptrdiff_t>(num_replace));
                }
                if (range.block_size2 > 0) {
                    std::copy(sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index2),
                              sample_fifo.begin() + static_cast<std::ptrdiff_t>(
                                  range.start_index2 + range.block_size2),
                              circular_buffer.begin() + static_cast<std::ptrdiff_t>(
                                  num_replace + range.block_size1));
                }
            }
            abstract_fifo_.finishRead(num_ready);
            // prepare Akima
            if (to_update_akima_.exchange(false, std::memory_order::acquire)) {
                prepareAkima();
                to_update = true;
            }
            // run forward FFT & interpolate
            for (const auto &i: is_on_vector) {
                std::copy(circular_buffers_[i].begin(), circular_buffers_[i].end(), fft_buffer_.begin());
                auto temp = kfr::make_univector(fft_buffer_.data(), window_.size());
                temp = temp * window_;
                fft_.forwardMagnitudeOnly(fft_buffer_.data());
                const auto decay = actual_decay_rates_[i].load(std::memory_order::relaxed);
                auto &input_dbs{seq_input_dbs_[i]};
                if (to_reset_[i].exchange(false)) {
                    std::fill(input_dbs.begin(), input_dbs.end(), kMinDB);
                }
                for (size_t j = 0; j < input_dbs.size(); ++j) {
                    const auto start_idx = seq_input_starts_[j];
                    const auto end_idx = seq_input_ends_[j];
                    float mean_square = 0.0;
                    for (size_t k = start_idx; k < end_idx; ++k) {
                        mean_square += fft_buffer_[k] * fft_buffer_[k];
                    }
                    mean_square = mean_square / static_cast<float>(end_idx - start_idx);
                    const auto current_db = chore::squareGainToDecibels(mean_square);
                    input_dbs[j] = current_db < input_dbs[j]
                                          ? input_dbs[j] * decay + current_db * (1 - decay)
                                          : current_db;
                }
                seq_akima_[i]->prepare();
                seq_akima_[i]->eval(interplot_freqs_.data(), interplot_dbs_[i].data(), interplot_freqs_.size());
            }
            // apply tilt
            if (to_update_tilt_.exchange(false, std::memory_order::acquire)) {
                prepareTilt();
            }
            for (const auto &i: is_on_vector) {
                auto v1 = kfr::make_univector(interplot_dbs_[i]);
                auto v2 = kfr::make_univector(tilt_shift_);
                auto v3 = kfr::make_univector(result_dbs_[i]);
                v3 = v1 + v2;
            }
            return to_update;
        }

        void setON(std::array<bool, FFTNum> fs) {
            for (size_t i = 0; i < FFTNum; ++i) {
                is_on_[i].store(fs[i]);
            }
        }

        void setDecayRate(const size_t idx, const float x) {
            decay_rates_[idx].store(x, std::memory_order::relaxed);
            updateActualDecayRate();
        }

        void setRefreshRate(const float x) {
            refresh_rate_.store(x, std::memory_order::relaxed);
            updateActualDecayRate();
        }

        void setTiltSlope(const float x) {
            tilt_slope_.store(x, std::memory_order::relaxed);
            to_update_tilt_.store(true, std::memory_order::release);
        }

        void setExtraTilt(const float x) {
            extra_tilt_.store(x, std::memory_order::relaxed);
            to_update_tilt_.store(true, std::memory_order::release);
        }

        void setExtraSpeed(const float x) {
            extra_speed_.store(x, std::memory_order::relaxed);
            updateActualDecayRate();
        }

        void setMinMaxFreq(const double min_freq, const double max_freq) {
            min_freq_.store(min_freq, std::memory_order::relaxed);
            max_freq_.store(max_freq, std::memory_order::relaxed);
            to_update_akima_.store(true, std::memory_order::release);
        }

        zldsp::lock::SpinLock &getLock() { return lock_; }

    protected:
        size_t default_fft_order_ = 12;
        zldsp::lock::SpinLock lock_;

        std::array<std::vector<float>, FFTNum> sample_fifos_;
        std::array<std::vector<float>, FFTNum> circular_buffers_;
        zldsp::container::AbstractFIFO abstract_fifo_{0};
        std::vector<float> fft_buffer_;

        // smooth dbs over high frequency for Akimas input
        std::atomic<double> sample_rate_{48000.}, min_freq_{10.}, max_freq_{22000.};
        std::atomic<bool> to_update_akima_{true};
        std::vector<float> seq_input_freqs_{};
        std::vector<size_t> seq_input_starts_, seq_input_ends_;
        std::array<std::vector<float>, FFTNum> seq_input_dbs_{};

        std::array<std::unique_ptr<zldsp::interpolation::SeqMakima<float> >, FFTNum> seq_akima_;

        std::vector<float> interplot_freqs_{}, interplot_freqs_p_{};
        std::array<std::vector<float>, FFTNum> interplot_dbs_{};
        std::array<std::vector<float>, FFTNum> result_dbs_{};

        std::atomic<float> refresh_rate_{60}, tilt_slope_{4.5f};
        std::array<std::atomic<float>, FFTNum> decay_rates_{}, actual_decay_rates_{};
        std::atomic<float> extra_tilt_{0.f}, extra_speed_{1.f};

        std::vector<float> tilt_shift_{};
        std::atomic<bool> to_update_tilt_{true};

        zldsp::fft::KFREngine<float> fft_;
        kfr::univector<float> window_;

        std::array<std::atomic<bool>, FFTNum> to_reset_;

        std::array<std::atomic<bool>, FFTNum> is_on_{};

        void prepareAkima() {
            // cache sample rate and frequency values
            const auto sample_rate = sample_rate_.load(std::memory_order::relaxed);
            const auto min_freq = min_freq_.load(std::memory_order::relaxed);
            const auto max_freq = std::min(max_freq_.load(std::memory_order::relaxed), sample_rate * .5);
            const auto fft_size = fft_.getSize();
            // calculate start/end indices
            {
                const auto freq_delta = sample_rate / static_cast<double>(fft_size);
                const auto freq_mul = std::pow(max_freq / min_freq, 1. / static_cast<double>(PointNum / 2));
                auto freq = min_freq * std::sqrt(freq_mul);
                seq_input_starts_.clear();
                seq_input_starts_.reserve(PointNum / 2);
                seq_input_ends_.clear();
                seq_input_ends_.reserve(PointNum / 2);

                seq_input_starts_.emplace_back(0);
                seq_input_ends_.emplace_back(
                    std::max(static_cast<size_t>(std::round(freq / freq_delta)),
                             static_cast<size_t>(1)));
                const auto limit = fft_size / 2;
                for (size_t i = 1; i < PointNum / 2 + 1; ++i) {
                    freq *= freq_mul;
                    const auto new_index = std::min(
                        static_cast<size_t>(std::round(freq / freq_delta)), limit);
                    if (new_index > seq_input_ends_.back()) {
                        seq_input_starts_.emplace_back(seq_input_ends_.back());
                        seq_input_ends_.emplace_back(new_index);
                    }
                }
                seq_input_starts_.emplace_back(seq_input_ends_.back());
                seq_input_ends_.emplace_back(fft_size / 2 + 1);
            }
            // calculate medium of each start-end range, which is served as Akima input x
            {
                const auto freq_delta = 0.5 * sample_rate / static_cast<double>(fft_size);
                seq_input_freqs_.clear();
                seq_input_freqs_.resize(seq_input_starts_.size());
                for (size_t i = 0; i < seq_input_starts_.size(); ++i) {
                    seq_input_freqs_[i] = static_cast<float>(
                        static_cast<double>(seq_input_starts_[i] + seq_input_ends_[i] - 1) * freq_delta);
                }
                for (size_t i = 0; i < FFTNum; ++i) {
                    seq_input_dbs_[i].resize(seq_input_freqs_.size());
                    seq_akima_[i] = std::make_unique<zldsp::interpolation::SeqMakima<float> >(
                        seq_input_freqs_.data(), seq_input_dbs_[i].data(), seq_input_freqs_.size(),
                        0.f, 0.f);
                }
            }
            // calculate the Akima output x
            {
                auto freq = min_freq;
                const auto freq_mul = std::pow(max_freq / min_freq, 1.1 / static_cast<double>(PointNum));

                interplot_freqs_.clear();
                interplot_freqs_.reserve(PointNum);
                for (size_t i = 1; i < seq_input_freqs_.size(); ++i) {
                    while (freq < seq_input_freqs_[i]) {
                        interplot_freqs_.emplace_back(static_cast<float>(freq));
                        freq *= freq_mul;
                    }
                    if (std::abs(seq_input_freqs_[i] - interplot_freqs_.back()) > 1e-3f) {
                        freq = seq_input_freqs_[i];
                    }
                }

                interplot_freqs_p_.resize(interplot_freqs_.size());
                for (size_t i = 0; i < interplot_freqs_.size(); ++i) {
                    interplot_freqs_p_[i] = std::log(interplot_freqs_[i] / static_cast<float>(min_freq)
                        ) / static_cast<float>(std::log(max_freq / min_freq));
                }

                for (size_t i = 0; i < FFTNum; ++i) {
                    interplot_dbs_[i].resize(interplot_freqs_.size());
                    result_dbs_[i].resize(interplot_freqs_.size());
                }
                tilt_shift_.resize(interplot_freqs_.size());
            }
            reset();
        }

        void prepareTilt() {
            float final_tilt = tilt_slope_.load(std::memory_order::relaxed);
            final_tilt += extra_tilt_.load(std::memory_order::relaxed);

            const auto sample_rate = sample_rate_.load(std::memory_order::relaxed);
            const auto min_freq = min_freq_.load(std::memory_order::relaxed);
            const auto max_freq = std::min(max_freq_.load(std::memory_order::relaxed), sample_rate * .5);

            const float total_tilt = static_cast<float>(std::log2(max_freq / min_freq)) * final_tilt;

            for (size_t i = 0; i < tilt_shift_.size(); ++i) {
                tilt_shift_[i] = (interplot_freqs_p_[i] - .5f) * total_tilt;
            }
        }

        void setOrder(const int fft_order) {
            fft_.setOrder(static_cast<size_t>(fft_order));
            const auto fft_size = fft_.getSize();

            window_.resize(static_cast<size_t>(fft_size));
            zldsp::fft::fillCycleHanningWindow(window_, static_cast<size_t>(fft_size));
            const auto scale = 1.f / static_cast<float>(fft_size);
            window_ = window_ * scale;

            fft_buffer_.resize(fft_size * 2);
            abstract_fifo_.setCapacity(static_cast<int>(fft_size));
            for (size_t i = 0; i < FFTNum; ++i) {
                sample_fifos_[i].resize(fft_size);
                std::fill(sample_fifos_[i].begin(), sample_fifos_[i].end(), 0.f);
                circular_buffers_[i].resize(fft_size);
                std::fill(circular_buffers_[i].begin(), circular_buffers_[i].end(), 0.f);
            }
        }

        void updateActualDecayRate() {
            for (size_t i = 0; i < FFTNum; ++i) {
                const auto x = 1 - (1 - decay_rates_[i].load(std::memory_order::relaxed)
                               ) * extra_speed_.load(std::memory_order::relaxed);
                actual_decay_rates_[i].store(
                    std::pow(x, 23.4375f / refresh_rate_.load(std::memory_order::relaxed)),
                    std::memory_order::relaxed);
            }
        }
    };
}
