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
#include <algorithm>

namespace zlContainer {
    /**
     * a circular buffer
     * @tparam T the type of elements
     */
    template<typename T, bool findMin, bool findMax>
    class CircularMinMaxBuffer {
    public:
        explicit CircularMinMaxBuffer(const size_t capacity = 1) {
            static_assert(findMin != findMax);
            data.resize(capacity);
        }

        [[nodiscard]] size_t capacity() const { return data.size(); }

        void setCapacity(const size_t capacity) {
            data.resize(capacity);
        }

        void setSize(const int x) {
            if (x < size) {
                const auto shift = static_cast<size_t>(size - x);
                for (size_t i = 0; i < static_cast<size_t>(x); ++i) {
                    data[i] = data[i + shift];
                }
            }
            size = x;
            if (findMin) {
                std::fill(data.begin() + size, data.end(), static_cast<T>(1e6));
            }
            if (findMax) {
                std::fill(data.begin() + size, data.end(), static_cast<T>(-1e6));
            }
            updateMinMaxPos();
        }

        void clear() {
            std::fill(data.begin(), data.end(), T());
            pos = 0;
            minmaxPos = 0;
        }

        T push(T x) {
            data[static_cast<size_t>(pos)] = x;
            if (findMin) {
                if (x < minmaxValue) {
                    minmaxValue = x;
                    minmaxPos = pos;
                } else if (pos == minmaxPos) {
                    updateMinMaxPos();
                }
            }
            if (findMax) {
                if (x > minmaxValue) {
                    minmaxValue = x;
                    minmaxPos = pos;
                } else if (pos == minmaxPos) {
                    updateMinMaxPos();
                }
            }
            pos = (pos + 1) % size;
            return minmaxValue;
        }

    private:
        std::vector<T> data;
        int pos = 0, minmaxPos = 0, size = 0;
        T minmaxValue;

        void updateMinMaxPos() {
            if (findMin) {
                const auto *minElement = std::min_element(data.begin(), data.begin() + size);
                minmaxValue = *minElement;
                minmaxPos = static_cast<int>(std::distance(data.begin(), minElement));
            }
            if (findMax) {
                const auto *maxElement = std::max_element(data.begin(), data.begin() + size);
                minmaxValue = *maxElement;
                minmaxPos = static_cast<int>(std::distance(data.begin(), maxElement));
            }
        }
    };
}
