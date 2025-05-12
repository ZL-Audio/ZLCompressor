// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

namespace zldsp::splitter {
    /**
     * a splitter that splits the stereo audio signal input mid signal and side signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class MSSplitter {
    public:
        MSSplitter() = default;

        /**
         * switch left/right buffer to mid/side buffer
         */
        static constexpr void split(FloatType *l_buffer, FloatType *r_buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto l = l_buffer[i];
                const auto r = r_buffer[i];
                l_buffer[i] = (l + r) * FloatType(0.5);
                r_buffer[i] = l_buffer[i] - r_buffer[i];
            }
        }

        /**
         * switch mis/side buffer to left/right buffer
         */
        static constexpr void combine(FloatType *l_buffer, FloatType *r_buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto l = l_buffer[i];
                const auto r = r_buffer[i];
                l_buffer[i] = l + r;
                r_buffer[i] = l - r;
            }
        }
    };
}
