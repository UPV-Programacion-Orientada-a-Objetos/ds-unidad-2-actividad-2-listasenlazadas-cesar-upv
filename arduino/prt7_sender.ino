/**
 * @file prt7_sender.ino
 * @brief Emisor de tramas PRT-7 para un ESP32-S3 o Arduino compatible.
 */

#include <Arduino.h>

/**
 * @brief Marca de inicio para indicar que comienza una nueva secuencia.
 */
static const char* kInicio = "INICIO";

/**
 * @brief Tramas cortas de prueba para depurar rápidamente.
 */
static const char* kTramasPrueba[] = {
  "L,H",
  "M,3",
  "L,E",
  "M,-3",
  "L,L",
  "L,A"
};

/**
 * @brief Tramas de ejemplo que siguen el protocolo PRT-7.
 */
static const char* kTramas[] = {
  "L,H",
  "L,O",
  "L,L",
  "M,2",
  "L,A",
  "L,Space",
  "L,W",
  "M,-2",
  "L,O",
  "L,R",
  "L,L",
  "L,D"
};

/**
 * @brief Número de tramas predefinidas.
 */
static const size_t kCantidadTramas = sizeof(kTramas) / sizeof(kTramas[0]);

/**
 * @brief Configura el puerto serie a 115200 8N1 y espera al monitor.
 */
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("# Emisor PRT-7 listo. Enviando una trama cada segundo.");
}

/**
 * @brief Envía "INICIO" y la secuencia de tramas una sola vez con delay(1000) entre líneas.
 */
void loop() {
  Serial.println(kInicio);
  delay(1000);

  for (size_t i = 0; i < kCantidadTramas; ++i) {
    Serial.println(kTramas[i]);
    delay(1000);
  }
}
