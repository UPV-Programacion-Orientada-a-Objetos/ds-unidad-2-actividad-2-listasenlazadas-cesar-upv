#pragma once

#include <cstddef>

class AuxiliarCli;
class ListaDeCarga;
class RotorDeMapeo;

/**
 * @file LineaDispatcher.h
 * @brief Declara el despachador que interpreta las líneas del protocolo PRT-7.
 */
/**
 * @class LineaDispatcher
 * @brief Gestiona las tramas crudas recibidas y coordina la decodificación.
 */
class LineaDispatcher {
public:
    /**
     * @brief Construye el dispatcher con sus colaboradores opcionales.
     * @param carga Lista donde se ensamblan los caracteres.
     * @param rotor Rotor que traduce los caracteres.
     * @param logger Mecanismo opcional para registrar mensajes.
     */
    LineaDispatcher(ListaDeCarga* carga = nullptr, RotorDeMapeo* rotor = nullptr, AuxiliarCli* logger = nullptr) noexcept;

    /**
     * @brief Define el logger a utilizar.
     * @param logger Nuevo logger; puede ser nulo.
     */
    void setLogger(AuxiliarCli* logger) noexcept;

    /**
     * @brief Configura la lista y el rotor que serán manipulados.
     * @param carga Lista destino.
     * @param rotor Rotor responsable del mapeo.
     */
    void setComponentes(ListaDeCarga* carga, RotorDeMapeo* rotor) noexcept;

    /**
     * @brief Inicia una nueva sesión de decodificación.
     * @param motivo Texto que describe el origen del reinicio.
     * @param limpiar Si es true, limpia la lista y reinicia el rotor.
     */
    void iniciarSesion(const char* motivo = "INICIO", bool limpiar = true);

    /**
     * @brief Termina la sesión actual; se ignoran tramas hasta reactivarla.
     */
    void terminarSesion() noexcept;

    /**
     * @brief Indica si hay una sesión activa.
     * @return true cuando se aceptan tramas.
     */
    bool sesionActiva() const noexcept;

    /**
     * @brief Procesa una línea cruda del puerto serie.
     *
     * Se ignoran todas las tramas hasta recibir la palabra clave "INICIO".
     * Posteriormente, cada línea válida genera los logs correspondientes,
     * crea la trama adecuada y la procesa.
     *
     * @param linea Texto recibido (sin incluir el salto de línea final).
     */
    void onRawLine(const char* linea);

    /**
     * @brief Obtiene el número de líneas procesadas exitosamente.
     * @return Contador de tramas válidas.
     */
    std::size_t totalProcesado() const noexcept;

private:
    ListaDeCarga* _carga;
    RotorDeMapeo* _rotor;
    AuxiliarCli* _logger;
    std::size_t _procesadas;
    bool _sesionActiva;

    bool procesarCarga(char* payload);
    bool procesarMapa(char* payload);
    static char interpretarTokenCarga(const char* payload, bool& valido);
    void log(const char* tipo, const char* mensaje) const;
    static void describirCaracter(char caracter, char* destino, std::size_t tam);
    void registrarSaltoLinea() const;
};
