// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

namespace zlContainer {
    /**
     * a circular buffer
     * @tparam T the type of elements
     */
    template<typename T>
    class CircularBuffer {
    public:
        explicit CircularBuffer(const size_t capacity = 1) {
            data.resize(capacity);
            cap = static_cast<int>(capacity);
        }

        [[nodiscard]] size_t capacity() const { return data.size(); }

        [[nodiscard]] size_t size() const { return static_cast<size_t>(current_num); }

        void setCapacity(const size_t capacity) {
            data.resize(capacity);
            cap = static_cast<int>(capacity);
        }

        void clear() {
            std::fill(data.begin(), data.end(), T());
            pos = 0;
            current_num = 0;
        }

        void pushBack(T x) {
            data[static_cast<size_t>(pos)] = x;
            pos = (pos + 1) % cap;
            current_num = std::min(current_num + 1, cap);
        }

        T popFront() {
            const auto frontPos = (pos - current_num + cap) % cap;
            current_num -= 1;
            return data[static_cast<size_t>(frontPos)];
        }

    private:
        std::vector<T> data;
        int pos = 0, current_num = 0, cap = 0;
    };
}
