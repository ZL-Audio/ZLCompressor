// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

namespace zlContainer {
    /**
     * an array which has a fixed maximum size (capacity)
     * @tparam T the type of elements
     * @tparam N the capacity of the array
     */
    template<typename T, size_t N>
    class FixedMaxSizeArray {
    public:
        FixedMaxSizeArray() = default;

        FixedMaxSizeArray &operator =(const FixedMaxSizeArray<T, N> &that) {
            for (size_t i = 0; i < mSize; ++i) {
                data[i] = that.data[i];
            }
            return *this;
        }

        T &operator [](const size_t idx) const {
            return data[idx];
        }

        void push(const T x) {
            if (mSize == N) {
                mSize = 0;
            }
            data[mSize] = x;
            mSize++;
        }

        void clear() {
            mSize = 0;
        }

        [[nodiscard]] size_t size() const {
            return mSize;
        }

        [[nodiscard]] static size_t capacity() {
            return N;
        }

    private:
        std::array<T, N> data{};
        size_t mSize = 0;
    };
}
