#include <cctype>
#include <cstring>
#include <iostream>
#include <limits>

#include "ArduinoParser.h"
#include "AuxiliarCli.h"
#include "LineaDispatcher.h"
#include "ListaDeCarga.h"
#include "RotorDeMapeo.h"

/**
 * @brief Elimina espacios iniciales y finales del buffer recibido.
 * @param texto Cadena a limpiar.
 */
static void recortarEnLugar(char* texto);

/**
 * @brief Muestra el menú principal con la configuración actual.
 * @param rutaActual Texto con la ruta del dispositivo serie.
 * @param baud Baudrate configurado.
 */
static void imprimirMenuPrincipal(const char* rutaActual, unsigned baud);

/**
 * @brief Permite seleccionar interactívamente el preset del puerto serie.
 * @param logger Utilidad de logging y lectura validada.
 * @param parser Parser que recibirá el nuevo preset.
 */
static void configurarPresetInteractivo(AuxiliarCli& logger, ArduinoParser& parser);

/**
 * @brief Ajusta el baudrate del parser entre los presets disponibles.
 * @param logger Utilidad de logging y lectura validada.
 * @param parser Parser al que se aplicará el baudrate.
 */
static void configurarBaudrateInteractivo(AuxiliarCli& logger, ArduinoParser& parser);

/**
 * @brief Ejecuta el modo simulación con entrada manual o dataset de ejemplo.
 * @param logger Utilidad para mensajes y lectura.
 * @param dispatcher Dispatcher encargado de procesar las tramas simuladas.
 * @param lista Lista utilizada para reconstruir el mensaje (se reutiliza).
 */
static void menuSimulacion(AuxiliarCli& logger, LineaDispatcher& dispatcher, ListaDeCarga& lista);

/**
 * @brief Captura tramas reales desde el puerto serie visible en tiempo real.
 * @param logger Utilidad para mensajes.
 * @param parser Parser que realiza la lectura del puerto.
 * @param dispatcher Dispatcher que procesa las tramas recibidas.
 */
static void ejecutarCapturaSerie(AuxiliarCli& logger, ArduinoParser& parser, LineaDispatcher& dispatcher);

/**
 * @brief Punto de entrada del decodificador interactivo PRT-7.
 * @return Código de salida del programa.
 */
int main()
{
    AuxiliarCli logger;
    ListaDeCarga lista;
    RotorDeMapeo rotor;
    LineaDispatcher dispatcher(&lista, &rotor, &logger);
    ArduinoParser parser(&logger, &dispatcher);

    bool salir = false;
    logger.imprimirLog("STATUS", "Decodificador PRT-7 listo.");

    while (!salir) {
        const char* rutaActual = ArduinoParser::defaultPathFor(parser.getPreset());
        const unsigned baudActual = parser.getBaudrate();
        imprimirMenuPrincipal(rutaActual, baudActual);

        int opcion = -1;
        logger.obtenerDato("Seleccione una opción", opcion);

        switch (opcion) {
        case 1:
            configurarPresetInteractivo(logger, parser);
            break;
        case 2:
            configurarBaudrateInteractivo(logger, parser);
            break;
        case 3:
            menuSimulacion(logger, dispatcher, lista);
            break;
        case 4:
            ejecutarCapturaSerie(logger, parser, dispatcher);
            break;
        case 0:
            salir = true;
            break;
        default:
            logger.imprimirLog("WARNING", "Opción no reconocida.");
            break;
        }
    }

    logger.imprimirLog("STATUS", "Programa finalizado.");
    return 0;
}

void recortarEnLugar(char* texto)
{
    if (!texto) {
        return;
    }

    std::size_t longitud = std::strlen(texto);
    while (longitud > 0 && std::isspace(static_cast<unsigned char>(texto[longitud - 1]))) {
        texto[--longitud] = '\0';
    }

    std::size_t inicio = 0;
    while (texto[inicio] != '\0' && std::isspace(static_cast<unsigned char>(texto[inicio]))) {
        ++inicio;
    }

    if (inicio > 0) {
        std::memmove(texto, texto + inicio, longitud - inicio + 1);
    }
}

void imprimirMenuPrincipal(const char* rutaActual, unsigned baud)
{
    std::cout << "\nDecodificador PRT-7\n"
                 "Dispositivo: " << (rutaActual ? rutaActual : "(sin definir)") << "\n"
                 "Baudrate: " << baud << "\n"
                 "────────────────────────────────────────────────\n"
                 "1 | Seleccionar preset del puerto serie\n"
                 "2 | Ajustar baudrate\n"
                 "3 | Ejecutar simulación\n"
                 "4 | Capturar desde el dispositivo serie\n"
                 "0 | Salir\n";
}

