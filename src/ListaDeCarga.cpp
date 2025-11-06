#include "ListaDeCarga.h"

#include "AuxiliarCli.h"

#include <cstdio>
#include <cstring>

ListaDeCarga::ListaDeCarga() noexcept : _cabeza(nullptr), _cola(nullptr), _cantidad(0) {}

ListaDeCarga::~ListaDeCarga()
{
    limpiar();
}

void ListaDeCarga::insertarAlFinal(char dato)
{
    Nodo* nuevo = new Nodo{dato, _cola, nullptr};
    if (_cola) {
        _cola->siguiente = nuevo;
    } else {
        _cabeza = nuevo;
    }
    _cola = nuevo;
    ++_cantidad;
}

void ListaDeCarga::limpiar() noexcept
{
    Nodo* actual = _cabeza;
    while (actual) {
        Nodo* siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    _cabeza = nullptr;
    _cola = nullptr;
    _cantidad = 0;
}

bool ListaDeCarga::estaVacia() const noexcept
{
    return _cantidad == 0;
}

void ListaDeCarga::copiarMensaje(char* destino, std::size_t capacidad) const
{
    if (!destino || capacidad == 0) {
        return;
    }

    std::size_t usado = 0;
    Nodo* actual = _cabeza;
    while (actual && usado + 1 < capacidad) {
        destino[usado++] = actual->dato;
        actual = actual->siguiente;
    }
    destino[usado] = '\0';
}

void ListaDeCarga::imprimirMensaje(AuxiliarCli* logger) const
{
    const std::size_t longitud = _cantidad + 1;
    char bufferPequeno[256];
    char* destino = bufferPequeno;

    if (longitud > sizeof(bufferPequeno)) {
        destino = new char[longitud];
    }

    copiarMensaje(destino, longitud);

    if (logger) {
        if (destino[0] == '\0') {
            logger->imprimirLog("WARNING", "No se ensamblaron datos.");
        } else {
            logger->imprimirLog("STATUS", destino);
        }
    } else {
        if (destino[0] == '\0') {
            std::puts("Mensaje ensamblado: <vacio>");
        } else {
            std::puts(destino);
        }
    }

    if (destino != bufferPequeno) {
        delete[] destino;
    }
}

std::size_t ListaDeCarga::tamano() const noexcept
{
    return _cantidad;
}
