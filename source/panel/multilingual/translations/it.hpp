// Copyright (C) 2025 - zsliu98
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

namespace zlpanel::multilingual::it {
    static constexpr std::array kTexts = {
        "Regola la lunghezza della transizione tra lo stato non compresso e quello compresso.", // Knee
        "Regola il comportamento non lineare dello stato compresso.", // Ratio non-linearity
        "Regola il punto di inizio dello stato compresso.", // Threshold
        "Regola la quantità di riduzione del guadagno dello stato compresso.", // Ratio
        "Scegli lo stile di compressione.", // Mode/Style
        "Regola il comportamento della fase di attacco.", // Attack
        "Regola il comportamento della fase di rilascio.", // Release
        "Regola la velocità di aumento della riduzione del guadagno.", // Attack speed
        "Regola la velocità di diminuzione della riduzione del guadagno.", // Release speed
        "Premi: abilita la compressione RMS\nRilascia: disabilita la compressione RMS.", // RMS toggle
        "Regola la durata della misurazione RMS.", // RMS window
        "Regola la velocità relativa di attacco/rilascio della compressione RMS.", // RMS attack/release
        "Regola la percentuale di mix della compressione RMS.", // RMS mix
        "Regola il valore massimo della riduzione del guadagno.", // Range/Max GR
        "Regola il tempo minimo in cui la riduzione del guadagno non può diminuire.", // Hold
        "Regola il guadagno di compensazione applicato dopo la compressione.", // Makeup Gain
        "Premi: avvia la misurazione del loudness integrato del segnale di ingresso e di uscita\nRilascia: aggiorna il Makeup alla differenza tra i due valori di loudness.",
        // Auto Makeup
        "Regola la percentuale del segnale processato (wet).", // Mix/Wet/Dry
        "I canali M/S sono collegati tra loro.", // M/S Link
        "I canali L/R sono collegati tra loro.", // L/R Link
        "I canali M/S sono collegati al massimo tra M/S.", // M/S Max Link
        "I canali L/R sono collegati al massimo tra L/R.", // L/R Max Link
        "Premi: scambia i canali stereo del side-chain.", // Side-chain swap
        "Regola la percentuale di collegamento tra i due canali stereo.", // Stereo Link
        "Regola la percentuale del segnale processato (wet) per ogni canale stereo.", // Channel-specific wet mix
        "Regola il guadagno del segnale di side-chain.", // Side-chain gain
        "Premi: usa segnale di side-chain esterno\nRilascia: usa segnale di side-chain interno.", // External side-chain
        "Premi: ascolta il segnale di side-chain.", // Side-chain listen
        "Rilascia: bypassa il plugin.", // Bypass
        "Premi: emetti il segnale delta.", // Delta signal
        "Regola il tempo di lookahead del segnale di side-chain.", // Lookahead
        "Scegli il fattore di oversampling.", // Oversampling
        "Regola il drive relativo del clipper.", // Clipper Drive
        "Doppio clic: apri le impostazioni dell'interfaccia utente." // UI Settings
    };
}
