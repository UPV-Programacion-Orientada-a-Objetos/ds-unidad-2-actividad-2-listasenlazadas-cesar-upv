#ifndef AUXILIARCLI_H
#define AUXILIARCLI_H

#include <iostream>
#include <limits>
#include <cstring>

/**
 * @file AuxiliarCli.h
 * @brief Utilidades para interacción en consola y mensajes coloreados.
 */
/**
 * @brief Auxilia a la interfaz de consola con logs y lectura validada.
 */
class AuxiliarCli
{
public:
    AuxiliarCli() = default;

    /**
     * @brief Imprime un mensaje con color según la etiqueta proporcionada.
     * @param tipo Texto que describe el tipo de log (STATUS, WARNING, SUCCESS, ERROR).
     * @param msj Mensaje a desplegar.
     */
    void imprimirLog(const char* tipo, const char* msj)
    {
        if (!tipo || !msj)
        {
            return;
        }

        int colorId = 37;
        if (coincide(tipo, "RED") || coincide(tipo, "ERROR") || coincide(tipo, "error"))
        {
            colorId = 31;
        }
        else if (coincide(tipo, "GREEN") || coincide(tipo, "SUCCESS") || coincide(tipo, "success"))
        {
            colorId = 32;
        }
        else if (coincide(tipo, "YELLOW") || coincide(tipo, "WARNING") || coincide(tipo, "warning"))
        {
            colorId = 33;
        }
        else if (coincide(tipo, "CYAN") || coincide(tipo, "STATUS") || coincide(tipo, "status"))
        {
            colorId = 36;
        }

        std::cout << "\033[" << colorId << "m";
        std::cout << "[" << tipo << "] " << msj << std::endl;
        std::cout << "\033[0m";
    }

    /**
     * @brief Solicita un dato primitivo y valida la entrada.
     * @param mensaje Texto guía que se muestra antes de leer.
     * @param valor Referencia donde se almacenará el dato válido.
     */
    template <typename T>
    void obtenerDato(const char* mensaje, T& valor)
    {
        if (!mensaje)
        {
            mensaje = "Entrada";
        }

        std::cout << mensaje << ": ";
        while (!(std::cin >> valor))
        {
            imprimirLog("WARNING", "Entrada no válida, por favor, intente de nuevo.");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << mensaje << ": ";
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    /**
     * @brief Lee una cadena con espacios y obliga a repetir si queda vacía.
     * @param mensaje Texto guía que se muestra antes de leer.
     * @param destino Buffer donde se almacena la cadena resultante.
     * @param capacidad Número máximo de caracteres aceptados (incluye terminador nulo).
     */
    void obtenerCadena(const char* mensaje, char* destino, std::size_t capacidad)
    {
        if (!destino || capacidad == 0)
        {
            imprimirLog("WARNING", "Buffer inválido para lectura de cadena.");
            return;
        }

        if (!mensaje)
        {
            mensaje = "Entrada";
        }

        std::cout << mensaje << ": ";
        std::cin.getline(destino, static_cast<std::streamsize>(capacidad));

        while (destino[0] == '\0')
        {
            if (std::cin.fail())
            {
                std::cin.clear();
            }
            imprimirLog("WARNING", "Entrada vacía, por favor, intente de nuevo.");
            std::cout << mensaje << ": ";
            std::cin.getline(destino, static_cast<std::streamsize>(capacidad));
        }
    }

private:
    bool coincide(const char* texto, const char* referencia) const
    {
        return std::strcmp(texto, referencia) == 0;
    }
};

#endif
