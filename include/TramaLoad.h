#pragma once

#include "TramaBase.h"

class ListaDeCarga;
class RotorDeMapeo;

/**
 * @file TramaLoad.h
 * @brief Declara la trama de carga del protocolo PRT-7.
 */
/**
 * @class TramaLoad
 * @brief Inserta en la lista el carácter decodificado por el rotor.
 */
class TramaLoad : public TramaBase {
public:
    /**
     * @brief Construye la trama con el carácter recibido.
     * @param dato Carácter bruto enviado por el Arduino.
     */
    explicit TramaLoad(char dato) noexcept;

    /**
     * @brief Decodifica el carácter y lo agrega a la lista de carga.
     * @param carga Lista doblemente enlazada que forma el mensaje final.
     * @param rotor Rotor encargado de mapear el carácter de entrada.
     */
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override;

private:
    char _dato;
};
