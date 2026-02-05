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

namespace zlpanel::multilingual::es {
    inline constexpr std::array kTexts = {
        "Controla el ancho de la rodilla (knee) entre los estados de ganancia lineal y procesada.",
        "Controla la curvatura no lineal de la función de transferencia de ganancia.",
        "Controla el nivel de umbral donde comienza el procesamiento dinámico.",
        "Controla la proporción (ratio) de la función de transferencia de ganancia.",
        "Elige el estilo de procesamiento.",
        "Controla la respuesta temporal de la etapa de ataque.",
        "Controla la respuesta temporal de la etapa de liberación.",
        "Controla el tiempo que tarda el cambio de ganancia en aumentar hasta su nivel objetivo.",
        "Controla el tiempo que tarda el cambio de ganancia en disminuir hasta su nivel objetivo.",
        "Pulsar: activar el procesamiento RMS.",
        "Controla la ventana de integración para la medición RMS.",
        "Controla el tiempo relativo de ataque/liberación del procesamiento RMS.",
        "Controla el porcentaje de mezcla del procesamiento RMS.",
        "Controla el cambio máximo de ganancia permitido.",
        "Controla la duración mínima que se mantiene el cambio de ganancia antes de liberar.",
        "Controla la ganancia de compensación (makeup) aplicada después del procesamiento.",
        "Pulsar: iniciar la medición de sonoridad integrada para entrada y salida\nSoltar: aplicar ganancia de compensación basada en la diferencia de sonoridad medida.",
        "Controla el porcentaje global de mezcla de señal procesada (wet).",
        "M/S están vinculados entre sí.",
        "L/R están vinculados entre sí.",
        "M/S se vincula con el máximo de M/S.",
        "L/R se vincula con el máximo de L/R.",
        "Pulsar: intercambiar los canales de entrada estéreo de la cadena lateral.",
        "Controla el porcentaje de acoplamiento estéreo entre canales.",
        "Controla el porcentaje de señal procesada (wet) para cada canal.",
        "Controla la ganancia de la señal de cadena lateral (side-chain).",
        "Pulsar: activar entrada de cadena lateral externa\nSoltar: usar señal de cadena lateral interna.",
        "Pulsar: monitorizar la señal de cadena lateral.",
        "Soltar: omitir (bypass) el procesamiento del plugin.",
        "Pulsar: monitorizar la señal delta.",
        "Controla el retardo de anticipación (lookahead) para el detector de cadena lateral.",
        "Elige el factor de sobremuestreo.",
        "Controla la intensidad de saturación (drive) del recortador suave.",
        "Doble clic: abrir configuración de la interfaz.",
        "Controla el suelo de la función de transferencia de ganancia.",
        "Pulsar: establecer el cambio máximo de ganancia a infinito.",
        "Compresión descendente: Atenúa las señales por encima del umbral para reducir el rango dinámico.",
        "Compresión ascendente: Realza las señales por debajo del umbral para reducir el rango dinámico.",
        "Expansión descendente: Atenúa las señales por debajo del umbral para aumentar el rango dinámico.",
        "Expansión ascendente: Realza las señales por encima del umbral para aumentar el rango dinámico.",
        "Pulsar: alternar la visualización del medidor.",
        "Pulsar: alternar el panel de análisis RMS acumulativo.",
        "Pulsar: alternar la visualización de la curva de transferencia de ganancia.",
        "Pulsar: alternar la interfaz del ecualizador de cadena lateral.",
        "Pulsar: alternar el panel de control de cadena lateral.",
        "Elige el método de medición de magnitud.",
        "Elige el canal estéreo de medición de magnitud.",
        "Elige la duración del analizador de magnitud.",
        "Elige el valor mínimo en decibelios del analizador de magnitud."
    };
}
