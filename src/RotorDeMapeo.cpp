#include "RotorDeMapeo.h"

RotorDeMapeo::RotorDeMapeo() : _cabeza(nullptr), _tamano(0)
{
    inicializar();
}

RotorDeMapeo::~RotorDeMapeo()
{
    if (!_cabeza) {
        return;
    }
    Nodo* actual = _cabeza->siguiente;
    while (actual && actual != _cabeza) {
        Nodo* siguiente = actual->siguiente;
        delete actual;
        actual = siguiente;
    }
    delete _cabeza;
    _cabeza = nullptr;
    _tamano = 0;
}

void RotorDeMapeo::rotar(int pasos)
{
    if (!_cabeza || pasos == 0) {
        return;
    }

    int offset = pasos % static_cast<int>(_tamano);
    if (offset < 0) {
        offset += static_cast<int>(_tamano);
    }

    while (offset-- > 0) {
        _cabeza = _cabeza->siguiente;
    }
}

char RotorDeMapeo::getMapeo(char entrada) const
{
    if (!_cabeza) {
        return entrada;
    }

    if (entrada < 'A' || entrada > 'Z') {
        return entrada;
    }

    int desplazamiento = entrada - 'A';
    const Nodo* actual = _cabeza;
    while (desplazamiento-- > 0) {
        actual = actual->siguiente;
    }
    return actual->valor;
}

void RotorDeMapeo::reiniciar() noexcept
{
    if (!_cabeza) {
        return;
    }

    Nodo* actual = _cabeza;
    while (actual->valor != 'A') {
        actual = actual->siguiente;
        if (actual == _cabeza) {
            break;
        }
    }
    _cabeza = actual;
}

void RotorDeMapeo::inicializar()
{
    Nodo* previo = nullptr;
    for (char letra = 'A'; letra <= 'Z'; ++letra) {
        Nodo* nuevo = crearNodo(letra, previo);
        if (!_cabeza) {
            _cabeza = nuevo;
        }
        if (previo) {
            previo->siguiente = nuevo;
        }
        previo = nuevo;
        ++_tamano;
    }

    if (_cabeza && previo) {
        _cabeza->previo = previo;
        previo->siguiente = _cabeza;
    }
}

RotorDeMapeo::Nodo* RotorDeMapeo::crearNodo(char valor, Nodo* previo) const
{
    Nodo* nodo = new Nodo{valor, previo, nullptr};
    if (previo) {
        previo->siguiente = nodo;
    }
    return nodo;
}
