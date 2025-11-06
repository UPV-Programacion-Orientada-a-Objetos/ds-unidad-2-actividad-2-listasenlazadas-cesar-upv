#include "ArduinoParser.h"

#include "AuxiliarCli.h"
#include "LineaDispatcher.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace {

speed_t traducirBaudRate(unsigned baud)
{
    switch (baud) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    default:
        return B0;
    }
}

} // namespace

ArduinoParser::ArduinoParser(AuxiliarCli* logger, LineaDispatcher* target) noexcept
    : _fd(-1)
    , _preset(Preset::ACM0)
    , _baud(115200)
    , _logger(logger)
    , _target(target)
{
    _customPath[0] = '\0';
}

void ArduinoParser::setPreset(Preset p) noexcept
{
    _preset = p;
}

void ArduinoParser::setCustomPath(const char* path) noexcept
{
    if (!path) {
        _customPath[0] = '\0';
        return;
    }

    std::size_t longitud = std::strlen(path);
    if (longitud > kMaxRuta) {
        longitud = kMaxRuta;
    }
    std::memcpy(_customPath, path, longitud);
    _customPath[longitud] = '\0';
}

void ArduinoParser::setBaudrate(unsigned baud) noexcept
{
    _baud = baud;
}

void ArduinoParser::setTarget(LineaDispatcher* target) noexcept
{
    _target = target;
}

Preset ArduinoParser::getPreset() const noexcept
{
    return _preset;
}

unsigned ArduinoParser::getBaudrate() const noexcept
{
    return _baud;
}

const char* ArduinoParser::defaultPathFor(Preset p) noexcept
{
    switch (p) {
    case Preset::ACM0:
        return "/dev/ttyACM0";
    case Preset::USB0:
        return "/dev/ttyUSB0";
    case Preset::Custom:
    default:
        return "";
    }
}

bool ArduinoParser::openPort()
{
    if (_fd >= 0) {
        closePort();
    }

    if (_preset == Preset::Custom) {
        if (_logger) {
            _logger->imprimirLog("WARNING", "El preset Custom está deshabilitado.");
        }
        return false;
    }

    char ruta[kMaxRuta + 1];
    const char* predeterminada = defaultPathFor(_preset);
    std::strncpy(ruta, predeterminada, sizeof(ruta));
    ruta[sizeof(ruta) - 1] = '\0';

    _fd = ::open(ruta, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_fd < 0) {
        if (_logger) {
            _logger->imprimirLog("ERROR", "No se pudo abrir el puerto serie.");
        }
        return false;
    }

    termios opciones {};
    if (tcgetattr(_fd, &opciones) != 0) {
        if (_logger) {
            _logger->imprimirLog("ERROR", "tcgetattr falló al leer la configuración.");
        }
        closePort();
        return false;
    }

    speed_t velocidad = traducirBaudRate(_baud);
    if (velocidad == B0) {
        if (_logger) {
            _logger->imprimirLog("WARNING", "Baudrate no soportado, se usará 115200.");
        }
        velocidad = B115200;
    }

    cfsetispeed(&opciones, velocidad);
    cfsetospeed(&opciones, velocidad);

    opciones.c_cflag |= (CLOCAL | CREAD);
    opciones.c_cflag &= ~CSIZE;
    opciones.c_cflag |= CS8;
    opciones.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);

    opciones.c_iflag &= ~(IXON | IXOFF | IXANY);
    opciones.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    opciones.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opciones.c_oflag &= ~OPOST;

    opciones.c_cc[VMIN] = 0;
    opciones.c_cc[VTIME] = 1;

    if (tcsetattr(_fd, TCSANOW, &opciones) != 0) {
        if (_logger) {
            _logger->imprimirLog("ERROR", "tcsetattr falló al configurar el puerto.");
        }
        closePort();
        return false;
    }

    int flags = 0;
    if (ioctl(_fd, TIOCMGET, &flags) != -1) {
        flags |= (TIOCM_DTR | TIOCM_RTS);
        ioctl(_fd, TIOCMSET, &flags);
    } else if (_logger) {
        _logger->imprimirLog("WARNING", "No se pudieron activar DTR/RTS.");
    }

    int fcntlFlags = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, fcntlFlags & ~O_NONBLOCK);

    if (_logger) {
        _logger->imprimirLog("STATUS", "Puerto serie abierto correctamente.");
    }

    return true;
}

void ArduinoParser::closePort() noexcept
{
    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
        if (_logger) {
            _logger->imprimirLog("STATUS", "Puerto serie cerrado.");
        }
    }
}

bool ArduinoParser::listenUntilEnter()
{
    if (_fd < 0) {
        if (_logger) {
            _logger->imprimirLog("ERROR", "Puerto serie no disponible.");
        }
        return false;
    }

    if (_logger) {
        _logger->imprimirLog("STATUS", "ENTER detiene la captura.");
    }

    char recibido[256];
    std::size_t usados = 0;
    bool overflow = false;

    bool continuar = true;
    while (continuar) {
        fd_set lectura;
        FD_ZERO(&lectura);
        FD_SET(_fd, &lectura);
        FD_SET(STDIN_FILENO, &lectura);

        const int maxFd = (_fd > STDIN_FILENO) ? _fd : STDIN_FILENO;
        const int resultado = select(maxFd + 1, &lectura, nullptr, nullptr, nullptr);
        if (resultado < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (_logger) {
                _logger->imprimirLog("ERROR", "select() reportó un fallo.");
            }
            return false;
        }

        if (FD_ISSET(STDIN_FILENO, &lectura)) {
            char buffer[32];
            const ssize_t leidos = ::read(STDIN_FILENO, buffer, sizeof(buffer));
            if (leidos > 0) {
                for (ssize_t i = 0; i < leidos; ++i) {
                    if (buffer[i] == '\n') {
                        continuar = false;
                        break;
                    }
                }
            }
        }

        if (!continuar) {
            break;
        }

        if (FD_ISSET(_fd, &lectura)) {
            char buffer[64];
            const ssize_t leidos = ::read(_fd, buffer, sizeof(buffer));
            if (leidos > 0) {
                for (ssize_t i = 0; i < leidos; ++i) {
                    char c = buffer[i];
                    if (c == '\r') {
                        continue;
                    }
                    if (c == '\n') {
                        if (usados > 0) {
                            recibido[usados] = '\0';
                            if (_target) {
                                _target->onRawLine(recibido);
                            }
                        } else if (overflow && _logger) {
                            _logger->imprimirLog("WARNING", "Trama descartada por exceder el buffer.");
                        }
                        usados = 0;
                        overflow = false;
                    } else {
                        if (usados + 1 < sizeof(recibido)) {
                            recibido[usados++] = c;
                        } else {
                            overflow = true;
                        }
                    }
                }
            } else if (leidos == 0) {
                if (_logger) {
                    _logger->imprimirLog("WARNING", "Desconexión detectada en el puerto serie.");
                }
                return false;
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                if (_logger) {
                    _logger->imprimirLog("WARNING", "Fallo al leer del puerto serie.");
                }
                return false;
            }
        }
    }

    return true;
}
