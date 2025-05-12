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
    template<typename FloatType, size_t PointNum>
    class MagReductionAnalyzer : public MultipleMagAnalyzer<FloatType, 2, PointNum> {
    public:
        explicit MagReductionAnalyzer() : MultipleMagAnalyzer<FloatType, 2, PointNum>() {
        }

        template<bool CloseInPath = false, bool CloseOutPath = false>
        void createPath(juce::Path &in_path, juce::Path &out_path, juce::Path &reduction_path,
                       const juce::Rectangle<float> bound,
                       const float shift = 0.f, const float reduction_bias = 0.f,
                       const float min_db = -72.f, const float max_db = 0.f) {
            const auto delta_x = bound.getWidth() / static_cast<float>(PointNum - 1);
            const auto x0 = bound.getX(), y0 = bound.getY(), height = bound.getHeight();
            float x = x0 - shift * delta_x;
            {
                const auto in_y = this->magToY(this->circular_mags_[0][0], y0, height, min_db, max_db);
                const auto out_y = this->magToY(this->circular_mags_[1][0], y0, height, min_db, max_db);
                if (CloseInPath) {
                    in_path.startNewSubPath(x, bound.getBottom());
                    in_path.lineTo(x, in_y);
                } else {
                    in_path.startNewSubPath(x, in_y);
                }
                if (CloseOutPath) {
                    out_path.startNewSubPath(x, bound.getBottom());
                    out_path.lineTo(x, out_y);
                } else {
                    out_path.startNewSubPath(x, out_y);
                }
                reduction_path.startNewSubPath(x, out_y - in_y - reduction_bias);
                x += delta_x;
            }
            for (size_t idx = 1; idx < PointNum; ++idx) {
                const auto in_y = this->magToY(this->circular_mags_[0][idx], y0, height, min_db, max_db);
                const auto out_y = this->magToY(this->circular_mags_[1][idx], y0, height, min_db, max_db);
                in_path.lineTo(x, in_y);
                out_path.lineTo(x, out_y);
                reduction_path.lineTo(x, out_y - in_y - reduction_bias);
                x += delta_x;
            }
            if (CloseInPath) {
                in_path.lineTo(x - delta_x, bound.getBottom());
                in_path.closeSubPath();
            }
            if (CloseOutPath) {
                out_path.lineTo(x - delta_x, bound.getBottom());
                out_path.closeSubPath();
            }
        }
    };
}
