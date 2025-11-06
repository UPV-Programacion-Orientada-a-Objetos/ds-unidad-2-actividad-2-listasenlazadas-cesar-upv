#include "LineaDispatcher.h"

#include "AuxiliarCli.h"
#include "ListaDeCarga.h"
#include "RotorDeMapeo.h"
#include "TramaLoad.h"
#include "TramaMap.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>

LineaDispatcher::LineaDispatcher(ListaDeCarga* carga, RotorDeMapeo* rotor, AuxiliarCli* logger) noexcept
    : _carga(carga)
    , _rotor(rotor)
    , _logger(logger)
    , _procesadas(0)
    , _sesionActiva(false)
{
}

void LineaDispatcher::setLogger(AuxiliarCli* logger) noexcept
{
    _logger = logger;
}

void LineaDispatcher::setComponentes(ListaDeCarga* carga, RotorDeMapeo* rotor) noexcept
{
    _carga = carga;
    _rotor = rotor;
    _sesionActiva = false;
    _procesadas = 0;
}

void LineaDispatcher::iniciarSesion(const char* motivo, bool limpiar)
{
    if (limpiar) {
        if (_carga) {
            _carga->limpiar();
        }
        if (_rotor) {
            _rotor->reiniciar();
        }
    }
    _procesadas = 0;
    _sesionActiva = true;

    if (_logger && motivo) {
        char mensaje[160];
        std::snprintf(mensaje, sizeof(mensaje), "Secuencia %s detectada. Reiniciando estructuras.", motivo);
        _logger->imprimirLog("STATUS", mensaje);
        registrarSaltoLinea();
    }
}

void LineaDispatcher::terminarSesion() noexcept
{
    _sesionActiva = false;
    _procesadas = 0;
}

bool LineaDispatcher::sesionActiva() const noexcept
{
    return _sesionActiva;
}

std::size_t LineaDispatcher::totalProcesado() const noexcept
{
    return _procesadas;
}

void LineaDispatcher::onRawLine(const char* linea)
{
    if (!linea || linea[0] == '\0') {
        return;
    }

    char buffer[128];
    const std::size_t limite = sizeof(buffer) - 1;
    std::size_t longitud = std::strlen(linea);
    if (longitud > limite) {
        longitud = limite;
    }
    std::memcpy(buffer, linea, longitud);
    buffer[longitud] = '\0';

    while (longitud > 0 && (buffer[longitud - 1] == '\r' || buffer[longitud - 1] == '\n')) {
        buffer[--longitud] = '\0';
    }

    if (longitud == 0) {
        return;
    }

    char mayus[128];
    std::size_t idx = 0;
    for (; idx < longitud && idx < sizeof(mayus) - 1; ++idx) {
        mayus[idx] = static_cast<char>(std::toupper(static_cast<unsigned char>(buffer[idx])));
    }
    mayus[idx] = '\0';

    if (std::strcmp(mayus, "INICIO") == 0) {
        iniciarSesion("INICIO", true);
        return;
    }

    if (!_sesionActiva) {
        log("WARNING", "Se ignora la trama porque no se ha recibido INICIO.");
        return;
    }

    if (_logger) {
        char mensaje[160];
        std::snprintf(mensaje, sizeof(mensaje), "Trama recibida: [%s]", buffer);
        _logger->imprimirLog("STATUS", mensaje);
    }

    char* tipo = std::strtok(buffer, ",");
    char* payload = std::strtok(nullptr, ",");

    if (!tipo || !payload) {
        log("WARNING", "Trama incompleta recibida.");
        return;
    }

    while (*payload == ' ') {
        ++payload;
    }

    const char prefijo = static_cast<char>(std::toupper(static_cast<unsigned char>(tipo[0])));
    bool exito = false;
    if (prefijo == 'L') {
        exito = procesarCarga(payload);
    } else if (prefijo == 'M') {
        exito = procesarMapa(payload);
    } else {
        log("WARNING", "Prefijo de trama desconocido.");
    }

    if (exito) {
        ++_procesadas;
    }
}

