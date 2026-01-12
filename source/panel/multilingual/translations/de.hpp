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

namespace zlpanel::multilingual::de {
    static constexpr std::array kTexts = {
        "Stellt die Übergangslänge zwischen dem unkomprimierten und dem komprimierten Zustand ein (Knee).",
        "Stellt das nichtlineare Verhalten des komprimierten Zustands ein.",
        "Legt den Schwellenwert (Threshold) fest, ab dem die Kompression beginnt.",
        "Stellt das Ausmaß der Pegelreduzierung im komprimierten Zustand ein (Ratio).",
        "Wählt den Kompressionsstil aus.",
        "Stellt das Verhalten der Attack-Phase ein.",
        "Stellt das Verhalten der Release-Phase ein.",
        "Stellt die Geschwindigkeit ein, mit der die Pegelreduzierung greift (Attack).",
        "Stellt die Geschwindigkeit ein, mit der die Pegelreduzierung nachlässt (Release).",
        "Drücken: RMS-Kompression aktivieren\nLoslassen: RMS-Kompression deaktivieren.",
        "Stellt die Zeitdauer für die RMS-Messung ein.",
        "Stellt die relative Attack-/Release-Geschwindigkeit der RMS-Kompression ein.",
        "Stellt den Mix-Anteil der RMS-Kompression in Prozent ein.",
        "Stellt den Maximalwert für die Pegelreduzierung ein.",
        "Stellt die minimale Zeitspanne ein, in der die Pegelreduzierung gehalten wird (Hold).",
        "Stellt die Aufholverstärkung (Makeup Gain) ein, die nach der Kompression angewendet wird.",
        "Drücken: Startet die Messung der integrierten Lautheit des Eingangs- und Ausgangssignals\nLoslassen: Aktualisiert die Aufholverstärkung auf die Differenz der beiden Lautheitswerte.",
        "Stellt den prozentualen Anteil des bearbeiteten Signals (Wet) ein.",
        "M/S (Mitte/Seite) ist miteinander verkoppelt.",
        "L/R (Links/Rechts) ist miteinander verkoppelt.",
        "M/S (Mitte/Seite) wird durch den Maximalwert von M/S gesteuert.",
        "L/R (Links/Rechts) wird durch den Maximalwert von L/R gesteuert.",
        "Drücken: Tauscht die Sidechain-Stereokanäle.",
        "Stellt den Verkopplungsgrad zwischen den beiden Stereokanälen in Prozent ein.",
        "Stellt den prozentualen Anteil des bearbeiteten Signals (Wet) für jeden Stereokanal ein.",
        "Stellt die Verstärkung des Sidechain-Signals ein.",
        "Drücken: Externes Sidechain-Signal verwenden\nLoslassen: Internes Sidechain-Signal verwenden.",
        "Drücken: Sidechain-Signal abhören.",
        "Loslassen: Plugin umgehen (Bypass).",
        "Drücken: Delta-Signal (Differenzsignal) ausgeben.",
        "Stellt die Lookahead-Zeit (Vorausschau) des Sidechain-Signals ein.",
        "Wählt den Oversampling-Faktor aus.",
        "Stellt den relativen Drive des Clippers ein.",
        "Doppelklick: Öffnet die Benutzeroberflächen-Einstellungen."
    };
}
