#pragma once

#include <cstddef>

class AuxiliarCli;

/**
 * @file ListaDeCarga.h
 * @brief Lista doblemente enlazada para almacenar caracteres decodificados.
 */
/**
 * @class ListaDeCarga
 * @brief Lista doblemente enlazada que guarda los fragmentos decodificados.
 */
class ListaDeCarga {
public:
    /**
     * @brief Crea una lista vacía.
     */
    ListaDeCarga() noexcept;

    /**
     * @brief Libera todos los nodos almacenados.
     */
    ~ListaDeCarga();

    /**
     * @brief Inserta un nuevo carácter al final de la lista.
     * @param dato Carácter a agregar.
     */
    void insertarAlFinal(char dato);

    /**
     * @brief Elimina todos los nodos y deja la lista vacía.
     */
    void limpiar() noexcept;

    /**
     * @brief Indica si la lista no contiene elementos.
     * @return true cuando no hay nodos.
     */
    bool estaVacia() const noexcept;

    /**
     * @brief Copia el mensaje ensamblado en el buffer proporcionado.
     * @param destino Arreglo donde se escribirá el mensaje.
     * @param capacidad Número máximo de caracteres permitidos (incluye terminador nulo).
     */
    void copiarMensaje(char* destino, std::size_t capacidad) const;

    /**
     * @brief Imprime el mensaje ensamblado. Utiliza el logger si está disponible.
     * @param logger Utilidad opcional para emitir el mensaje u advertencias.
     */
    void imprimirMensaje(AuxiliarCli* logger = nullptr) const;

    /**
     * @brief Obtiene el número de caracteres en la lista.
     * @return Cantidad de nodos actuales.
     */
    std::size_t tamano() const noexcept;

private:
    struct Nodo {
        char dato;
        Nodo* previo;
        Nodo* siguiente;
    };

    Nodo* _cabeza;
    Nodo* _cola;
    std::size_t _cantidad;
};
