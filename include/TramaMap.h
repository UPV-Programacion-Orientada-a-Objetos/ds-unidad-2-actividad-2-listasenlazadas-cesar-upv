#pragma once

#include "TramaBase.h"

class ListaDeCarga;
class RotorDeMapeo;

/**
 * @file TramaMap.h
 * @brief Declara la trama de rotación del protocolo PRT-7.
 */
/**
 * @class TramaMap
 * @brief Ordena la rotación del rotor de mapeo.
 */
class TramaMap : public TramaBase {
public:
    /**
     * @brief Construye la trama con la magnitud de rotación solicitada.
     * @param desplazamiento Número de pasos a rotar; positivo hacia adelante.
     */
    explicit TramaMap(int desplazamiento) noexcept;

    /**
     * @brief Aplica la rotación indicada sobre el rotor de mapeo.
     * @param carga Lista de carga (no se utiliza, pero se respeta la interfaz).
     * @param rotor Rotor circular que se debe rotar.
     */
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override;

private:
    int _desplazamiento;
};
