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

namespace zlpanel::multilingual::it {
    static constexpr std::array kTexts = {
        "Controlla la larghezza del knee tra gli stati di guadagno lineare ed elaborato.",
        "Controlla la curvatura non lineare della funzione di trasferimento del guadagno.",
        "Controlla il livello di soglia (threshold) dove inizia l'elaborazione della dinamica.",
        "Controlla il rapporto (ratio) della funzione di trasferimento del guadagno.",
        "Scegli lo stile di elaborazione.",
        "Controlla la risposta temporale della fase di attacco.",
        "Controlla la risposta temporale della fase di rilascio.",
        "Controlla il tempo necessario affinché la variazione di guadagno aumenti al livello target.",
        "Controlla il tempo necessario affinché la variazione di guadagno diminuisca al livello target.",
        "Premi: abilita l'elaborazione RMS.",
        "Controlla la finestra di integrazione per la misurazione RMS.",
        "Controlla il tempo relativo di attacco/rilascio dell'elaborazione RMS.",
        "Controlla la percentuale di mix dell'elaborazione RMS.",
        "Controlla la massima variazione di guadagno consentita.",
        "Controlla la durata minima di mantenimento della variazione di guadagno prima del rilascio.",
        "Controlla il guadagno di compensazione (makeup gain) applicato dopo l'elaborazione.",
        "Premi: avvia la misurazione del loudness integrato per ingresso e uscita\nRilascia: applica il guadagno di compensazione basato sulla differenza di loudness misurata.",
        "Controlla la percentuale globale di mix wet.",
        "M/S sono collegati tra loro.",
        "L/R sono collegati tra loro.",
        "M/S sono collegati al massimo di M/S.",
        "L/R sono collegati al massimo di L/R.",
        "Premi: scambia i canali di ingresso stereo del side-chain.",
        "Controlla la percentuale di accoppiamento stereo tra i canali.",
        "Controlla la percentuale di segnale wet per ogni canale.",
        "Controlla il guadagno del segnale side-chain.",
        "Premi: attiva l'ingresso side-chain esterno\nRilascia: usa il segnale side-chain interno.",
        "Premi: monitora il segnale side-chain.",
        "Rilascia: bypassa l'elaborazione del plugin.",
        "Premi: monitora il segnale delta.",
        "Controlla il ritardo di lookahead per il rilevatore side-chain.",
        "Scegli il fattore di sovracampionamento.",
        "Controlla l'intensità di drive del soft clipper.",
        "Doppio clic: apri la configurazione dell'interfaccia.",
        "Controlla il livello minimo (floor) della funzione di trasferimento del guadagno.",
        "Premi: imposta la variazione massima di guadagno su infinito.",
        "Compressione verso il basso (Downward): Attenua i segnali sopra la soglia per ridurre la gamma dinamica.",
        "Compressione verso l'alto (Upward): Amplifica i segnali sotto la soglia per ridurre la gamma dinamica.",
        "Espansione verso il basso (Downward): Attenua i segnali sotto la soglia per aumentare la gamma dinamica.",
        "Espansione verso l'alto (Upward): Amplifica i segnali sopra la soglia per aumentare la gamma dinamica.",
        "Premi: attiva/disattiva la visualizzazione dei meter.",
        "Premi: attiva/disattiva il pannello di analisi RMS cumulativa.",
        "Premi: attiva/disattiva la visualizzazione della curva di trasferimento del guadagno.",
        "Premi: attiva/disattiva l'interfaccia dell'equalizzatore side-chain.",
        "Premi: attiva/disattiva il pannello di controllo side-chain.",
        "Scegli il metodo di misurazione dell'ampiezza.",
        "Scegli il canale stereo per la misurazione dell'ampiezza.",
        "Scegli la durata temporale dell'analizzatore di ampiezza.",
        "Scegli il valore minimo in decibel dell'analizzatore di ampiezza."
    };
}
