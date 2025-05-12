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

namespace zldsp::container {
    /**
     * a circular buffer that can track the min/max value
     * @tparam T the type of elements
     * @tparam FindMin
     * @tparam FindMax
     */
    template<typename T, bool FindMin, bool FindMax>
    class CircularMinMaxBuffer {
    public:
        explicit CircularMinMaxBuffer(const size_t capacity = 1) {
            static_assert(FindMin != FindMax);
            data_.resize_(capacity);
        }

        [[nodiscard]] size_t capacity() const { return data_.size_(); }

        void setCapacity(const size_t capacity) {
            data_.resize_(capacity);
        }

        void setSize(const int x) {
            if (x < size_) {
                const auto shift = static_cast<size_t>(size_ - x);
                for (size_t i = 0; i < static_cast<size_t>(x); ++i) {
                    data_[i] = data_[i + shift];
                }
            }
            size_ = x;
            if (FindMin) {
                std::fill(data_.begin() + size_, data_.end(), static_cast<T>(1e6));
            }
            if (FindMax) {
                std::fill(data_.begin() + size_, data_.end(), static_cast<T>(-1e6));
            }
            updateMinMaxPos();
        }

        void clear() {
            std::fill(data_.begin(), data_.end(), T());
            pos_ = 0;
            minmax_pos_ = 0;
        }

        T push(T x) {
            data_[static_cast<size_t>(pos_)] = x;
            if (FindMin) {
                if (x < minmax_value_) {
                    minmax_value_ = x;
                    minmax_pos_ = pos_;
                } else if (pos_ == minmax_pos_) {
                    updateMinMaxPos();
                }
            }
            if (FindMax) {
                if (x > minmax_value_) {
                    minmax_value_ = x;
                    minmax_pos_ = pos_;
                } else if (pos_ == minmax_pos_) {
                    updateMinMaxPos();
                }
            }
            pos_ = (pos_ + 1) % size_;
            return minmax_value_;
        }

    private:
        std::vector<T> data_;
        int pos_ = 0, minmax_pos_ = 0, size_ = 0;
        T minmax_value_;

        void updateMinMaxPos() {
            if (FindMin) {
                const auto *min_element = std::min_element(data_.begin(), data_.begin() + size_);
                minmax_value_ = *min_element;
                minmax_pos_ = static_cast<int>(std::distance(data_.begin(), min_element));
            }
            if (FindMax) {
                const auto *max_element = std::max_element(data_.begin(), data_.begin() + size_);
                minmax_value_ = *max_element;
                minmax_pos_ = static_cast<int>(std::distance(data_.begin(), max_element));
            }
        }
    };
}
