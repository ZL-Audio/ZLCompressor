// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <limits>

namespace zlpanel {
    class MagDBRange {
    public:
        constexpr MagDBRange(const float max_db, const float range_db) noexcept :
            max_db_(max_db), range_db_(range_db) {
        }

        [[nodiscard]] constexpr float getMaxDB() const noexcept {
            return max_db_;
        }

        [[nodiscard]] constexpr float getMinDB() const noexcept {
            return max_db_ + range_db_;
        }

        [[nodiscard]] constexpr float getRangeDB() const noexcept {
            return range_db_;
        }

        template <typename FloatType>
        [[nodiscard]] constexpr FloatType getYProportion(const FloatType db) const noexcept {
            const auto range_db = static_cast<FloatType>(range_db_);
            if (range_db > -std::numeric_limits<FloatType>::epsilon() &&
                range_db < std::numeric_limits<FloatType>::epsilon()) {
                return FloatType{};
            }
            return (db - static_cast<FloatType>(max_db_)) / range_db;
        }

        template <typename FloatType>
        [[nodiscard]] constexpr FloatType getReductionYProportion(const FloatType reduction_db) const noexcept {
            const auto range_db = static_cast<FloatType>(range_db_);
            if (range_db > -std::numeric_limits<FloatType>::epsilon() &&
                range_db < std::numeric_limits<FloatType>::epsilon()) {
                return FloatType{};
            }
            return reduction_db / range_db;
        }

        template <typename FloatType>
        [[nodiscard]] constexpr FloatType getDBAtYProportion(const FloatType proportion) const noexcept {
            return static_cast<FloatType>(max_db_) + proportion * static_cast<FloatType>(range_db_);
        }

    private:
        float max_db_;
        float range_db_;
    };
}
