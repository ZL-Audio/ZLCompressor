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
    inline constexpr std::array kTexts = {
        "Steuert die Kniebreite zwischen dem linearen und dem bearbeiteten Verstärkungsbereich.",
        "Steuert die nichtlineare Krümmung der Verstärkungskennlinie.",
        "Steuert den Schwellenwert, ab dem die Dynamikbearbeitung beginnt.",
        "Steuert das Verhältnis der Verstärkungskennlinie.",
        "Wählt den Bearbeitungsstil.",
        "Steuert das Zeitverhalten der Attack-Phase.",
        "Steuert das Zeitverhalten der Release-Phase.",
        "Steuert die Zeit, die die Verstärkungsänderung benötigt, um auf den Zielwert anzusteigen.",
        "Steuert die Zeit, die die Verstärkungsänderung benötigt, um auf den Zielwert abzufallen.",
        "Drücken: RMS-Verarbeitung aktivieren.",
        "Steuert das Integrationsfenster für die RMS-Messung.",
        "Steuert die relative Attack-/Release-Zeit der RMS-Verarbeitung.",
        "Steuert den Mischungsanteil der RMS-Verarbeitung.",
        "Steuert die maximal zulässige Verstärkungsänderung.",
        "Steuert die Mindestdauer, die die Verstärkungsänderung gehalten wird, bevor sie abklingt.",
        "Steuert das nach der Bearbeitung angewendete Makeup Gain.",
        "Drücken: Startet die Messung der integrierten Lautheit für Ein- und Ausgang\nLoslassen: Wendet Makeup Gain basierend auf der gemessenen Lautheitsdifferenz an.",
        "Steuert den globalen Wet-Mix-Anteil.",
        "M/S sind miteinander gekoppelt.",
        "L/R sind miteinander gekoppelt.",
        "M/S ist mit dem Maximum von M/S gekoppelt.",
        "L/R ist mit dem Maximum von L/R gekoppelt.",
        "Drücken: Vertauscht die Stereo-Eingangskanäle des Sidechains.",
        "Steuert den Prozentsatz der Stereokopplung zwischen den Kanälen.",
        "Steuert den Wet-Signal-Anteil für jeden Kanal.",
        "Steuert die Verstärkung des Sidechain-Signals.",
        "Drücken: Aktiviert den externen Sidechain-Eingang\nLoslassen: Verwendet das interne Sidechain-Signal.",
        "Drücken: Sidechain-Signal abhören.",
        "Loslassen: Plugin-Bearbeitung umgehen (Bypass).",
        "Drücken: Delta-Signal abhören.",
        "Steuert die Lookahead-Verzögerung für den Sidechain-Detektor.",
        "Wählt den Oversampling-Faktor.",
        "Steuert die Drive-Intensität des Soft Clippers.",
        "Doppelklick: Öffnet die UI-Konfiguration.",
        "Steuert den Boden der Verstärkungskennlinie.",
        "Drücken: Setzt die maximale Verstärkungsänderung auf unendlich.",
        "Abwärtskompression: Dämpft Signale über dem Schwellenwert, um den Dynamikbereich zu reduzieren.",
        "Aufwärtskompression: Hebt Signale unter dem Schwellenwert an, um den Dynamikbereich zu reduzieren.",
        "Abwärtsexpansion: Dämpft Signale unter dem Schwellenwert, um den Dynamikbereich zu vergrößern.",
        "Aufwärtsexpansion: Hebt Signale über dem Schwellenwert an, um den Dynamikbereich zu vergrößern.",
        "Drücken: Schaltet die Meter-Anzeige um.",
        "Drücken: Schaltet das kumulative RMS-Analyse-Panel um.",
        "Drücken: Schaltet die Anzeige der Verstärkungskennlinie um.",
        "Drücken: Schaltet die Sidechain-Equalizer-Oberfläche um.",
        "Drücken: Schaltet das Sidechain-Bedienfeld um.",
        "Wählt die Methode der Magnitudenmessung.",
        "Wählt den Stereokanal für die Magnitudenmessung.",
        "Wählt die Zeitdauer des Magnitudenanalysators.",
        "Wählt den minimalen Dezibelwert des Magnitudenanalysators."
    };
}