void configurarPresetInteractivo(AuxiliarCli& logger, ArduinoParser& parser)
{
    std::cout << "\nPresets disponibles:\n"
                 "────────────────────────────────\n"
                 "0 | Cancelar\n"
                 "1 | /dev/ttyACM0\n"
                 "2 | /dev/ttyUSB0\n";
    int opcion = 0;
    logger.obtenerDato("Seleccione un preset", opcion);

    switch (opcion) {
    case 0:
        logger.imprimirLog("STATUS", "Selección cancelada.");
        break;
    case 1:
        parser.setPreset(Preset::ACM0);
        logger.imprimirLog("STATUS", "Preset ACM0 seleccionado.");
        break;
    case 2:
        parser.setPreset(Preset::USB0);
        logger.imprimirLog("STATUS", "Preset USB0 seleccionado.");
        break;
    default:
        logger.imprimirLog("WARNING", "Opción de preset no válida.");
        break;
    }
}

void configurarBaudrateInteractivo(AuxiliarCli& logger, ArduinoParser& parser)
{
    std::cout << "\nBaudrates disponibles:\n"
                 "────────────────────────────────\n"
                 "0 | Cancelar\n"
                 "1 | 115200\n"
                 "2 | 9600\n";
    int opcion = 0;
    logger.obtenerDato("Seleccione una opción", opcion);

    switch (opcion) {
    case 0:
        logger.imprimirLog("STATUS", "Baudrate sin cambios.");
        break;
    case 1:
        parser.setBaudrate(115200);
        logger.imprimirLog("STATUS", "Baudrate ajustado a 115200.");
        break;
    case 2:
        parser.setBaudrate(9600);
        logger.imprimirLog("STATUS", "Baudrate ajustado a 9600.");
        break;
    default:
        logger.imprimirLog("WARNING", "Opción de baudrate no válida.");
        break;
    }
}

void menuSimulacion(AuxiliarCli& logger, LineaDispatcher& dispatcher, ListaDeCarga& lista)
{
    logger.imprimirLog("STATUS", "Modo simulación seleccionado.");
    (void)lista;

    std::cout << "\nOpciones de simulación:\n"
                 "────────────────────────────────\n"
                 "0 | Cancelar\n"
                 "1 | Capturar tramas ingresadas manualmente\n"
                 "2 | Usar la secuencia de ejemplo del README\n";
    int opcion = 0;
    logger.obtenerDato("Seleccione una opción", opcion);

    switch (opcion) {
    case 0:
        logger.imprimirLog("STATUS", "Simulación cancelada.");
        dispatcher.terminarSesion();
        return;
    case 1: {
        dispatcher.iniciarSesion("SIMULACION", true);
        std::cout << "Ingrese tramas (escriba TERMINAR para finalizar):\n";
        char linea[128];
        while (true) {
            std::cin.getline(linea, sizeof(linea));
            if (!std::cin) {
                if (std::cin.eof()) {
                    std::cin.clear();
                    break;
                }
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            recortarEnLugar(linea);
            if (linea[0] == '\0') {
                continue;
            }

            char mayus[128];
            std::size_t idx = 0;
            for (; linea[idx] != '\0' && idx < sizeof(mayus) - 1; ++idx) {
                mayus[idx] = static_cast<char>(std::toupper(static_cast<unsigned char>(linea[idx])));
            }
            mayus[idx] = '\0';

            if (std::strcmp(mayus, "TERMINAR") == 0) {
                logger.imprimirLog("STATUS", "Entrada manual finalizada.");
                break;
            }

            dispatcher.onRawLine(linea);
        }
        break;
    }
    case 2: {
        dispatcher.iniciarSesion("SIMULACION", true);
        static const char* ejemplo[] = {
            "L,H", "L,O", "L,L", "M,2",
            "L,A", "L,Space", "L,W", "M,-2",
            "L,O", "L,R", "L,L", "L,D"
        };
        const std::size_t total = sizeof(ejemplo) / sizeof(ejemplo[0]);
        for (std::size_t i = 0; i < total; ++i) {
            dispatcher.onRawLine(ejemplo[i]);
        }
        break;
    }
    default:
        logger.imprimirLog("WARNING", "Opción de simulación no válida.");
        dispatcher.terminarSesion();
        return;
    }

    logger.imprimirLog("SUCCESS", "Simulación finalizada.");
    dispatcher.terminarSesion();
}

void ejecutarCapturaSerie(AuxiliarCli& logger, ArduinoParser& parser, LineaDispatcher& dispatcher)
{
    logger.imprimirLog("STATUS", "Preparando captura desde el puerto serie.");
    dispatcher.terminarSesion();

    if (!parser.openPort()) {
        logger.imprimirLog("ERROR", "No se pudo abrir el puerto serie.");
        return;
    }

    logger.imprimirLog("STATUS", "Esperando marcador INICIO desde el dispositivo...");

    const bool exito = parser.listenUntilEnter();
    parser.closePort();

    if (exito) {
        logger.imprimirLog("SUCCESS", "Captura finalizada correctamente.");
    } else {
        logger.imprimirLog("WARNING", "La captura terminó con incidencias.");
    }
    dispatcher.terminarSesion();
}
