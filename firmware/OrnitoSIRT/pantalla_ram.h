// ═══════════════════════════════════════════════════════════════════
//  OrnitoSIRT · framebuffer en RAM partido en franjas
//
//  La imagen completa (240×240×2 = 112 KB) no cabe en un único bloque:
//  la memoria del ESP32-C3 está fragmentada. Se reparte en franjas de
//  60 filas (28 KB) que se reservan por separado. Se dibuja aquí con las
//  funciones normales de Adafruit GFX y luego se envía a la pantalla de una
//  vez, así no hay parpadeos.
// ═══════════════════════════════════════════════════════════════════
#pragma once
#include <Adafruit_GFX.h>
#include <stdlib.h>

class PantallaRAM : public Adafruit_GFX {
 public:
  static const int W = 240, H = 240, FILAS = 60, FRANJAS = H / FILAS;

  PantallaRAM() : Adafruit_GFX(W, H) {}

  bool reserva() {
    for (int k = 0; k < FRANJAS; k++) {
      franja[k] = (uint16_t *)malloc(W * FILAS * sizeof(uint16_t));
      if (!franja[k]) {
        for (int j = 0; j < k; j++) { free(franja[j]); franja[j] = nullptr; }
        return false;
      }
    }
    return true;
  }

  inline uint16_t *fila(int y) { return franja[y / FILAS] + (y % FILAS) * W; }

  template <class Pantalla> void envia(Pantalla &tft) {
    for (int k = 0; k < FRANJAS; k++) tft.drawRGBBitmap(0, k * FILAS, franja[k], W, FILAS);
  }

  void drawPixel(int16_t x, int16_t y, uint16_t c) override {
    if ((unsigned)x < (unsigned)W && (unsigned)y < (unsigned)H) fila(y)[x] = c;
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t c) override {
    if ((unsigned)y >= (unsigned)H || w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > W) w = W - x;
    uint16_t *p = fila(y) + x;
    while (w-- > 0) *p++ = c;
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t c) override {
    if ((unsigned)x >= (unsigned)W || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > H) h = H - y;
    for (; h > 0; h--, y++) fila(y)[x] = c;
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) override {
    if (w <= 0 || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > H) h = H - y;
    for (; h > 0; h--, y++) drawFastHLine(x, y, w, c);
  }

  void fillScreen(uint16_t c) override {
    for (int k = 0; k < FRANJAS; k++) {
      uint16_t *p = franja[k];
      for (int i = 0; i < W * FILAS; i++) p[i] = c;
    }
  }

 private:
  uint16_t *franja[FRANJAS] = {};
};
