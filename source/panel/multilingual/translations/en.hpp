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
        "Control the knee width between the linear and processed gain states.",
        "Control the non-linear curvature of the gain transfer function.",
        "Control the threshold level where dynamics processing begins.",
        "Control the ratio of the gain transfer function.",
        "Choose the processing style.",
        "Control the temporal response of the attack stage.",
        "Control the temporal response of the release stage.",
        "Control the time it takes for the gain change to increase to its target level.",
        "Control the time it takes for the gain change to decrease to its target level.",
        "Press: enbale RMS processing.",
        "Control the integration window for RMS measurement.",
        "Control the relative attack/release time of the RMS processing.",
        "Control the mix percentage of the RMS processing",
        "Control the maximum allowable gain change.",
        "Control the minimum duration the gain change is held before releasing.",
        "Control the makeup gain applied post-processing.",
        "Press: initiate integrated loudness measurement for input and output\nRelease: apply makeup gain based on the measured loudness delta.",
        "Control the global wet mix percentage.",
        "M/S is linked with each other.",
        "L/R is linked with each other.",
        "M/S is linked with the maximum of M/S.",
        "L/R is linked with the maximum of L/R.",
        "Press: swap the side-chain stereo input channels.",
        "Control the stereo coupling percentage between channels.",
        "Control the wet signal percentage for each channel.",
        "Control the gain of the side-chain signal.",
        "Press: engage external side-chain input\nRelease: use internal side-chain signal.",
        "Press: monitor the side-chain signal.",
        "Release: bypass the plugin processing.",
        "Press: monitor the delta signal.",
        "Control the lookahead delay for the side-chain detector.",
        "Choose the oversampling factor.",
        "Control the drive intensity of the soft clipper.",
        "Double click: open UI configuration.",
        "Control the floor of the gain transfer function.",
        "Press: set the maximum gain change to infinity.",
        "Downward Compression: Attenuates signals above the threshold to reduce dynamic range.",
        "Upward Compression: Boosts signals below the threshold to reduce dynamic range.",
        "Downward Expansion: Attenuates signals below the threshold to increase dynamic range.",
        "Upward Expansion: Boosts signals above the threshold to increase dynamic range.",
        "Press: toggle the meter visualization.",
        "Press: toggle the cumulative RMS analysis panel.",
        "Press: toggle the gain transfer curve display.",
        "Press: toggle the side-chain equalizer interface.",
        "Press: toggle the side-chain control panel.",
        "Choose the magnitude measurement method.",
        "Choose the magnitude measurement stereo channel.",
        "Choose the magnitude analyzer time length.",
        "Choose the magnitude analyzer minimum decible value."
    };
}
