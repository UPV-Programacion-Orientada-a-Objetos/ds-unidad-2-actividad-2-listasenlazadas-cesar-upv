#include "TramaMap.h"

#include "RotorDeMapeo.h"

TramaMap::TramaMap(int desplazamiento) noexcept : _desplazamiento(desplazamiento) {}

void TramaMap::procesar(ListaDeCarga* /*carga*/, RotorDeMapeo* rotor)
{
    if (!rotor) {
        return;
    }
    rotor->rotar(_desplazamiento);
}
