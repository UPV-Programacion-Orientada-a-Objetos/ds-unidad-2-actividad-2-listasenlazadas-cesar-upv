#pragma once

#include <cstddef>

/**
 * @file ArduinoParser.h
 * @brief Declaraciones de la interfaz que gestiona el puerto serie del Arduino.
 */

class AuxiliarCli;
class LineaDispatcher;

/**
 * @brief Presets válidos para elegir el dispositivo serie.
 *
 * - Preset::ACM0 apunta a "/dev/ttyACM0".
 * - Preset::USB0 apunta a "/dev/ttyUSB0".
 * - Preset::Custom usa la ruta entregada mediante setCustomPath().
 * - Nunca se emplean rutas del tipo /dev/serial/by-id/.
 */
enum class Preset { ACM0, USB0, Custom };

/**
 * @brief Gestiona las lecturas crudas del puerto serie y las reenvía.
 *
 * Esta clase solo entrega cada línea completa al LineaDispatcher configurado.
 * Cualquier detalle de configuración POSIX se realiza en la implementación.
 */
class ArduinoParser {
public:
    /**
     * @brief Construye el parser con apuntadores opcionales a logger y destino.
     * @param logger Instancia para mostrar mensajes de estado o advertencia. Puede ser nulo.
     * @param target Receptor de líneas crudas. Puede ser nulo hasta que se configure.
     */
    explicit ArduinoParser(AuxiliarCli* logger = nullptr, LineaDispatcher* target = nullptr) noexcept;

    /**
     * @brief Define el preset que resolverá la ruta del dispositivo serie.
     * @param p Nuevo preset a emplear al abrir el puerto.
     */
    void setPreset(Preset p) noexcept;

    /**
     * @brief Almacena la ruta personalizada cuando el preset sea Preset::Custom.
     *
     * En la aplicación actual el preset Custom está deshabilitado, por lo que esta
     * configuración no se utiliza.
     *
     * @param path Ruta completa hacia el dispositivo serie elegida por el usuario.
     */
    void setCustomPath(const char* path) noexcept;

    /**
     * @brief Ajusta el baudrate que se intentará aplicar al abrir el puerto.
     * @param baud Valor deseado; por defecto 115200 (configuración 8N1 sin flow control).
     */
    void setBaudrate(unsigned baud = 115200) noexcept;

    /**
     * @brief Cambia el objetivo que recibirá las líneas crudas.
     * @param target Instancia de LineaDispatcher; puede ser nula para desactivar el reenvío.
     */
    void setTarget(LineaDispatcher* target) noexcept;

    /**
     * @brief Devuelve el preset actualmente configurado.
     * @return Valor del preset activo.
     */
    Preset getPreset() const noexcept;

    /**
     * @brief Devuelve el baudrate que se aplicará al abrir el puerto.
     * @return Valor numérico del baudrate.
     */
    unsigned getBaudrate() const noexcept;

    /**
     * @brief Abre y configura el puerto serie según el preset y la ruta personalizada.
     * @return true si el descriptor se abrió y configuró correctamente; false en caso contrario.
     */
    bool openPort();

    /**
     * @brief Cierra el descriptor abierto, si existe.
     */
    void closePort() noexcept;

    /**
     * @brief Inicia el ciclo de lectura hasta que el usuario presione ENTER en STDIN.
     *
     * Cada línea terminada en '\n' debe reenviarse mediante LineaDispatcher::onRawLine().
     * LineaDispatcher es quien valida y procesa cada cadena recibida.
     *
     * @return true cuando el bucle concluyó sin fallas fatales; false en caso de error.
     */
    bool listenUntilEnter();

    /**
     * @brief Devuelve la ruta predeterminada asociada a un preset dado.
     * @param p Preset a consultar.
     * @return Cadena literal con la ruta correspondiente.
     */
    static const char* defaultPathFor(Preset p) noexcept;

private:
    static const std::size_t kMaxRuta = 255;

    int _fd;
    Preset _preset;
    unsigned _baud;
    char _customPath[kMaxRuta + 1];
    AuxiliarCli* _logger;
    LineaDispatcher* _target;
};
