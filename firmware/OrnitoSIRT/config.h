// ═══════════════════════════════════════════════════════════════════
//  OrnitoSIRT · configuración
//  Todo lo que normalmente hay que tocar está aquí.
// ═══════════════════════════════════════════════════════════════════
#pragma once

// ── Pantalla ST7789 1,3" 240×240 ──────────────────────────────────
//  Cableado (igual que clawd-mochi):
//    SCL/SCK → GPIO 8    SDA/MOSI → GPIO 10   RES → GPIO 2
//    DC      → GPIO 1    CS       → GPIO 4    BLK → GPIO 3
//    VCC     → 3V3 (¡nunca 5 V!)              GND → GND
//  Los módulos de 1,3" de 7 pines no tienen CS: no pasa nada, el pin 4
//  se queda sin conectar. El modo SPI 3 funciona con y sin CS.
#define PIN_TFT_SCK    8
#define PIN_TFT_MOSI   10
#define PIN_TFT_RST    2
#define PIN_TFT_DC     1
#define PIN_TFT_CS     4
#define PIN_TFT_BL     3

#define TFT_SPI_HZ     40000000   // si salen rayas o basura, prueba 27000000
#define TFT_SPI_MODO   SPI_MODE3

// Valores de fábrica. Se pueden cambiar desde la web (Ajustes) y se guardan.
#define TFT_ROTACION   1          // 0..3 · gira la imagen si sale torcida
#define TFT_INVERTIR   true       // paneles IPS: true · si los colores salen en negativo: false

// ── WiFi (punto de acceso propio) ────────────────────────────────
#define WIFI_NOMBRE    "OrnitoSIRT"
#define WIFI_CLAVE     "ornitosirt"   // mínimo 8 caracteres · cambiadla antes de entregar
#define WIFI_CANAL     6
#define WIFI_SUFIJO_MAC 0             // 1 = "OrnitoSIRT-A1B2" (útil si hay varios cubos juntos)
#define PORTAL_CAUTIVO 1              // 1 = al conectarse, el móvil abre el panel solo
// Las placas ESP32-C3 Super Mini tienen una antena pequeña que da problemas
// a máxima potencia. A 8,5 dBm la conexión es mucho más estable (alcance ~10 m).
#define WIFI_POTENCIA_REDUCIDA 1

// ── Comportamiento ───────────────────────────────────────────────
#define MINUTOS_PARA_DORMIR  5        // sin interacción → bosteza y se duerme (0 = nunca)
#define SEGUNDOS_PANTALLA_WIFI 60     // tras arrancar, tiempo máximo mostrando el QR
#define FPS_OBJETIVO         30

#define VERSION_FIRMWARE "1.0"
