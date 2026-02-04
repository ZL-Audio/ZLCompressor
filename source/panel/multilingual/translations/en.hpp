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
        "Control the transition width between the unprocessed state and the processed state.",
        "Control the non-linearity behavior of the processed state.",
        "Control the starting point of the processed state.",
        "Control the processing ratio of the processed state.",
        "Choose the processing style.",
        "Control the behaviour of the attack stage.",
        "Control the behaviour of the release stage.",
        "Control the time it takes for the gain change to increase to its target level.",
        "Control the time it takes for the gain change to decrease to its target level.",
        "Press: enable RMS processing\nRelease: disable RMS processing.",
        "Control the length of RMS measurement.",
        "Control the relative attack/release speed of RMS processing.",
        "Control the mix percent of RMS processing.",
        "Control the maximum value of gain change.",
        "Control the minimum amount of time where gain change is prevented from decreasing.",
        "Control the makeup gain that is applied after the processing.",
        "Press: start to measure the integrated loudness of the input signal and the output signal\nRelease: update the Makeup to the difference between two loudness values.",
        "Control the percent of wet signal.",
        "M/S is linked with each other.",
        "L/R is linked with each other.",
        "M/S is linked with the maximum of M/S.",
        "L/R is linked with the maximum of L/R.",
        "Press: swap the side-chain stereo channels.",
        "Control the link percent between two stereo channels.",
        "Control the percent of wet signal for each stereo channel.",
        "Control the gain of the side-chain signal.",
        "Press: use external side-chain signal\nRelease: use internal side-chain signal.",
        "Press: listen to the side-chain signal.",
        "Release: bypass the plugin.",
        "Press: output delta signal.",
        "Control the lookahead or delay time of the side-chain signal.",
        "Choose the over-sampling factor.",
        "Control the relative drive of the clipper.",
        "Double click: open UI settings.",
        "Control the floor of the processed state.",
        "Press: set the maximum value of gain change to inf.",
    };
}
