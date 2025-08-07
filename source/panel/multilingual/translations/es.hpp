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

namespace zlpanel::multilingual::es {
    static constexpr std::array kTexts = {
        "Ajusta la longitud de la transición entre el estado sin comprimir y el comprimido.",
        "Ajusta el comportamiento no lineal del estado comprimido.",
        "Ajusta el punto de inicio del estado comprimido.",
        "Ajusta la cantidad de reducción de ganancia del estado comprimido.",
        "Elige el estilo de compresión.",
        "Ajusta el comportamiento de la etapa de ataque.",
        "Ajusta el comportamiento de la etapa de liberación.",
        "Ajusta la velocidad de aumento de la reducción de ganancia.",
        "Ajusta la velocidad de disminución de la reducción de ganancia.",
        "Pulsar: activa la compresión RMS\nSoltar: desactiva la compresión RMS.",
        "Ajusta la duración de la medición RMS.",
        "Ajusta la velocidad relativa de ataque/liberación de la compresión RMS.",
        "Ajusta el porcentaje de mezcla de la compresión RMS.",
        "Ajusta el valor máximo de reducción de ganancia.",
        "Ajusta el tiempo mínimo durante el cual se evita que la reducción de ganancia disminuya.",
        "Ajusta la ganancia de compensación que se aplica después de la compresión.",
        "Pulsar: inicia la medición de la sonoridad integrada de la señal de entrada y salida\nSoltar: actualiza la ganancia de compensación a la diferencia entre los dos valores de sonoridad.",
        "Ajusta el porcentaje de la señal procesada (wet).",
        "Los canales M/S (Medio/Lateral) están enlazados.",
        "Los canales L/R (Izquierda/Derecha) están enlazados.",
        "Los canales M/S están enlazados al máximo de M/S.",
        "Los canales L/R están enlazados al máximo de L/R.",
        "Pulsar: intercambia los canales estéreo del side-chain.",
        "Ajusta el porcentaje de enlace entre los dos canales estéreo.",
        "Ajusta el porcentaje de señal procesada (wet) para cada canal estéreo.",
        "Ajusta la ganancia de la señal de side-chain.",
        "Pulsar: usa la señal de side-chain externa\nSoltar: usa la señal de side-chain interna.",
        "Pulsar: escucha la señal de side-chain.",
        "Soltar: desactiva (bypass) el plugin.",
        "Pulsar: emite la señal delta (diferencia).",
        "Ajusta el tiempo de anticipación (lookahead) de la señal de side-chain.",
        "Elige el factor de sobremuestreo (oversampling).",
        "Ajusta el 'drive' relativo del clipper (recortador).",
        "Doble clic: abre los ajustes de la interfaz."
    };
}
