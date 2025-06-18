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
        static constexpr float kMinFreq = 10.f, kMaxFreq = 22000.f, kMinDB = -256.f;
        static constexpr float kMinFreqLog2 = 3.321928094887362f;
        static constexpr float kMaxFreqLog2 = 14.425215903299383f;

    public:
        explicit MultipleFFTBase(const size_t fft_order = 12) {
            default_fft_order_ = fft_order;
            bin_size_ = (1 << (default_fft_order_ - 1)) + 1;

            for (auto &db: smoothed_dbs_) {
                db.resize(bin_size_);
            }

            prepareAkima();

            interplot_freqs_[0] = kMinFreq;
            for (size_t i = 1; i < PointNum; ++i) {
                const float temp = static_cast<float>(i) / static_cast<float>(PointNum - 1) * (
                                       kMaxFreqLog2 - kMinFreqLog2) + kMinFreqLog2;
                interplot_freqs_[i] = std::pow(2.f, temp);
            }
            for (auto &db: interplot_dbs_) {
                std::fill(db.begin(), db.end(), kMinDB * 2.f);
            }
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
            sample_rate_.store(static_cast<float>(sample_rate));
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
            is_prepared_.store(true, std::memory_order::release);
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

        /**
         * run the forward FFT and calculate the interpolated DBs
         */
        void run() {
            if (!is_prepared_.load(std::memory_order::acquire)) {
                return;
            }
            std::vector<size_t> is_on_vector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (is_on_[i].load()) is_on_vector.push_back(i);
            } {
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
                                  sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index1 + range.block_size1),
                                  circular_buffer.begin() + static_cast<std::ptrdiff_t>(num_replace));
                    }
                    if (range.block_size2 > 0) {
                        std::copy(sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index2),
                                  sample_fifo.begin() + static_cast<std::ptrdiff_t>(range.start_index2 + range.block_size2),
                                  circular_buffer.begin() + static_cast<std::ptrdiff_t>(num_replace + range.block_size1));
                    }
                }
                abstract_fifo_.finishRead(num_ready);
            } {
                for (const auto &i: is_on_vector) {
                    std::copy(circular_buffers_[i].begin(), circular_buffers_[i].end(), fft_buffer_.begin());
                    auto temp = kfr::make_univector(fft_buffer_.data(), window_.size());
                    temp = temp * window_;
                    fft_.forwardMagnitudeOnly(fft_buffer_.data());
                    const auto decay = actual_decay_rates_[i].load(std::memory_order::relaxed);
                    auto &smoothed_db{smoothed_dbs_[i]};
                    if (to_reset_[i].exchange(false)) {
                        std::fill(smoothed_db.begin(), smoothed_db.end(), kMinDB * 2.f);
                    }
                    for (size_t j = 0; j < smoothed_db.size(); ++j) {
                        const auto current_db = chore::gainToDecibels(fft_buffer_[j]);
                        smoothed_db[j] = current_db < smoothed_db[j]
                                             ? smoothed_db[j] * decay + current_db * (1 - decay)
                                             : current_db;
                    }
                    for (size_t j = 0; j < seq_input_dbs_.size(); ++j) {
                        const auto start_idx = seq_input_starts_[j];
                        const auto end_idx = seq_input_ends_[j];
                        seq_input_dbs_[j] = std::reduce(
                                                smoothed_db.begin() + start_idx,
                                                smoothed_db.begin() + end_idx) / static_cast<float>(
                                                end_idx - start_idx);
                    }
                    seq_akima_->prepare();
                    seq_akima_->eval(interplot_freqs_.data(), pre_interplot_dbs_[i].data(), PointNum);
                }
            }
            if (to_update_tilt_.exchange(false, std::memory_order::acquire)) {
                const float total_tilt = tilt_slope_.load() + extra_tilt_.load();
                const float tilt_shift_total = (kMaxFreqLog2 - kMinFreqLog2) * total_tilt;
                const float tilt_shift_delta = tilt_shift_total / static_cast<float>(PointNum - 1);
                float tilt_shift = -tilt_shift_total * .5f;
                for (size_t idx = 0; idx < PointNum; ++idx) {
                    tilt_shift_[idx] = tilt_shift;
                    tilt_shift += tilt_shift_delta;
                }
            } {
                for (const auto &i: is_on_vector) {
                    auto v1 = kfr::make_univector(interplot_dbs_[i]);
                    auto v2 = kfr::make_univector(tilt_shift_);
                    auto v3 = kfr::make_univector(pre_interplot_dbs_[i]);
                    v1 = v2 + v3;
                }
            }
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

    protected:
        size_t default_fft_order_ = 12;
        size_t bin_size_ = (1 << (default_fft_order_ - 1)) + 1;

        std::array<std::vector<float>, FFTNum> sample_fifos_;
        std::array<std::vector<float>, FFTNum> circular_buffers_;
        zldsp::container::AbstractFIFO abstract_fifo_{0};

        std::vector<float> fft_buffer_;

        // smooth dbs over time
        std::array<std::vector<float>, FFTNum> smoothed_dbs_{};
        // smooth dbs over high frequency for Akimas input
        std::vector<float> seq_input_freqs_{};
        std::vector<std::vector<float>::difference_type> seq_input_starts_, seq_input_ends_;
        std::vector<size_t> seq_input_indices_;
        std::vector<float> seq_input_dbs_{};

        std::unique_ptr<zldsp::interpolation::SeqMakima<float> > seq_akima_;

        std::array<float, PointNum> interplot_freqs_{};
        std::array<std::array<float, PointNum>, FFTNum> pre_interplot_dbs_{};
        std::array<std::array<float, PointNum>, FFTNum> interplot_dbs_{};

        std::atomic<float> delta_t_{1.f}, refresh_rate_{60}, tilt_slope_{4.5f};
        std::array<std::atomic<float>, FFTNum> decay_rates_{}, actual_decay_rates_{};
        std::atomic<float> extra_tilt_{0.f}, extra_speed_{1.f};

        std::array<float, PointNum> tilt_shift_{};
        std::atomic<bool> to_update_tilt_{true};

        zldsp::fft::KFREngine<float> fft_;
        kfr::univector<float> window_;

        std::atomic<float> sample_rate_{48000.f};
        std::array<std::atomic<bool>, FFTNum> to_reset_;
        std::atomic<bool> is_prepared_{false};

        std::array<std::atomic<bool>, FFTNum> is_on_{};

        void prepareAkima() {
            std::vector<size_t> seq_input_indices{};
            seq_input_indices.push_back(0);
            size_t i = 1, i0 = 1;
            const float delta = std::pow(
                static_cast<float>(bin_size_), .75f / static_cast<float>(PointNum));
            while (i < bin_size_ - 1) {
                while (static_cast<float>(i) / static_cast<float>(i0) < delta) {
                    i += 1;
                    if (i >= bin_size_ - 1) {
                        break;
                    }
                }
                i0 = i;
                seq_input_indices.push_back(i);
            }

            seq_input_starts_.reserve(seq_input_indices.size());
            seq_input_ends_.reserve(seq_input_indices.size());
            seq_input_starts_.push_back(0);
            seq_input_ends_.push_back(1);
            for (size_t idx = 1; idx < seq_input_indices.size() - 1; ++idx) {
                seq_input_starts_.push_back(seq_input_ends_.back());
                seq_input_ends_.push_back(
                    static_cast<std::vector<float>::difference_type>(
                        seq_input_indices[idx] + seq_input_indices[idx + 1]) / 2);
            }
            seq_input_starts_.push_back(seq_input_ends_.back());
            seq_input_ends_.push_back(static_cast<std::vector<float>::difference_type>(bin_size_) - 1);

            seq_input_freqs_.resize(seq_input_indices.size());
            seq_input_dbs_.resize(seq_input_indices.size());
            seq_akima_ = std::make_unique<zldsp::interpolation::SeqMakima<float> >(
                seq_input_freqs_.data(), seq_input_dbs_.data(), seq_input_freqs_.size(), 0.f, 0.f);
        }

        void setOrder(const int fft_order) {
            fft_.setOrder(static_cast<size_t>(fft_order));

            window_.resize(static_cast<size_t>(fft_.getSize()));
            zldsp::fft::fillCycleHanningWindow(window_, static_cast<size_t>(fft_.getSize()));
            const auto scale = 1.f / static_cast<float>(fft_.getSize());
            window_ = window_ * scale;

            delta_t_.store(sample_rate_.load() / static_cast<float>(fft_.getSize()));

            const auto currentDeltaT = .5f * delta_t_.load();
            for (size_t idx = 0; idx < seq_input_freqs_.size(); ++idx) {
                seq_input_freqs_[idx] = static_cast<float>(seq_input_starts_[idx] + seq_input_ends_[idx] - 1) *
                                        currentDeltaT;
            }
            for (size_t i = 0; i < FFTNum; ++i) {
                std::fill(smoothed_dbs_[i].begin(), smoothed_dbs_[i].end(), kMinDB * 2.f);
            }

            const auto temp_size = fft_.getSize();
            fft_buffer_.resize(temp_size * 2);
            abstract_fifo_.setCapacity(static_cast<int>(temp_size));
            for (size_t i = 0; i < FFTNum; ++i) {
                sample_fifos_[i].resize(temp_size);
                std::fill(sample_fifos_[i].begin(), sample_fifos_[i].end(), 0.f);
                circular_buffers_[i].resize(temp_size);
                std::fill(circular_buffers_[i].begin(), circular_buffers_[i].end(), 0.f);
            }
        }

        void updateActualDecayRate() {
            for (size_t i = 0; i < FFTNum; ++i) {
                const auto x = 1 - (1 - decay_rates_[i].load(std::memory_order::relaxed)
                               ) * extra_speed_.load(std::memory_order::relaxed);
                actual_decay_rates_[i].store(std::pow(x, 23.4375f / refresh_rate_.load()),
                                             std::memory_order::relaxed);
            }
        }
    };
}
