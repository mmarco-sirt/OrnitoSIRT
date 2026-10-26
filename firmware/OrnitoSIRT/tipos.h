// ═══════════════════════════════════════════════════════════════════
//  OrnitoSIRT · tipos de datos
//  Van en un .h para que el IDE de Arduino los conozca antes de generar
//  los prototipos de las funciones del .ino.
// ═══════════════════════════════════════════════════════════════════
#pragma once
#include <stdint.h>

enum Escena : uint8_t {
  ESC_ARRANQUE, ESC_WIFI, ESC_CARA, ESC_LOGO, ESC_SERVICIO,
  ESC_RADAR, ESC_ATAQUE, ESC_TEXTO, ESC_TERMINAL, ESC_LIENZO
};

// El orden debe coincidir con la lista CARAS de pagina_web.h
enum Animo : uint8_t {
  A_NORMAL, A_FELIZ, A_ENFADADO, A_TRISTE, A_SORPRENDIDO, A_SOSPECHOSO,
  A_CANSADO, A_ENAMORADO, A_EMOCIONADO, A_MAREADO, A_DORMIDO, A_VIGILANTE, N_ANIMOS
};

enum Accion : uint8_t { AC_NADA, AC_GUINO, AC_SI, AC_NO, AC_MIRAR, AC_SALTO };

// Forma de un ojo: ancho, alto, radio de esquina, desplazamiento vertical,
// inclinación del párpado (+ enfado, − tristeza), párpado superior plano
// (0..1) y corte inferior redondo de ojo sonriente (0..1).
struct Ojo { float w, h, r, dy, incl, parpSup, parpInf; };
struct Rect { int x, y, w, h; };

struct Ajustes {
  uint8_t version;
  uint8_t brillo;     // 5..100 %
  uint8_t velocidad;  // 1 lenta · 2 normal · 3 rápida
  uint8_t ojos;
  uint8_t fondo;
  uint8_t rotacion;
  bool autonomo, luz, invertir, presAlArrancar;
};

struct Paso { uint8_t escena, arg; uint16_t dur; };

#define TERM_FILAS 10
#define TERM_COLS  19
#define COLA_TERM  12
struct LineaTerm { char txt[TERM_COLS + 1]; uint8_t tipo; };  // tipo: 0 orden · 1 texto · 2 destacado · 3 error
struct SalidaTerm { char txt[TERM_COLS + 1]; uint8_t tipo; uint16_t retardo; };

struct Blip { int16_t x, y; float ang; };
