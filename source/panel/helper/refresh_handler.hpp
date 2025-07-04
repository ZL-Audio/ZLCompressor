// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <algorithm>

namespace zlpanel {
    class RefreshHandler {
    public:
        explicit RefreshHandler(const double target_refresh_rate) {
            target_refresh_interval_ = 1.0 / target_refresh_rate;
        }

        bool tick(const double time_stamp) {
            if (time_stamp_ < 0) {
                time_stamp_ = time_stamp;
                return true;
            } else {
                const auto c_diff = time_stamp - time_stamp_;

                if (std::abs(std::abs(avg_diff_ / c_diff) - 1.0) > 0.25) {
                    diff_sum_ = 0.0;
                    diff_count_ = 0.0;
                }

                diff_sum_ += c_diff;
                diff_count_ += 1.0;
                time_stamp_ = time_stamp;

                avg_diff_ = std::max(diff_sum_ / diff_count_, 1e-6);
                const auto space = target_refresh_interval_ / avg_diff_;

                const auto final_space = std::max(static_cast<size_t>(std::round(space)),
                                                  static_cast<size_t>(1));
                call_count_ = (call_count_ + 1) % final_space;
                return call_count_ == 0;
            }
        }

        [[nodiscard]] double getActualRefreshRate() const {
            return diff_sum_ > 0 ? diff_count_ / diff_sum_ : 1.0 / target_refresh_interval_;
        }

    private:
        double target_refresh_interval_{1.0};
        double time_stamp_{-1.0};

        double diff_sum_{0.0}, diff_count_{0.0};
        double avg_diff_{0.0};

        size_t call_count_{0};
    };
}
