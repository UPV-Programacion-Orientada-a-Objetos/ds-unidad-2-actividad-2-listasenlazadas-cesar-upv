#pragma once

#include <cstddef>

/**
 * @file RotorDeMapeo.h
 * @brief Lista circular doble para traducir caracteres según el protocolo PRT-7.
 */

/**
 * @class RotorDeMapeo
 * @brief Rotor circular doblemente enlazado que implementa el mapeo dinámico.
 */
class RotorDeMapeo {
public:
    /**
     * @brief Construye el rotor cargado con el alfabeto A-Z.
     */
    RotorDeMapeo();

    /**
     * @brief Libera todos los nodos del rotor.
     */
    ~RotorDeMapeo();

    /**
     * @brief Rota el rotor la cantidad indicada.
     * @param pasos Entero positivo (derecha) o negativo (izquierda).
     */
    void rotar(int pasos);

    /**
     * @brief Obtiene el carácter mapeado considerando la posición actual del rotor.
     * @param entrada Carácter original recibido.
     * @return Carácter decodificado. Si el carácter no está en el alfabeto, se regresa sin cambios.
     */
    char getMapeo(char entrada) const;

    /**
     * @brief Restablece la cabeza del rotor para que apunte nuevamente a 'A'.
     */
    void reiniciar() noexcept;

private:
    struct Nodo {
        char valor;
        Nodo* previo;
        Nodo* siguiente;
    };

    Nodo* _cabeza;
    std::size_t _tamano;

    void inicializar();
    Nodo* crearNodo(char valor, Nodo* previo) const;
};