bool LineaDispatcher::procesarCarga(char* payload)
{
    if (!_carga || !_rotor) {
        log("WARNING", "Componentes no configurados para procesar LOAD.");
        return false;
    }

    bool valido = false;
    const char dato = interpretarTokenCarga(payload, valido);
    if (!valido) {
        log("WARNING", "Token de carga inválido.");
        return false;
    }

    const char decodificado = _rotor->getMapeo(dato);

    TramaLoad trama(dato);
    trama.procesar(_carga, _rotor);

    char origen[32];
    char destino[32];
    describirCaracter(dato, origen, sizeof(origen));
    describirCaracter(decodificado, destino, sizeof(destino));

    char detalle[192];
    std::snprintf(detalle, sizeof(detalle), " -> Procesando... -> Fragmento %s decodificado como %s.", origen, destino);
    log("STATUS", detalle);

    char ensamblado[768];
    if (_carga) {
        _carga->copiarMensaje(ensamblado, sizeof(ensamblado));
    } else {
        ensamblado[0] = '\0';
    }

    char mensaje[800];
    std::snprintf(mensaje, sizeof(mensaje), " Mensaje: %s", ensamblado);
    log("STATUS", mensaje);
    registrarSaltoLinea();

    return true;
}

bool LineaDispatcher::procesarMapa(char* payload)
{
    if (!_rotor) {
        log("WARNING", "Rotor no configurado para procesar MAP.");
        return false;
    }

    char* fin = nullptr;
    const long desplazamiento = std::strtol(payload, &fin, 10);
    if (fin == payload) {
        log("WARNING", "Valor de rotación inválido.");
        return false;
    }

    TramaMap trama(static_cast<int>(desplazamiento));
    trama.procesar(nullptr, _rotor);

    const int desplazamientoInt = static_cast<int>(desplazamiento);
    const char signo = (desplazamientoInt >= 0) ? '+' : '-';
    const int magnitud = (desplazamientoInt >= 0) ? desplazamientoInt : -desplazamientoInt;
    const char mapeo = _rotor->getMapeo('A');

    char mensaje[192];
    std::snprintf(mensaje, sizeof(mensaje), " -> Procesando... -> ROTANDO ROTOR %c%d. (Ahora 'A' se mapea a '%c')",
                  signo, magnitud, mapeo);
    log("STATUS", mensaje);
    registrarSaltoLinea();

    return true;
}

char LineaDispatcher::interpretarTokenCarga(const char* payload, bool& valido)
{
    valido = false;
    if (!payload) {
        return '\0';
    }

    if (payload[0] == '\'' && payload[2] == '\'' && payload[3] == '\0') {
        valido = true;
        return payload[1];
    }

    const std::size_t longitud = std::strlen(payload);
    if (longitud == 1) {
        valido = true;
        return payload[0];
    }

    if (std::strcmp(payload, "Space") == 0 || std::strcmp(payload, "SPACE") == 0) {
        valido = true;
        return ' ';
    }

    if (std::strcmp(payload, "Tab") == 0 || std::strcmp(payload, "TAB") == 0) {
        valido = true;
        return '\t';
    }

    if (std::strcmp(payload, "Comma") == 0 || std::strcmp(payload, "COMMA") == 0) {
        valido = true;
        return ',';
    }

    return '\0';
}

void LineaDispatcher::log(const char* tipo, const char* mensaje) const
{
    if (_logger) {
        _logger->imprimirLog(tipo, mensaje);
    }
}

void LineaDispatcher::describirCaracter(char caracter, char* destino, std::size_t tam)
{
    if (!destino || tam == 0) {
        return;
    }

    if (caracter == '\t') {
        std::snprintf(destino, tam, "'\\t'");
        return;
    }
    if (caracter == '\n') {
        std::snprintf(destino, tam, "'\\n'");
        return;
    }
    if (caracter == '\r') {
        std::snprintf(destino, tam, "'\\r'");
        return;
    }

    if (std::isprint(static_cast<unsigned char>(caracter)) || caracter == ' ') {
        std::snprintf(destino, tam, "'%c'", caracter);
    } else {
        std::snprintf(destino, tam, "0x%02X", static_cast<unsigned int>(static_cast<unsigned char>(caracter)));
    }
}

void LineaDispatcher::registrarSaltoLinea() const
{
    std::cout << '\n';
}
