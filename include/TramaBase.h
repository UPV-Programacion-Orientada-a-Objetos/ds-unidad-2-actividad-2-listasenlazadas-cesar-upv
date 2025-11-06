#pragma once

/**
 * @file TramaBase.h
 * @brief Clase base para cualquier trama del protocolo PRT-7.
 */

class ListaDeCarga;
class RotorDeMapeo;

/**
 * @brief Interface polimórfica para cualquier trama recibida del protocolo PRT-7.
 */
class TramaBase {
public:
    /**
     * @brief Ejecuta la acción asociada a la trama sobre la carga y el rotor.
     * @param carga Lista donde se ensamblan los caracteres decodificados.
     * @param rotor Disco de mapeo utilizado para traducir los fragmentos.
     */
    virtual void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) = 0;

    /**
     * @brief Destructor virtual obligatorio para liberar memoria de forma segura.
     */
    virtual ~TramaBase() = default;
};
