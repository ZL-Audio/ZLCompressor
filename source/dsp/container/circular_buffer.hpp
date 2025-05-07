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
        }

        [[nodiscard]] size_t capacity() const { return data.size(); }

        [[nodiscard]] size_t size() const { return static_cast<size_t>(currentNum); }

        void setCapacity(const size_t capacity) {
            data.resize(capacity);
        }

        void clear() {
            std::fill(data.begin(), data.end(), T());
            pos = 0;
            currentNum = 0;
        }

        void pushBack(T x) {
            data[static_cast<size_t>(pos)] = x;
            pos = (pos + 1) % static_cast<int>(data.size());
            currentNum = std::min(currentNum + 1, static_cast<int>(data.size()));
        }

        T popFront() {
            const auto frontPos = (pos - currentNum + static_cast<int>(data.size())) % static_cast<int>(data.size());
            currentNum -= 1;
            return data[static_cast<size_t>(frontPos)];
        }

    private:
        std::vector<T> data;
        int pos = 0, currentNum = 0;
    };
}
