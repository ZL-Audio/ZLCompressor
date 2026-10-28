// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <cstddef>

namespace zlpanel {
    inline constexpr double kUnsetHistoryTimeStamp = -1.0;

    inline bool isValidHistoryTimeStamp(const double time_stamp) {
        return std::isfinite(time_stamp) && time_stamp >= 0.0;
    }

    inline bool hasHistoryTimeDiscontinuity(const double next_time_stamp,
                                           const double previous_time_stamp,
                                           const double history_length) {
        return isValidHistoryTimeStamp(next_time_stamp) && isValidHistoryTimeStamp(previous_time_stamp) &&
               (next_time_stamp < previous_time_stamp || next_time_stamp - previous_time_stamp > history_length);
    }

    struct HistoryAdvance {
        size_t num_points;
        double time_stamp;
        bool needs_reset;
    };

    // calculate a bounded history update before consuming samples
    inline HistoryAdvance getHistoryAdvance(const double next_time_stamp, const double point_time_stamp,
                                            const double second_per_point, const size_t max_num_points) {
        const HistoryAdvance reset{0, next_time_stamp, true};
        if (!isValidHistoryTimeStamp(next_time_stamp) || !isValidHistoryTimeStamp(point_time_stamp) ||
            !std::isfinite(second_per_point) || second_per_point <= 0.0 ||
            next_time_stamp < point_time_stamp ||
            next_time_stamp - point_time_stamp > second_per_point * static_cast<double>(max_num_points)) {
            return reset;
        }

        HistoryAdvance advance{0, point_time_stamp, false};
        while (advance.num_points < max_num_points && next_time_stamp - advance.time_stamp > second_per_point) {
            const auto next_point_time = advance.time_stamp + second_per_point;
            if (!std::isfinite(next_point_time) || next_point_time <= advance.time_stamp) {
                return reset;
            }
            advance.time_stamp = next_point_time;
            ++advance.num_points;
        }
        return next_time_stamp - advance.time_stamp > second_per_point ? reset : advance;
    }
}
