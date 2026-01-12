// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

#pragma once

#include <array>

namespace zlpanel::multilingual::en {
    static constexpr std::array kTexts = {
        "Adjust the transition length between the uncompressed state and the compressed state.",
        "Adjust the non-linearity behavior of the compressed state.",
        "Adjust the starting point of the compressed state.",
        "Adjust the amount of gain reduction of the compressed state.",
        "Choose the compression style.",
        "Adjust the behaviour of the attack stage.",
        "Adjust the behaviour of the release stage.",
        "Adjust the speed of gain reduction increase.",
        "Adjust the speed of gain reduction decrease.",
        "Press: enable RMS compression\nRelease: disable RMS compression.",
        "Adjust the length of RMS measurement.",
        "Adjust the relative attack/release speed of RMS compression.",
        "Adjust the mix percent of RMS compression.",
        "Adjust the maximum value of gain reduction.",
        "Adjust the minimum amount of time where gain reduction is prevented from decreasing.",
        "Adjust the makeup gain that is applied after the compression.",
        "Press: start to measure the integrated loudness of the input signal and the output signal\nRelease: update the Makeup to the difference between two loudness values.",
        "Adjust the percent of wet signal.",
        "M/S is linked with each other.",
        "L/R is linked with each other.",
        "M/S is linked with the maximum of M/S.",
        "L/R is linked with the maximum of L/R.",
        "Press: swap the side-chain stereo channels.",
        "Adjust the link percent between two stereo channels.",
        "Adjust the percent of wet signal for each stereo channel.",
        "Adjust the gain of the side-chain signal.",
        "Press: use external side-chain signal\nRelease: use internal side-chain signal.",
        "Press: listen to the side-chain signal.",
        "Release: bypass the plugin.",
        "Press: output delta signal.",
        "Adjust the lookahead time of the side-chain signal.",
        "Choose the over-sampling factor.",
        "Adjust the relative drive of the clipper.",
        "Double click: open UI settings."
    };
}
