// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../computer/computer.hpp"
#include "../tracker/tracker.hpp"
#include "../follower/follower.hpp"

#include "../../vector/vector.hpp"

namespace zldsp::compressor {
    template<typename FloatType>
    class CompressorStyleBase {
    public:
        CompressorStyleBase(ComputerBase<FloatType> &computer,
                            RMSTracker<FloatType> &tracker,
                            FollowerBase<FloatType> &follower)
            : computer_(computer), tracker_(tracker), follower_(follower) {
        }

        virtual ~CompressorStyleBase() = default;

    protected:
        ComputerBase<FloatType> &computer_;
        RMSTracker<FloatType> &tracker_;
        FollowerBase<FloatType> &follower_;
    };
}
