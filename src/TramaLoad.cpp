#include "TramaLoad.h"

#include "ListaDeCarga.h"
#include "RotorDeMapeo.h"

TramaLoad::TramaLoad(char dato) noexcept : _dato(dato) {}

void TramaLoad::procesar(ListaDeCarga* carga, RotorDeMapeo* rotor)
{
    if (!carga || !rotor) {
        return;
    }
    const char decodificado = rotor->getMapeo(_dato);
    carga->insertarAlFinal(decodificado);
}
