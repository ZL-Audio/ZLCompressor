// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "multiple_mag_analyzer.hpp"

namespace zldsp::analyzer {
    template <typename FloatType, size_t PointNum>
    class MagReductionAnalyzer : public MultipleMagAnalyzer<FloatType, 3, PointNum> {
    public:
        explicit MagReductionAnalyzer() : MultipleMagAnalyzer<FloatType, 3, PointNum>() {
        }

        void createReductionPath(std::span<float> xs,
                                 std::span<float> in_ys, std::span<float> out_ys, std::span<float> reduction_ys,
                                 const float width, const float height, const float shift = 0.f,
                                 const float min_db = -72.f, const float max_db = 0.f) {
            this->createPath(xs, {in_ys, reduction_ys, out_ys,}, width, height, shift, min_db, max_db);
            auto reduction_vector = kfr::make_univector(reduction_ys);
            auto in_vector = kfr::make_univector(in_ys);
            reduction_vector = reduction_vector - in_vector;
        }
    };
}
