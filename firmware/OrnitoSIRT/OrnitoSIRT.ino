/*
 * ╔══════════════════════════════════════════════════════════════════════╗
 *   OrnitoSIRT · la mascota tecnológica de SIRT
 *   ESP32-C3 Super Mini + pantalla ST7789 1,3" (240×240)
 *
 *   1. Al encender: logo SIRT y después un QR en pantalla.
 *   2. Escanea el QR con el móvil (o conéctate al WiFi "OrnitoSIRT",
 *      clave "ornitosirt"). El panel se abre solo; si no, entra en
 *      http://192.168.4.1
 *
 *   Pines, WiFi y pantalla ............. config.h
 *   Panel web .......................... pagina_web.h
 *   Logo, iconos y tipografía Inter .... recursos.h (generado)
 *
 *   Basado en clawd-mochi © 2026 Yousuf Amanuel (licencia MIT).
 * ╚══════════════════════════════════════════════════════════════════════╝
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <math.h>

#include "config.h"
#include "tipos.h"
#include "recursos.h"
#include "pagina_web.h"
#include "pantalla_ram.h"

// QR nativo del core ESP32 (3.x). Si no existe, la pantalla WiFi muestra solo texto.
#if __has_include(<qrcode.h>)
#include <qrcode.h>
#endif
#ifdef ESP_QRCODE_CONFIG_DEFAULT
#define HAY_QR 1
#else
#define HAY_QR 0
#endif

#define ANCHO   240
#define ALTO    240
#define DOS_PI  6.2831853f

// ═════════════════════════════════════════════════════════════
//  COLORES · paleta corporativa de sirt.com
// ═════════════════════════════════════════════════════════════
constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
const uint16_t C_LIMA     = rgb(0xC6, 0xD3, 0x2B);  // primario
const uint16_t C_VERDE    = rgb(0x2A, 0xB1, 0x7E);  // secundario
const uint16_t C_PETROLEO = rgb(0x07, 0x64, 0x71);  // acento
const uint16_t C_MARINO   = rgb(0x07, 0x0F, 0x26);  // fondo (como la carcasa)
const uint16_t C_NOCHE    = rgb(0x00, 0x19, 0x1D);
const uint16_t C_GRIS     = rgb(0xE6, 0xE7, 0xE9);
const uint16_t C_NEGRO    = 0x0000;
const uint16_t C_ALERTA   = rgb(0xF2, 0x5C, 0x3B);  // solo para la demo de ataque
const uint16_t C_LAGRIMA  = rgb(0x8F, 0xE3, 0xD0);
const uint16_t C_CLAVE    = 0x0841;  // color interno: los ojos se pintan así y luego se tiñen

enum { OJOS_DEGRADADO, OJOS_LIMA, OJOS_VERDE, OJOS_BLANCO, N_TEMAS_OJOS };
const uint16_t FONDOS[] = { C_MARINO, C_NOCHE, C_NEGRO };
#define N_FONDOS 3

const char *const NOMBRE_ESCENA[] = {
  "arranque", "wifi", "cara", "logo", "servicio",
  "radar", "ataque", "texto", "terminal", "lienzo"
};

// ═════════════════════════════════════════════════════════════
//  ESTADO GLOBAL
// ═════════════════════════════════════════════════════════════
Adafruit_ST7789 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
PantallaRAM ram;                 // framebuffer: se dibuja aquí y se envía entero (sin parpadeos)
Adafruit_GFX *g = &tft;          // destino de dibujo (ram, o la pantalla si no hay memoria)
bool hayFB = false;
uint16_t fondoSinFB = C_MARINO;

WebServer server(80);
DNSServer dns;
Preferences prefs;
const IPAddress IP_AP(192, 168, 4, 1);
char nombreWifi[33];
volatile bool nuevoCliente = false;

#define VERSION_AJUSTES 1
Ajustes aj = { VERSION_AJUSTES, 80, 2, OJOS_DEGRADADO, 0, TFT_ROTACION, true, true, TFT_INVERTIR, false };
bool ajustesSucios = false;
uint32_t ajustesCambio = 0;

// Reloj de animación en ms "virtuales": corre más o menos según la velocidad
uint32_t t = 0, restoReloj = 0, relojAnt = 0;

uint8_t escena = ESC_ARRANQUE, escenaArg = 0;
uint32_t escenaT0 = 0, escenaDur = 0;
bool presentacion = false, wifiArranque = false;
uint8_t presPaso = 0;

uint32_t ultimaInteraccion = 0, dormirEn = 0;
bool dormido = false, dormirPendiente = false;

// ── Cara ──────────────────────────────────────────────────────
#define CARA_CY   108
#define CARA_SEP  60   // distancia del centro de cada ojo al centro de la pantalla
#define MIRADA_X  20   // cuánto se desplazan los ojos al mirar a un lado (px)
#define MIRADA_Y  16

//                        w    h   r   dy  incl  pSup   pInf
#define OJO_NORMAL      { 68,  92, 22,  0,   0,  0,     0     }
const Ojo FORMA[N_ANIMOS][2] = {
  /* NORMAL      */ { OJO_NORMAL, OJO_NORMAL },
  /* FELIZ       */ { { 72,  84, 26, -6,   0,  0,     0.45f }, { 72,  84, 26, -6,   0,  0,     0.45f } },
  /* ENFADADO    */ { { 68,  78, 16,  4,  30,  0.06f, 0     }, { 68,  78, 16,  4,  30,  0.06f, 0     } },
  /* TRISTE      */ { { 64,  76, 18,  8, -26,  0.04f, 0     }, { 64,  76, 18,  8, -26,  0.04f, 0     } },
  /* SORPRENDIDO */ { { 76, 108, 38, -6,   0,  0,     0     }, { 76, 108, 38, -6,   0,  0,     0     } },
  /* SOSPECHOSO  */ { { 70,  92, 18,  2,   0,  0.52f, 0     }, { 66,  78, 18,  0,   0,  0.10f, 0     } },
  /* CANSADO     */ { { 70,  88, 18,  6, -10,  0.48f, 0     }, { 70,  88, 18,  6, -10,  0.48f, 0     } },
  /* ENAMORADO   */ { OJO_NORMAL, OJO_NORMAL },
  /* EMOCIONADO  */ { OJO_NORMAL, OJO_NORMAL },
  /* MAREADO     */ { OJO_NORMAL, OJO_NORMAL },
  /* DORMIDO     */ { OJO_NORMAL, OJO_NORMAL },
  /* VIGILANTE   */ { { 76,  52, 16,  0,  10,  0,     0     }, { 76,  52, 16,  0,  10,  0,     0     } },
};

Ojo ojoAct[2], ojoObj[2];   // 0 = izquierdo, 1 = derecho (vistos de frente)
Rect rectOjo[2];
uint8_t animo = A_NORMAL, animoPendiente = 0xFF, animoTrasTemporal = A_NORMAL;
uint32_t finAnimoTemporal = 0;
float mirX = 0, mirY = 0, mirObjX = 0, mirObjY = 0;
uint32_t mirarManualHasta = 0;
bool parpActivo = false;
uint32_t parpT0 = 0;
uint16_t parpDur = 200;
float aperturaFrame = 1;
uint8_t accion = AC_NADA;
uint32_t accionT0 = 0;
float acDX = 0, acDY = 0, guino = 1;
uint32_t proxParpadeo = 2500, proxMirada = 1500, proxMicro = 25000;
uint8_t carruselIdx = 0;
uint32_t proxCarrusel = 0;
bool saludoMirado = false;
char subtitulo[64] = "";
uint32_t subT0 = 0, subHasta = 0;
uint16_t GRAD_LOGO[256], GRAD_OJO[64], C_BRILLO = 0xFFFF;

// ── Escenas ───────────────────────────────────────────────────
Blip blips[6];
uint8_t nBlips = 0;
uint8_t ataqueFase = 0;
char mensaje[128] = "¡Bienvenidos a SIRT!";
uint8_t mensajeColor = 0;
int mensajeAncho = 0;

LineaTerm term[TERM_FILAS];
uint8_t termN = 0;
char entrada[48];
uint8_t entradaLen = 0;
SalidaTerm colaT[COLA_TERM];
uint8_t colaIni = 0, colaN = 0;
uint32_t proxSalida = 0;

bool lienzoSucio = false;
uint16_t lienzoFondo = C_MARINO;

uint8_t qrTam = 0;
uint8_t qrBits[41 * 41 / 8 + 1];

// Guion del modo presentación (duraciones en ms a velocidad normal)
const Paso PRESENTACION[] = {
  { ESC_CARA,     1, 7000 },   // saludo
  { ESC_LOGO,     0, 7000 },
  { ESC_SERVICIO, 0, 5500 },   // ciberseguridad
  { ESC_SERVICIO, 1, 5500 },   // infraestructura
  { ESC_SERVICIO, 2, 5500 },   // cloud
  { ESC_RADAR,    0, 7600 },
  { ESC_CARA,     2, 9100 },   // carrusel de expresiones
  { ESC_ATAQUE,   0, 7200 },
  { ESC_WIFI,     0, 9000 },
};
const uint8_t N_PASOS = sizeof(PRESENTACION) / sizeof(PRESENTACION[0]);

// ═════════════════════════════════════════════════════════════
//  UTILIDADES DE COLOR Y DIBUJO
// ═════════════════════════════════════════════════════════════

uint16_t mezcla565(uint16_t fg, uint16_t bg, uint8_t a) {
  uint32_t rb = bg & 0xF81F;
  rb += ((fg & 0xF81F) - rb) * (a >> 2) >> 6;
  uint32_t g6 = bg & 0x07E0;
  g6 += ((fg & 0x07E0) - g6) * a >> 8;
  return (rb & 0xF81F) | (g6 & 0x07E0);
}

uint16_t mezclaRGB(uint32_t a, uint32_t b, int k) {   // k: 0..255
  const int r1 = (a >> 16) & 255, g1 = (a >> 8) & 255, b1 = a & 255;
  const int r2 = (b >> 16) & 255, g2 = (b >> 8) & 255, b2 = b & 255;
  return rgb(r1 + (r2 - r1) * k / 255, g1 + (g2 - g1) * k / 255, b1 + (b2 - b1) * k / 255);
}

uint8_t alfaRampa(uint32_t e, uint32_t dur) { return e >= dur ? 255 : e * 255 / dur; }

uint16_t colorFondo() { return FONDOS[aj.fondo % N_FONDOS]; }

void preparaColores() {
  // Degradado del logo SIRT: #C1D42F → #2AB17E (52 %) → #076471
  for (int i = 0; i < 256; i++)
    GRAD_LOGO[i] = i < 133 ? mezclaRGB(0xC1D42F, 0x2AB17E, i * 255 / 133)
                           : mezclaRGB(0x2AB17E, 0x076471, (i - 133) * 255 / 122);
  uint32_t arriba, abajo;
  switch (aj.ojos) {
    case OJOS_LIMA:   arriba = abajo = 0xC6D32B; break;
    case OJOS_VERDE:  arriba = abajo = 0x2AB17E; break;
    case OJOS_BLANCO: arriba = abajo = 0xF2F4F5; break;
    default:          arriba = 0xC6D32B; abajo = 0x2AB17E; break;
  }
  for (int i = 0; i < 64; i++) GRAD_OJO[i] = mezclaRGB(arriba, abajo, i * 255 / 63);
  C_BRILLO = mezcla565(0xFFFF, GRAD_OJO[0], 170);
}

inline void pixelAlfa(int x, int y, uint16_t c, uint8_t a) {
  if ((unsigned)x >= ANCHO || (unsigned)y >= ALTO || !a) return;
  if (hayFB) {
    uint16_t &p = ram.fila(y)[x];
    p = a == 255 ? c : mezcla565(c, p, a);
  } else if (a > 96) {
    tft.drawPixel(x, y, a > 200 ? c : mezcla565(c, fondoSinFB, a));
  }
}

// Pinta un mapa de transparencia de 4 bits (logo, iconos, letras).
// Si gx1 > gx0, el color sigue el degradado SIRT entre esas dos x.
void pintaMapa(const uint8_t *bits, int w, int h, int x0, int y0, uint16_t color, uint8_t alfa,
               int gx0, int gx1, int maxX, int maxFilas) {
  const int gw = gx1 - gx0;
  for (int fy = 0; fy < h && fy < maxFilas; fy++) {
    const int y = y0 + fy;
    if ((unsigned)y >= ALTO) continue;
    for (int fx = 0; fx < w; fx++) {
      const int x = x0 + fx;
      if (x >= maxX) break;
      if ((unsigned)x >= ANCHO) continue;
      const int i = fy * w + fx;
      const uint8_t b = pgm_read_byte(bits + (i >> 1));
      const uint8_t a4 = (i & 1) ? (b & 0x0F) : (b >> 4);
      if (!a4) continue;
      const uint16_t c = gw > 0 ? GRAD_LOGO[constrain((x - gx0) * 255 / gw, 0, 255)] : color;
      pixelAlfa(x, y, c, a4 * 17 * alfa / 255);
    }
  }
}

void dibujaImagen(const ImagenAA &im, int x, int y, uint16_t color, uint8_t alfa, bool degradado,
                  int revelaAncho, int revelaFilas) {
  pintaMapa(im.bits, im.w, im.h, x, y, color, alfa, degradado ? x : 0, degradado ? x + im.w : 0,
            x + revelaAncho, revelaFilas);
}

// Línea horizontal con el degradado de marca (para barras y subrayados)
void lineaDegradado(int x, int y, int w, int h, bool relativo) {
  for (int i = 0; i < w; i++) {
    const int k = relativo ? i * 255 / (w > 1 ? w - 1 : 1) : constrain((x + i) * 255 / (ANCHO - 1), 0, 255);
    g->drawFastVLine(x + i, y, h, GRAD_LOGO[k]);
  }
}

void barraProgreso(uint32_t e) {
  if (escenaDur) lineaDegradado(0, ALTO - 3, constrain((int)(e * ANCHO / escenaDur), 0, ANCHO), 3, false);
}

// Línea gruesa con extremos redondeados
void lineaGruesa(float x0, float y0, float x1, float y1, float grosor, uint16_t c) {
  const float dx = x1 - x0, dy = y1 - y0, L = sqrtf(dx * dx + dy * dy), r = grosor * 0.5f;
  if (L > 0.5f) {
    const float nx = -dy / L * r, ny = dx / L * r;
    const int ax = lroundf(x0 + nx), ay = lroundf(y0 + ny), bx = lroundf(x1 + nx), by = lroundf(y1 + ny);
    const int cx = lroundf(x1 - nx), cy = lroundf(y1 - ny), dx2 = lroundf(x0 - nx), dy2 = lroundf(y0 - ny);
    g->fillTriangle(ax, ay, bx, by, cx, cy, c);
    g->fillTriangle(ax, ay, cx, cy, dx2, dy2, c);
  }
  // Remates redondos sin sobresalir del grosor (si no, las curvas salen "con cuentas")
  const int ri = (int)(r - 0.25f);
  g->fillCircle(lroundf(x0), lroundf(y0), ri, c);
  g->fillCircle(lroundf(x1), lroundf(y1), ri, c);
}

void lineaEscaner(int x, int y, int h) {
  for (int k = 1; k <= 10; k++)
    for (int yy = y; yy < y + h; yy++) pixelAlfa(x - k, yy, C_LIMA, 200 - k * 18);
  g->fillRect(x, y, 2, h, C_GRIS);
}

uint16_t hex565(const String &s) {
  String h = s;
  h.replace("#", "");
  if (h.length() != 6) return C_GRIS;
  const long v = strtol(h.c_str(), nullptr, 16);
  return rgb((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

// ═════════════════════════════════════════════════════════════
//  TEXTO (tipografía Inter con antialiasing, UTF-8: tildes, ñ, ¡, ¿)
// ═════════════════════════════════════════════════════════════

uint32_t leeUTF8(const char *&s) {
  const uint8_t c = *s++;
  if (c < 0x80) return c;
  if ((c & 0xE0) == 0xC0 && (s[0] & 0xC0) == 0x80) return ((c & 0x1F) << 6) | (*s++ & 0x3F);
  if ((c & 0xF0) == 0xE0 && (s[0] & 0xC0) == 0x80 && (s[1] & 0xC0) == 0x80) {
    const uint32_t cp = ((c & 0x0F) << 12) | ((s[0] & 0x3F) << 6) | (s[1] & 0x3F);
    s += 2;
    switch (cp) {   // comillas y guiones tipográficos → equivalentes simples
      case 0x2018: case 0x2019: return '\'';
      case 0x201C: case 0x201D: return '"';
      case 0x2013: case 0x2014: return '-';
    }
    return cp;
  }
  while ((*s & 0xC0) == 0x80) s++;   // emojis y demás: se ignoran
  return 0;
}

const GlifoAA *buscaGlifo(const FuenteAA &f, uint32_t cp) {
  int lo = 0, hi = f.n - 1;
  while (lo <= hi) {
    const int m = (lo + hi) / 2;
    const uint16_t c = f.glifos[m].cp;
    if (c == cp) return &f.glifos[m];
    if (c < cp) lo = m + 1; else hi = m - 1;
  }
  return cp == '?' ? nullptr : buscaGlifo(f, '?');
}

int anchoTexto(const FuenteAA &f, const char *s) {
  int w = 0;
  while (*s) {
    const uint32_t cp = leeUTF8(s);
    if (!cp) continue;
    const GlifoAA *gl = buscaGlifo(f, cp);
    if (gl) w += gl->avance;
  }
  return w;
}

// Devuelve la x final. gx0/gx1: rango del degradado SIRT (0,0 = color sólido).
int dibujaTexto(const FuenteAA &f, const char *s, int x, int yBase, uint16_t color, uint8_t alfa,
                int gx0, int gx1) {
  while (*s && x < ANCHO) {
    const uint32_t cp = leeUTF8(s);
    if (!cp) continue;
    const GlifoAA *gl = buscaGlifo(f, cp);
    if (!gl) continue;
    const int gx = x + gl->dx;
    if (gl->w && gx + gl->w > 0)
      pintaMapa(f.bits + gl->off, gl->w, gl->h, gx, yBase + gl->dy, color, alfa, gx0, gx1, ANCHO, 9999);
    x += gl->avance;
  }
  return x;
}

void textoCentrado(const FuenteAA &f, const char *s, int cx, int yBase, uint16_t color, uint8_t alfa,
                   bool degradado) {
  const int w = anchoTexto(f, s), x = cx - w / 2;
  dibujaTexto(f, s, x, yBase, color, alfa, degradado ? x : 0, degradado ? x + w : 0);
}

// Parte un texto en líneas de ancho máximo maxW (respeta '\n'). Devuelve cuántas.
uint8_t partirLineas(const FuenteAA &f, const char *s, int maxW, char lineas[][48], uint8_t maxLineas) {
  uint8_t n = 0;
  char actual[48] = "";
  while (*s && n < maxLineas) {
    const char *ini = s;
    while (*s && *s != ' ' && *s != '\n') s++;
    char palabra[48];
    const size_t len = min((size_t)(s - ini), sizeof(palabra) - 1);
    memcpy(palabra, ini, len);
    palabra[len] = 0;
    char prueba[100];
    snprintf(prueba, sizeof prueba, "%s%s%s", actual, actual[0] ? " " : "", palabra);
    if (actual[0] && anchoTexto(f, prueba) > maxW) {
      strlcpy(lineas[n++], actual, 48);
      strlcpy(actual, palabra, sizeof actual);
    } else {
      strlcpy(actual, prueba, sizeof actual);
    }
    if (*s == '\n') {
      if (n < maxLineas) strlcpy(lineas[n++], actual, 48);
      actual[0] = 0;
      s++;
    } else if (*s == ' ') {
      s++;
    }
  }
  if (actual[0] && n < maxLineas) strlcpy(lineas[n++], actual, 48);
  return n;
}

void copiaUTF8(char *dst, const char *src, size_t n) {
  strlcpy(dst, src, n);
  size_t len = strlen(dst);
  if (len < strlen(src)) {   // si se ha cortado, no dejar un carácter a medias
    while (len && (dst[len - 1] & 0xC0) == 0x80) len--;
    if (len && (dst[len - 1] & 0x80)) len--;
    dst[len] = 0;
  }
}

// ═════════════════════════════════════════════════════════════
//  CARA
// ═════════════════════════════════════════════════════════════

bool esEspecial(uint8_t a) {
  return a == A_ENAMORADO || a == A_EMOCIONADO || a == A_MAREADO || a == A_DORMIDO;
}

void iniciaParpadeo(uint16_t dur) {
  parpActivo = true;
  parpT0 = t;
  parpDur = dur;
}

void aplicaAnimo(uint8_t a, bool salto) {
  animo = a;
  animoPendiente = 0xFF;
  for (int i = 0; i < 2; i++) {
    ojoObj[i] = FORMA[a][i];
    if (salto) ojoAct[i] = FORMA[a][i];
  }
}

// Las formas normales se transforman suavemente; las especiales
// (corazones, espirales…) cambian a mitad de un parpadeo.
void ponAnimo(uint8_t a) {
  if (a >= N_ANIMOS) return;
  finAnimoTemporal = 0;
  if (a == animo && animoPendiente == 0xFF) return;
  if (esEspecial(a) || esEspecial(animo)) {
    animoPendiente = a;
    iniciaParpadeo(280);
  } else {
    aplicaAnimo(a, false);
  }
}

void animoTemporal(uint8_t a, uint32_t ms, uint8_t despues) {
  ponAnimo(a);
  finAnimoTemporal = t + ms;
  animoTrasTemporal = despues;
}

void iniciaAccion(uint8_t a) {
  accion = a;
  accionT0 = t;
}

void diSubtitulo(const char *s, uint32_t ms) {
  strlcpy(subtitulo, s, sizeof subtitulo);
  subT0 = t;
  subHasta = t + ms;
}

float calculaApertura() {
  if (!parpActivo) return 1.0f;
  const uint32_t e = t - parpT0;
  const float f = e >= parpDur ? 1.0f : (float)e / parpDur;
  if (f >= 0.4f && animoPendiente != 0xFF) aplicaAnimo(animoPendiente, true);
  if (e >= parpDur) { parpActivo = false; return 1.0f; }
  if (f < 0.4f) return 1.0f - f / 0.4f;
  if (f < 0.55f) return 0.0f;
  return (f - 0.55f) / 0.45f;
}

void calculaAccion() {
  acDX = acDY = 0;
  guino = 1;
  if (accion == AC_NADA) return;
  const uint32_t e = t - accionT0;
  switch (accion) {
    case AC_GUINO:
      if (e >= 560) break;
      {
        const float f = e / 560.0f;
        guino = f < 0.3f ? 1 - f / 0.3f : f < 0.65f ? 0 : (f - 0.65f) / 0.35f;
      }
      return;
    case AC_SI:
      if (e >= 1000) break;
      acDY = sinf(e * DOS_PI * 2 / 1000.0f) * 14;
      return;
    case AC_NO:
      if (e >= 1000) break;
      acDX = sinf(e * DOS_PI * 2.5f / 1000.0f) * 26 * (1 - e / 1000.0f);
      return;
    case AC_SALTO:
      if (e >= 760) break;
      acDY = -fabsf(sinf(e * DOS_PI / 760.0f)) * 18;
      return;
    case AC_MIRAR:
      if (e >= 2200) break;
      if (e < 750)       { mirObjX = -MIRADA_X; mirObjY = -8; }
      else if (e < 1500) { mirObjX = MIRADA_X;  mirObjY = -8; }
      else               { mirObjX = 0;   mirObjY = 0; }
      return;
  }
  accion = AC_NADA;
}

// Parpadeos, miradas y micro-expresiones espontáneas
void vidaAutonoma(bool manual, bool micro) {
  if (animo != A_DORMIDO && t >= proxParpadeo) {
    iniciaParpadeo(animo == A_CANSADO ? 460 : 190);
    proxParpadeo = t + (random(7) == 0 ? 330 : random(1800, 5200));   // a veces, doble parpadeo
  }
  if (!manual && accion != AC_MIRAR && animo != A_VIGILANTE && animo != A_DORMIDO && t >= proxMirada) {
    if (random(10) < 4) { mirObjX = 0; mirObjY = 0; }
    else { mirObjX = random(-MIRADA_X, MIRADA_X + 1); mirObjY = random(-MIRADA_Y + 2, MIRADA_Y - 1); }
    proxMirada = t + random(1200, 4200);
  }
  if (micro && animo == A_NORMAL && !finAnimoTemporal && accion == AC_NADA && !dormirPendiente &&
      t >= proxMicro) {
    proxMicro = t + random(18000, 40000);
    switch (random(6)) {
      case 0:  animoTemporal(A_FELIZ, 2200, A_NORMAL); break;
      case 1:  animoTemporal(A_SORPRENDIDO, 1300, A_NORMAL); break;
      case 2:  animoTemporal(A_SOSPECHOSO, 2400, A_NORMAL); break;
      case 3:  iniciaAccion(AC_MIRAR); break;
      case 4:  iniciaAccion(AC_SALTO); break;
      default: iniciaAccion(AC_GUINO); break;
    }
  }
}

// Expresiones encadenadas durante el modo presentación
void carrusel() {
  const uint32_t e = t - escenaT0;
  if (escenaArg == 1) {
    if (e > 3800 && !saludoMirado) { saludoMirado = true; iniciaAccion(AC_MIRAR); }
    return;
  }
  static const uint8_t SECUENCIA[] = { A_ENAMORADO, A_SORPRENDIDO, A_EMOCIONADO, A_FELIZ,
                                       A_VIGILANTE, A_MAREADO, A_NORMAL };
  if (t >= proxCarrusel && carruselIdx < sizeof(SECUENCIA)) {
    const uint8_t a = SECUENCIA[carruselIdx++];
    ponAnimo(a);
    if (a == A_FELIZ) iniciaAccion(AC_GUINO);
    proxCarrusel = t + 1300;
  }
}

void actualizaCara(uint32_t dt) {
  const float k = min(1.0f, dt * 0.012f);
  for (int i = 0; i < 2; i++) {
    Ojo &a = ojoAct[i];
    const Ojo &o = ojoObj[i];
    a.w += (o.w - a.w) * k;
    a.h += (o.h - a.h) * k;
    a.r += (o.r - a.r) * k;
    a.dy += (o.dy - a.dy) * k;
    a.incl += (o.incl - a.incl) * k;
    a.parpSup += (o.parpSup - a.parpSup) * k;
    a.parpInf += (o.parpInf - a.parpInf) * k;
  }
  if (finAnimoTemporal && t >= finAnimoTemporal) {
    finAnimoTemporal = 0;
    ponAnimo(animoTrasTemporal);
  }
  const bool manual = t < mirarManualHasta;
  if (!manual && animo == A_VIGILANTE) {   // barrido de cámara de seguridad
    mirObjX = sinf(t * DOS_PI / 4200.0f) * MIRADA_X;
    mirObjY = 4;
  }
  if (escena == ESC_CARA) {
    if (presentacion || aj.autonomo) vidaAutonoma(manual, !presentacion);
    if (presentacion) carrusel();
  }
  calculaAccion();
  const float km = min(1.0f, dt * 0.018f);
  mirX += (mirObjX - mirX) * km;
  mirY += (mirObjY - mirY) * km;
  aperturaFrame = calculaApertura();
}

void ojoParametrico(int lado, float cx, float cy, const Ojo &o, float ap, float escH, uint16_t ck, uint16_t bg) {
  const int wi = lroundf(o.w);
  int hi = lroundf(o.h * escH * ap);
  if (hi < 8) hi = 8;
  const int x = lroundf(cx - o.w / 2);
  const int y = lroundf(cy + o.dy - hi / 2.0f);
  const int r = min((int)lroundf(o.r), min(wi, hi) / 2);
  g->fillRoundRect(x, y, wi, hi, r, ck);
  rectOjo[lado] = { x, y, wi, hi };

  // Párpado inclinado: el lado interior es el que mira hacia el centro de la cara
  const int incl = lroundf(o.incl * min(1.0f, hi / 40.0f));
  if (incl) {
    const int xin = lado == 0 ? x + wi : x;
    const int xout = lado == 0 ? x - 1 : x + wi + 1;
    if (incl > 0) g->fillTriangle(xout, y - 1, xin, y - 1, xin, y + incl, bg);
    else          g->fillTriangle(xout, y - 1, xin, y - 1, xout, y - incl, bg);
  }
  if (o.parpSup > 0.01f) g->fillRect(x - 1, y - 1, wi + 2, lroundf(hi * o.parpSup) + 1, bg);
  if (o.parpInf > 0.01f) {   // ojo sonriente: un círculo grande "muerde" la parte de abajo
    const int corte = lroundf(hi * o.parpInf);
    const int R = lroundf(o.w * 0.85f);
    g->fillCircle(lroundf(cx), y + hi - corte + R, R, bg);
  }
}

void dibujaCorazon(float cx, float cy, float s, uint16_t c) {
  const int r = s * 0.27f;
  g->fillCircle(cx - s * 0.23f, cy - s * 0.12f, r, c);
  g->fillCircle(cx + s * 0.23f, cy - s * 0.12f, r, c);
  g->fillTriangle(cx - s * 0.49f, cy - s * 0.02f, cx + s * 0.49f, cy - s * 0.02f, cx, cy + s * 0.5f, c);
}

void espiral(float cx, float cy, float giro, uint16_t c) {
  float px = cx, py = cy;
  for (int k = 1; k <= 34; k++) {
    const float f = k / 34.0f, a = giro + f * 2.6f * DOS_PI, rad = 2 + f * 33;
    const float nx = cx + cosf(a) * rad, ny = cy + sinf(a) * rad;
    lineaGruesa(px, py, nx, ny, 6, c);
    px = nx;
    py = ny;
  }
}

// Tiñe los píxeles "llave" con el degradado vertical de los ojos
void tintaOjos(int gTop, int gBot) {
  const int h = max(1, gBot - gTop);
  for (int y = 0; y < ALTO; y++) {
    const uint16_t c = GRAD_OJO[constrain((y - gTop) * 63 / h, 0, 63)];
    uint16_t *p = ram.fila(y);
    for (int x = 0; x < ANCHO; x++)
      if (p[x] == C_CLAVE) p[x] = c;
  }
}

void pintaSubtitulo() {
  if (!subHasta || t >= subHasta) return;
  const uint8_t a = min(alfaRampa(t - subT0, 200), alfaRampa(subHasta - t, 300));
  textoCentrado(F_MED, subtitulo, 120, 222, C_GRIS, a, false);
}

void renderCara() {
  const uint16_t bg = colorFondo();
  fondoSinFB = bg;
  g->fillScreen(bg);
  const uint16_t ck = hayFB ? C_CLAVE : GRAD_OJO[20];
  const bool durmiendo = animo == A_DORMIDO;
  const float resp = sinf(t * DOS_PI / (durmiendo ? 5200.0f : 3800.0f)) * (durmiendo ? 3.5f : 1.6f);
  float ox = mirX + acDX, oy = mirY + acDY + resp;
  if (animo == A_EMOCIONADO) oy -= fabsf(sinf(t * DOS_PI / 520.0f)) * 10;
  if (animo == A_MAREADO) { ox += cosf(t / 260.0f) * 5; oy += sinf(t / 260.0f) * 4; }
  const float cy = CARA_CY + oy;
  const float cx[2] = { 120 - CARA_SEP + ox, 120 + CARA_SEP + ox };
  const float ap = aperturaFrame;
  const bool cerrado = esEspecial(animo) && !durmiendo && ap < 0.35f;

  if (cerrado) {
    for (int i = 0; i < 2; i++) g->fillRoundRect(cx[i] - 33, cy - 4, 66, 9, 4, ck);
  } else {
    switch (animo) {
      case A_ENAMORADO: {
        const float latido = powf(fabsf(sinf(t * DOS_PI / 900.0f)), 3);
        const float s = 78 * (1 + 0.10f * latido);
        for (int i = 0; i < 2; i++) dibujaCorazon(cx[i], cy, s, ck);
        break;
      }
      case A_EMOCIONADO:   // > <
        for (int i = 0; i < 2; i++) {
          const int sg = i == 0 ? 1 : -1;
          lineaGruesa(cx[i] - sg * 21, cy - 31, cx[i] + sg * 21, cy, 17, ck);
          lineaGruesa(cx[i] + sg * 21, cy, cx[i] - sg * 21, cy + 31, 17, ck);
        }
        break;
      case A_MAREADO:
        for (int i = 0; i < 2; i++) espiral(cx[i], cy, (i == 0 ? 1 : -1) * (t / 160.0f), ck);
        break;
      case A_DORMIDO:      // ojos cerrados en forma de U
        for (int i = 0; i < 2; i++) {
          float px = cx[i] + 32, py = cy + 4;
          for (int k = 1; k <= 12; k++) {
            const float a = k * PI / 12;
            const float nx = cx[i] + cosf(a) * 32, ny = cy + 4 + sinf(a) * 12;
            lineaGruesa(px, py, nx, ny, 9, ck);
            px = nx;
            py = ny;
          }
        }
        break;
      default: {
        // Perspectiva: el ojo hacia el que mira crece un poco
        const float persp = constrain(mirX / MIRADA_X, -1.0f, 1.0f) * 0.07f;
        ojoParametrico(0, cx[0], cy, ojoAct[0], ap, 1 - persp, ck, bg);
        ojoParametrico(1, cx[1], cy, ojoAct[1], ap * guino, 1 + persp, ck, bg);
      }
    }
  }
  if (hayFB) tintaOjos(cy - 54, cy + 54);

  if (!cerrado) {
    if (!esEspecial(animo)) {   // brillo en los ojos
      for (int i = 0; i < 2; i++) {
        const Rect &r = rectOjo[i];
        if (r.h < 44 || ojoAct[i].parpSup > 0.3f || fabsf(ojoAct[i].incl) > 4) continue;
        g->fillCircle(r.x + r.w * 0.30f, r.y + r.h * 0.26f, max(3, r.w / 10), C_BRILLO);
      }
    }
    if (animo == A_TRISTE) {   // lágrima
      const uint32_t ciclo = t % 2600;
      if (ciclo < 2000) {
        const Rect &r = rectOjo[0];
        const float f = ciclo / 2000.0f;
        const int x = r.x + r.w / 4, y = r.y + r.h - 6 + f * 50, rad = 5;
        g->fillTriangle(x - rad, y, x + rad, y, x, y - rad * 2.6f, C_LAGRIMA);
        g->fillCircle(x, y, rad, C_LAGRIMA);
      }
    }
    if (durmiendo) {           // Zzz
      for (int k = 0; k < 3; k++) {
        const float p = fmodf(t / 3000.0f + k / 3.0f, 1.0f);
        const uint8_t a = sinf(p * PI) * 255;
        dibujaTexto(k % 2 ? F_PEQ : F_MED, k % 2 ? "z" : "Z", 178 + p * 36, 76 - p * 52, C_VERDE, a, 0, 0);
      }
    }
  }
  pintaSubtitulo();
}

// ═════════════════════════════════════════════════════════════
//  ESCENAS
// ═════════════════════════════════════════════════════════════

void iniciaRadar() {
  nBlips = 5;
  for (int i = 0; i < nBlips; i++) {
    const float a = random(0, 628) / 100.0f;
    const int r = random(22, 84);
    blips[i] = { (int16_t)(120 + cosf(a) * r), (int16_t)(104 + sinf(a) * r), a };
  }
}

void cambiaEscena(uint8_t esc, uint32_t dur, uint8_t arg) {
  escena = esc;
  escenaT0 = t;
  escenaDur = dur;
  escenaArg = arg;
  subHasta = 0;
  if (esc == ESC_RADAR) iniciaRadar();
  if (esc == ESC_ATAQUE) ataqueFase = 0;
  if (esc == ESC_CARA) iniciaParpadeo(320);
}

void aCara() { cambiaEscena(ESC_CARA, 0, 0); }

void aplicaPaso() {
  const Paso &p = PRESENTACION[presPaso];
  cambiaEscena(p.escena, p.dur, p.arg);
  if (p.escena == ESC_CARA) {
    if (p.arg == 1) {
      ponAnimo(A_FELIZ);
      diSubtitulo("¡Hola! Soy OrnitoSIRT", 3600);
      saludoMirado = false;
    } else {
      carruselIdx = 0;
      proxCarrusel = t;
    }
  }
}

void iniciaPresentacion() {
  presentacion = true;
  presPaso = 0;
  aplicaPaso();
}

void detienePresentacion() { presentacion = false; }

void compruebaFinEscena() {
  if (!escenaDur || t - escenaT0 < escenaDur) return;
  if (presentacion) {
    presPaso = (presPaso + 1) % N_PASOS;
    aplicaPaso();
    return;
  }
  const uint8_t anterior = escena;
  aCara();
  if (anterior == ESC_ARRANQUE) {
    if (aj.presAlArrancar) iniciaPresentacion();
    else { cambiaEscena(ESC_WIFI, 0, 0); wifiArranque = true; }
  } else if (anterior == ESC_RADAR) {
    animoTemporal(A_FELIZ, 2600, A_NORMAL);
    diSubtitulo("¡Red segura!", 2600);
  } else if (anterior == ESC_ATAQUE) {
    animoTemporal(A_FELIZ, 1500, A_NORMAL);
  }
}

// ── Arranque: el logo se "escanea" de izquierda a derecha ────
void renderArranque(uint32_t e) {
  g->fillScreen(C_MARINO);
  fondoSinFB = C_MARINO;
  const ImagenAA &L = LOGO_GRANDE;
  const int lx = (ANCHO - L.w) / 2, ly = 66;
  if (e > 200) {
    const int rev = e >= 1200 ? L.w : (e - 200) * L.w / 1000;
    dibujaImagen(L, lx, ly, 0, 255, true, rev, 9999);
    if (rev < L.w) lineaEscaner(lx + rev, ly - 10, L.h + 20);
  }
  if (e > 1300) textoCentrado(F_MED, "OrnitoSIRT", 120, 168, C_GRIS, alfaRampa(e - 1300, 600), false);
  if (e > 1700) {
    const int w = e >= 2200 ? 140 : (e - 1700) * 140 / 500;
    lineaDegradado(120 - w / 2, 182, w, 2, true);
  }
  if (e > 2200) textoCentrado(F_PEQ, "v" VERSION_FIRMWARE, 120, 214, C_VERDE, alfaRampa(e - 2200, 500), false);
}

// ── QR para conectarse al WiFi ───────────────────────────────
bool qrModulo(int x, int y) {
  const int i = y * qrTam + x;
  return qrBits[i >> 3] & (1 << (i & 7));
}

void renderWifi(uint32_t e) {
  g->fillScreen(C_GRIS);
  fondoSinFB = C_GRIS;
  textoCentrado(F_MED, presentacion ? "¡Contrólame!" : "Escanéame", 120, 27, C_MARINO, 255, false);
  char l[64];
  if (qrTam) {
    const int m = min(6, 158 / qrTam), lado = qrTam * m;
    const int x0 = (ANCHO - lado) / 2, y0 = 36 + (158 - lado) / 2;
    for (int y = 0; y < qrTam; y++)
      for (int x = 0; x < qrTam; x++)
        if (qrModulo(x, y)) g->fillRect(x0 + x * m, y0 + y * m, m, m, C_MARINO);
  } else {
    textoCentrado(F_MED, "Conéctate al WiFi", 120, 100, C_MARINO, 255, false);
    textoCentrado(F_MED, "y abre el navegador", 120, 130, C_MARINO, 255, false);
  }
  snprintf(l, sizeof l, "WiFi  %s", nombreWifi);
  textoCentrado(F_PEQ, l, 120, 214, C_MARINO, 255, false);
  if ((e / 2600) % 2 == 0) snprintf(l, sizeof l, "Clave  %s", WIFI_CLAVE);
  else snprintf(l, sizeof l, "Panel  %s", IP_AP.toString().c_str());
  textoCentrado(F_PEQ, l, 120, 233, C_PETROLEO, 255, false);
}

// ── Logo + eslogan ───────────────────────────────────────────
void renderLogo(uint32_t e) {
  g->fillScreen(C_MARINO);
  fondoSinFB = C_MARINO;
  const ImagenAA &L = LOGO_GRANDE;
  const int lx = (ANCHO - L.w) / 2, ly = 30;
  const int rev = e >= 700 ? L.w : e * L.w / 700;
  dibujaImagen(L, lx, ly, 0, 255, true, rev, 9999);
  if (rev < L.w) lineaEscaner(lx + rev, ly - 8, L.h + 16);
  static const char *const ESLOGAN[] = { "Expertos en servicios", "tecnológicos", "avanzados" };
  for (int i = 0; i < 3; i++) {
    const uint32_t ini = 750 + i * 300;
    if (e > ini) textoCentrado(F_FINA, ESLOGAN[i], 120, 134 + i * 29, C_GRIS, alfaRampa(e - ini, 400), false);
  }
  if (e > 1800) textoCentrado(F_PEQ, "sirt.com", 120, 226, C_VERDE, alfaRampa(e - 1800, 400), false);
  barraProgreso(e);
}

// ── Servicios: ciberseguridad, infraestructura, cloud ───────
void renderServicio(uint32_t e) {
  static const ImagenAA *const ICONOS[] = { &ICONO_ESCUDO, &ICONO_SERVIDOR, &ICONO_NUBE };
  static const char *const TITULOS[] = { "Ciberseguridad", "Infraestructura IT", "Cloud" };
  static const char *const DETALLES[] = {
    "SOC 24x7 · Pentesting\nENS · ISO 27001 · NIS2",
    "Redes · Datacenter\nBackup y recuperación",
    "Migración · Arquitecturas\nseguras · Optimización",
  };
  const uint8_t i = escenaArg % 3;
  g->fillScreen(C_MARINO);
  fondoSinFB = C_MARINO;
  g->fillCircle(120, 72, 62, mezcla565(C_PETROLEO, C_MARINO, 60));
  const ImagenAA &ic = *ICONOS[i];
  const int caida = e >= 450 ? 0 : (450 - e) * 18 / 450;
  dibujaImagen(ic, 120 - ic.w / 2, 24 - caida, 0, alfaRampa(e, 450), true, 9999, 9999);
  if (e > 300) textoCentrado(F_MED, TITULOS[i], 120, 156, C_GRIS, alfaRampa(e - 300, 400), false);
  char lineas[3][48];
  const uint8_t n = partirLineas(F_PEQ, DETALLES[i], 224, lineas, 3);
  for (int k = 0; k < n; k++) {
    const uint32_t ini = 650 + k * 150;
    if (e > ini) textoCentrado(F_PEQ, lineas[k], 120, 186 + k * 21, C_VERDE, alfaRampa(e - ini, 400), false);
  }
  barraProgreso(e);
}

// ── Radar del SOC ────────────────────────────────────────────
void anillosRadar(int cx, int cy, int R) {
  const uint16_t c = C_PETROLEO;
  g->drawCircle(cx, cy, R, c);
  g->drawCircle(cx, cy, R * 2 / 3, c);
  g->drawCircle(cx, cy, R / 3, c);
  g->drawFastHLine(cx - R, cy, 2 * R, c);
  g->drawFastVLine(cx, cy - R, 2 * R, c);
}

void renderRadar(uint32_t e) {
  const int cx = 120, cy = 104, R = 88;
  const uint32_t FIN = 5200;
  g->fillScreen(C_MARINO);
  fondoSinFB = C_MARINO;
  if (e < FIN) {
    const float ang = e * DOS_PI / 1800.0f;
    for (int k = 0; k < 12; k++) {   // estela del barrido
      const float a0 = ang - k * 0.07f, a1 = a0 - 0.075f;
      g->fillTriangle(cx, cy, cx + cosf(a0) * R, cy + sinf(a0) * R, cx + cosf(a1) * R, cy + sinf(a1) * R,
                      mezcla565(C_VERDE, C_MARINO, 170 - k * 14));
    }
    anillosRadar(cx, cy, R);
    lineaGruesa(cx, cy, cx + cosf(ang) * R, cy + sinf(ang) * R, 3, C_LIMA);
    for (int i = 0; i < nBlips; i++) {   // los puntos se encienden al pasar el barrido
      float d = fmodf(ang - blips[i].ang, DOS_PI);
      if (d < 0) d += DOS_PI;
      if (d < 3.6f) g->fillCircle(blips[i].x, blips[i].y, 4, mezcla565(C_LIMA, C_MARINO, 255 - d * 255 / 3.6f));
    }
    char txt[40];
    snprintf(txt, sizeof txt, "Escaneando red… %u%%", (unsigned)(e * 100 / FIN));
    textoCentrado(F_PEQ, txt, 120, 228, C_GRIS, 255, false);
  } else {
    const uint32_t k = e - FIN;
    anillosRadar(cx, cy, R);
    for (int i = 0; i < nBlips; i++) g->fillCircle(blips[i].x, blips[i].y, 4, C_VERDE);
    g->fillCircle(cx, cy, k >= 300 ? 34 : k * 34 / 300, C_VERDE);
    if (k > 250) {
      lineaGruesa(cx - 14, cy + 1, cx - 4, cy + 11, 6, C_MARINO);
      lineaGruesa(cx - 4, cy + 11, cx + 15, cy - 10, 6, C_MARINO);
    }
    textoCentrado(F_MED, "Red segura", 120, 216, C_LIMA, alfaRampa(k, 400), false);
    textoCentrado(F_PEQ, "0 amenazas detectadas", 120, 236, C_GRIS, alfaRampa(k, 400), false);
  }
}

// ── Ataque simulado: sorpresa → alerta → escudo → ¡protegido! ─
void glitch(int n) {
  for (int k = 0; k < n; k++) {
    const int y = random(ALTO), h = random(2, 10), dx = random(-12, 13);
    for (int yy = y; yy < y + h && yy < ALTO; yy++) {
      uint16_t *fila = ram.fila(yy);
      if (dx > 0) memmove(fila + dx, fila, (ANCHO - dx) * 2);
      else if (dx < 0) memmove(fila, fila - dx, (ANCHO + dx) * 2);
    }
  }
  for (int k = 0; k < n / 2; k++)
    g->fillRect(random(ANCHO), random(ALTO), random(20, 90), random(1, 4), random(2) ? C_LIMA : C_VERDE);
}

void renderAtaque(uint32_t e) {
  if (e < 1000) {
    if (ataqueFase == 0) {
      ataqueFase = 1;
      ponAnimo(A_SORPRENDIDO);
      mirObjX = 0;
      mirObjY = -6;
    }
    renderCara();
    if ((e / 160) % 2) textoCentrado(F_MED, "¡¿?!", 120, 222, C_ALERTA, 255, false);
  } else if (e < 2800) {
    const uint16_t bg = (e / 110) % 2 ? mezcla565(C_ALERTA, C_MARINO, 70) : C_MARINO;
    g->fillScreen(bg);
    fondoSinFB = bg;
    g->fillTriangle(120, 34, 76, 110, 164, 110, C_ALERTA);
    lineaGruesa(120, 60, 120, 86, 9, bg);
    g->fillCircle(120, 99, 5, bg);
    textoCentrado(F_GRANDE, "¡ALERTA!", 120, 166, C_ALERTA, 255, false);
    textoCentrado(F_PEQ, "Intrusión detectada", 120, 196, C_GRIS, 255, false);
    if (hayFB && e < 2200) glitch((2200 - e) / 200 + random(3));
  } else if (e < 4800) {
    const uint32_t k = e - 2800;
    g->fillScreen(C_MARINO);
    fondoSinFB = C_MARINO;
    const ImagenAA &E = ICONO_ESCUDO_G;
    const int ey = 26, ecy = ey + E.h / 2;
    for (int i = 0; i < 3; i++) {   // ondas de protección
      const uint32_t ph = (k + i * 330) % 1000;
      const int r = 50 + ph * 70 / 1000;
      const uint16_t c = mezcla565(C_VERDE, C_MARINO, 180 - ph * 180 / 1000);
      g->drawCircle(120, ecy, r, c);
      g->drawCircle(120, ecy, r + 1, c);
    }
    dibujaImagen(E, 120 - E.w / 2, ey, 0, 255, true, 9999, k >= 450 ? E.h : k * E.h / 450);
    if (k > 500) textoCentrado(F_MED, "Amenaza bloqueada", 120, 184, C_LIMA, alfaRampa(k - 500, 300), false);
    if (k > 800) textoCentrado(F_PEQ, "SOC SIRT 24x7", 120, 210, C_VERDE, alfaRampa(k - 800, 300), false);
  } else {
    if (ataqueFase < 2) {
      ataqueFase = 2;
      ponAnimo(A_FELIZ);
      iniciaAccion(AC_SALTO);
      diSubtitulo("¡Protegido!", 2400);
    }
    renderCara();
  }
}

// ── Mensaje en marquesina ────────────────────────────────────
void renderTexto(uint32_t e) {
  const uint16_t bg = colorFondo();
  g->fillScreen(bg);
  fondoSinFB = bg;
  dibujaImagen(LOGO_PEQ, (ANCHO - LOGO_PEQ.w) / 2, 28, 0, 255, true, 9999, 9999);
  lineaDegradado(60, 194, 120, 2, true);
  const int base = 120 + F_GRANDE.ascenso / 2 - 2;
  const uint16_t COLORES[] = { C_LIMA, C_LIMA, C_VERDE, C_GRIS };
  const uint16_t color = COLORES[mensajeColor % 4];
  const bool grad = mensajeColor == 0;
  if (mensajeAncho <= ANCHO - 16) {
    const int x = (ANCHO - mensajeAncho) / 2;
    dibujaTexto(F_GRANDE, mensaje, x, base, color, 255, grad ? x : 0, grad ? x + mensajeAncho : 0);
  } else {
    const int ciclo = mensajeAncho + ANCHO;
    const int x = ANCHO - (int)((e * 80 / 1000) % ciclo);
    dibujaTexto(F_GRANDE, mensaje, x, base, color, 255, 0, grad ? ANCHO : 0);
  }
}

// ═════════════════════════════════════════════════════════════
//  TERMINAL
// ═════════════════════════════════════════════════════════════

// La fuente clásica de Adafruit GFX usa la tabla CP437, que tiene las letras españolas
uint8_t aCP437(uint32_t cp) {
  if (cp >= 32 && cp < 127) return cp;
  switch (cp) {
    case 0xE1: return 0xA0;  case 0xE9: return 0x82;  case 0xED: return 0xA1;   // á é í
    case 0xF3: return 0xA2;  case 0xFA: return 0xA3;  case 0xF1: return 0xA4;   // ó ú ñ
    case 0xD1: return 0xA5;  case 0xFC: return 0x81;  case 0xDC: return 0x9A;   // Ñ ü Ü
    case 0xC9: return 0x90;  case 0xBF: return 0xA8;  case 0xA1: return 0xAD;   // É ¿ ¡
    case 0xBA: return 0xA7;  case 0xAA: return 0xA6;  case 0xE7: return 0x87;   // º ª ç
    case 0xC7: return 0x80;  case 0xC1: return 'A';   case 0xCD: return 'I';    // Ç Á Í
    case 0xD3: return 'O';   case 0xDA: return 'U';                              // Ó Ú
  }
  return 0;
}

void utf8aCP437(char *dst, const char *src, size_t n) {
  size_t k = 0;
  while (*src && k < n - 1) {
    const uint8_t c = aCP437(leeUTF8(src));
    if (c) dst[k++] = c;
  }
  dst[k] = 0;
}

void agregaLinea(const char *cp437, uint8_t tipo) {
  if (termN == TERM_FILAS) {
    memmove(term, term + 1, sizeof(LineaTerm) * (TERM_FILAS - 1));
    termN--;
  }
  strlcpy(term[termN].txt, cp437, TERM_COLS + 1);
  term[termN].tipo = tipo;
  termN++;
}

void encola(const char *utf8, uint8_t tipo, uint16_t retardo) {
  if (colaN == COLA_TERM) return;
  SalidaTerm &s = colaT[(colaIni + colaN) % COLA_TERM];
  utf8aCP437(s.txt, utf8, sizeof s.txt);
  s.tipo = tipo;
  s.retardo = retardo;
  if (!colaN) proxSalida = millis() + retardo;
  colaN++;
}

void actualizaTerminal(uint32_t ahora) {
  while (colaN && ahora >= proxSalida) {
    const SalidaTerm &s = colaT[colaIni];
    agregaLinea(s.txt, s.tipo);
    colaIni = (colaIni + 1) % COLA_TERM;
    colaN--;
    if (colaN) proxSalida = ahora + colaT[colaIni].retardo;
  }
}

void abreTerminal() {
  termN = 0;
  entradaLen = 0;
  entrada[0] = 0;
  colaN = 0;
  cambiaEscena(ESC_TERMINAL, 0, 0);
  encola("OrnitoSIRT v" VERSION_FIRMWARE, 2, 0);
  encola("Escribe 'ayuda'", 1, 250);
}

void ejecutaEntrada() {
  char vis[TERM_COLS - 1];
  strlcpy(vis, entrada, sizeof vis);
  agregaLinea(vis, 0);
  char cmd[48];
  size_t n = 0;
  const char *p = entrada;
  while (*p == ' ') p++;
  while (*p && n < sizeof(cmd) - 1) cmd[n++] = tolower((uint8_t)*p++);
  while (n && cmd[n - 1] == ' ') n--;
  cmd[n] = 0;
  entradaLen = 0;
  entrada[0] = 0;
  if (!n) return;
  auto es = [&](const char *a) { return strcmp(cmd, a) == 0; };

  if (es("ayuda") || es("help")) {
    encola("Órdenes:", 2, 60);
    encola(" hola  quien  sirt", 1, 60);
    encola(" escanear  soc  ip", 1, 60);
    encola(" cara  limpiar", 1, 60);
  } else if (es("hola")) {
    encola("¡Hola! Soy Ornito", 2, 150);
  } else if (es("quien") || es("whoami")) {
    encola("ornito@sirt", 2, 100);
    encola("Mascota del SOC", 1, 120);
  } else if (es("sirt")) {
    encola("Ciberseguridad", 2, 120);
    encola("Infraestructura IT", 2, 120);
    encola("Cloud", 2, 120);
    encola("www.sirt.com", 1, 200);
  } else if (es("escanear") || es("scan") || es("nmap")) {
    encola("Escaneando red...", 1, 100);
    encola("[###.......]  30%", 1, 450);
    encola("[#######...]  70%", 1, 450);
    encola("[##########] 100%", 1, 450);
    encola("Equipos: 3", 1, 250);
    encola("0 amenazas. OK", 2, 300);
  } else if (es("soc")) {
    encola("SOC SIRT 24x7", 2, 100);
    encola("Estado: VIGILANDO", 1, 150);
    encola("Incidentes: 0", 1, 150);
  } else if (es("ip")) {
    char l[32];
    snprintf(l, sizeof l, "IP %s", IP_AP.toString().c_str());
    encola(l, 1, 80);
    snprintf(l, sizeof l, "Conectados: %u", (unsigned)WiFi.softAPgetStationNum());
    encola(l, 1, 80);
  } else if (es("cara") || es("salir") || es("exit")) {
    aCara();
  } else if (es("limpiar") || es("clear") || es("cls")) {
    termN = 0;
  } else if (strncmp(cmd, "sudo", 4) == 0) {
    encola("Buen intento ;)", 3, 150);
  } else if (strstr(cmd, "rm -rf")) {
    encola("Bloqueado por", 3, 150);
    encola("el SOC de SIRT", 3, 100);
  } else {
    encola("Orden desconocida", 3, 80);
    encola("Prueba 'ayuda'", 1, 80);
  }
}

void terminalTeclas(int borrar, const char *utf8) {
  for (; borrar > 0 && entradaLen; borrar--) entrada[--entradaLen] = 0;
  while (*utf8) {
    const uint8_t c = aCP437(leeUTF8(utf8));
    if (c && entradaLen < sizeof(entrada) - 1) {
      entrada[entradaLen++] = c;
      entrada[entradaLen] = 0;
    }
  }
}

void pintaLineaTerm(const char *txt, uint8_t tipo, int y) {
  int x = 6;
  if (tipo == 0) {
    g->setTextColor(C_VERDE);
    g->setCursor(x, y);
    g->print("$ ");
    x += 24;
  }
  const uint16_t COLORES[] = { C_GRIS, mezcla565(C_GRIS, C_NOCHE, 170), C_LIMA, C_ALERTA };
  g->setTextColor(COLORES[tipo & 3]);
  g->setCursor(x, y);
  g->print(txt);
}

void renderTerminal() {
  g->fillScreen(C_NOCHE);
  fondoSinFB = C_NOCHE;
  g->fillRect(0, 0, ANCHO, 22, mezcla565(C_PETROLEO, C_NOCHE, 90));
  g->fillCircle(12, 11, 4, C_ALERTA);
  g->fillCircle(25, 11, 4, C_LIMA);
  g->fillCircle(38, 11, 4, C_VERDE);
  dibujaTexto(F_PEQ, "ornito@sirt: ~", 52, 16, C_GRIS, 255, 0, 0);
  lineaDegradado(0, 22, ANCHO, 2, false);
  g->setTextSize(2);
  g->setTextWrap(false);
  int y = 30;
  for (int i = 0; i < termN; i++, y += 18) pintaLineaTerm(term[i].txt, term[i].tipo, y);
  if (!colaN) {   // línea de entrada con cursor parpadeante
    const char *vis = entrada + (entradaLen > TERM_COLS - 3 ? entradaLen - (TERM_COLS - 3) : 0);
    pintaLineaTerm(vis, 0, y);
    if ((millis() / 500) % 2 == 0) g->fillRect(30 + strlen(vis) * 12, y, 10, 16, C_VERDE);
  }
}

// ═════════════════════════════════════════════════════════════
//  AJUSTES, BRILLO Y SUEÑO
// ═════════════════════════════════════════════════════════════

void escribeLuz(uint32_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PIN_TFT_BL, duty);
#else
  ledcWrite(0, duty);
#endif
}

void aplicaBrillo() {
  uint32_t b = aj.brillo;
  if (dormido) b = b / 4 > 4 ? b / 4 : 4;
  uint32_t duty = aj.luz ? (b * b * 255 + 5000) / 10000 : 0;   // curva cuadrática: más natural
  if (aj.luz && duty < 2) duty = 2;
  escribeLuz(duty);
}

void cargaAjustes() {
  prefs.begin("ornito", false);
  if (prefs.getBytesLength("aj") == sizeof(Ajustes)) {
    Ajustes leido;
    prefs.getBytes("aj", &leido, sizeof leido);
    if (leido.version == VERSION_AJUSTES) aj = leido;
  }
  aj.brillo = constrain(aj.brillo, 5, 100);
  aj.velocidad = constrain(aj.velocidad, 1, 3);
  aj.ojos %= N_TEMAS_OJOS;
  aj.fondo %= N_FONDOS;
  aj.rotacion &= 3;
}

void marcaAjustes() {
  ajustesSucios = true;
  ajustesCambio = millis();
}

void despierta() {
  if (!dormido && !dormirPendiente) return;
  dormido = dormirPendiente = false;
  aplicaBrillo();
  if (escena == ESC_CARA) {
    animoTemporal(A_SORPRENDIDO, 900, A_NORMAL);
    diSubtitulo("¡Ya estoy!", 1800);
  }
}

void interaccion() {
  ultimaInteraccion = millis();
  despierta();
}

void saludaCliente() {
  interaccion();
  aCara();
  animoTemporal(A_FELIZ, 2500, A_NORMAL);
  diSubtitulo("¡Hola!", 2500);
}

void gestionaEventos(uint32_t ahora) {
  if (nuevoCliente && escena != ESC_ARRANQUE) {   // durante el logo se espera a que acabe
    nuevoCliente = false;
    if (escena == ESC_WIFI && !presentacion) {
      wifiArranque = false;
      saludaCliente();
    } else if (escena == ESC_CARA && !presentacion) {
      interaccion();
      iniciaAccion(AC_SALTO);
      diSubtitulo("¡Hola!", 2000);
    }
  }
  // QR del arranque: se quita en cuanto hay alguien conectado (un móvil que ya
  // conocía la red se reconecta solo) o al pasar el tiempo máximo
  if (escena == ESC_WIFI && wifiArranque && !presentacion) {
    if (WiFi.softAPgetStationNum() > 0) {
      wifiArranque = false;
      nuevoCliente = false;
      saludaCliente();
    } else if (ahora > SEGUNDOS_PANTALLA_WIFI * 1000UL) {
      wifiArranque = false;
      aCara();
    }
  }
#if MINUTOS_PARA_DORMIR > 0
  if (aj.autonomo && escena == ESC_CARA && !presentacion && !dormido && !dormirPendiente &&
      ahora - ultimaInteraccion > MINUTOS_PARA_DORMIR * 60000UL) {
    dormirPendiente = true;     // primero bosteza…
    dormirEn = ahora + 4000;
    ponAnimo(A_CANSADO);
  }
  if (dormirPendiente && ahora >= dormirEn) {   // …y luego se duerme
    dormirPendiente = false;
    dormido = true;
    ponAnimo(A_DORMIDO);
    aplicaBrillo();
  }
#endif
  if (ajustesSucios && ahora - ajustesCambio > 1500) {
    ajustesSucios = false;
    prefs.putBytes("aj", &aj, sizeof aj);
  }
}

// ═════════════════════════════════════════════════════════════
//  WIFI Y QR
// ═════════════════════════════════════════════════════════════

#if HAY_QR
void recibeQR(esp_qrcode_handle_t q) {
  const int n = esp_qrcode_get_size(q);
  if (n > 41) return;
  qrTam = n;
  memset(qrBits, 0, sizeof qrBits);
  for (int y = 0; y < n; y++)
    for (int x = 0; x < n; x++)
      if (esp_qrcode_get_module(q, x, y)) {
        const int i = y * n + x;
        qrBits[i >> 3] |= 1 << (i & 7);
      }
}
#endif

// Formato estándar que entienden las cámaras de iPhone y Android
void generaQR() {
#if HAY_QR
  char txt[160];
  size_t k = 0;
  auto pon = [&](const char *s, bool escapar) {
    for (; *s && k < sizeof(txt) - 3; s++) {
      if (escapar && strchr("\\;,:\"", *s)) txt[k++] = '\\';
      txt[k++] = *s;
    }
  };
  pon("WIFI:T:WPA;S:", false);
  pon(nombreWifi, true);
  pon(";P:", false);
  pon(WIFI_CLAVE, true);
  pon(";;", false);
  txt[k] = 0;
  esp_qrcode_config_t cfg = {};
  cfg.display_func = recibeQR;
  cfg.max_qrcode_version = 6;
  cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;
  esp_qrcode_generate(&cfg, txt);
#endif
}

void iniciaWifi() {
  WiFi.mode(WIFI_AP);
  strlcpy(nombreWifi, WIFI_NOMBRE, sizeof nombreWifi);
#if WIFI_SUFIJO_MAC
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  snprintf(nombreWifi, sizeof nombreWifi, "%s-%02X%02X", WIFI_NOMBRE, mac[4], mac[5]);
#endif
  WiFi.softAPConfig(IP_AP, IP_AP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(nombreWifi, WIFI_CLAVE, WIFI_CANAL, 0, 4);
#if WIFI_POTENCIA_REDUCIDA
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
  WiFi.onEvent([](arduino_event_id_t, arduino_event_info_t) { nuevoCliente = true; },
               ARDUINO_EVENT_WIFI_AP_STACONNECTED);
#if PORTAL_CAUTIVO
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", IP_AP);
#endif
  generaQR();
}

// ═════════════════════════════════════════════════════════════
//  API WEB
// ═════════════════════════════════════════════════════════════

int argInt(const char *nombre, int def) {
  return server.hasArg(nombre) ? server.arg(nombre).toInt() : def;
}

void enviaEstado() {
  char j[480];
  snprintf(j, sizeof j,
           "{\"esc\":\"%s\",\"animo\":%u,\"pres\":%u,\"dormido\":%u,\"brillo\":%u,\"vel\":%u,\"ojos\":%u,"
           "\"fondo\":%u,\"auto\":%u,\"luz\":%u,\"rot\":%u,\"inv\":%u,\"presIni\":%u,\"cli\":%u,"
           "\"wifi\":\"%s\",\"ver\":\"%s\",\"mem\":%u,\"fb\":%u}",
           NOMBRE_ESCENA[escena], (unsigned)animo, (unsigned)presentacion,
           (unsigned)(dormido || dormirPendiente), (unsigned)aj.brillo, (unsigned)aj.velocidad,
           (unsigned)aj.ojos, (unsigned)aj.fondo, (unsigned)aj.autonomo, (unsigned)aj.luz,
           (unsigned)aj.rotacion, (unsigned)aj.invertir, (unsigned)aj.presAlArrancar,
           (unsigned)WiFi.softAPgetStationNum(), nombreWifi, VERSION_FIRMWARE,
           (unsigned)ESP.getFreeHeap(), (unsigned)hayFB);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", j);
}

void rutaRaiz() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", PAGINA_HTML);
}

void apiCara() {
  interaccion();
  detienePresentacion();
  if (escena != ESC_CARA) aCara();
  ponAnimo(argInt("a", A_NORMAL));
  enviaEstado();
}

void apiAccion() {
  interaccion();
  detienePresentacion();
  if (escena != ESC_CARA) aCara();
  const String a = server.arg("a");
  if (a == "parpadeo") iniciaParpadeo(220);
  else if (a == "guino") iniciaAccion(AC_GUINO);
  else if (a == "si") iniciaAccion(AC_SI);
  else if (a == "no") iniciaAccion(AC_NO);
  else if (a == "mirar") iniciaAccion(AC_MIRAR);
  else if (a == "salto") iniciaAccion(AC_SALTO);
  enviaEstado();
}

void apiMirar() {
  interaccion();
  if (server.hasArg("soltar")) {
    mirObjX = mirObjY = 0;
    mirarManualHasta = t + 900;
  } else {
    if (escena != ESC_CARA) { detienePresentacion(); aCara(); }
    mirObjX = constrain(argInt("x", 0), -100, 100) * (MIRADA_X / 100.0f);
    mirObjY = constrain(argInt("y", 0), -100, 100) * (MIRADA_Y / 100.0f);
    mirarManualHasta = t + 6000;
  }
  server.send(200, "application/json", "{\"ok\":1}");
}

void apiDemo() {
  interaccion();
  const String d = server.arg("d");
  if (d == "presentacion") {
    if (argInt("on", 1)) iniciaPresentacion();
    else { detienePresentacion(); aCara(); }
  } else {
    detienePresentacion();
    if (d == "radar") cambiaEscena(ESC_RADAR, 7600, 0);
    else if (d == "ataque") cambiaEscena(ESC_ATAQUE, 7200, 0);
    else if (d == "logo") cambiaEscena(ESC_LOGO, 7000, 0);
    else if (d == "wifi") { cambiaEscena(ESC_WIFI, 30000, 0); wifiArranque = false; }
  }
  enviaEstado();
}

void apiTexto() {
  interaccion();
  detienePresentacion();
  String s = server.arg("t");
  s.trim();
  if (s.length()) copiaUTF8(mensaje, s.c_str(), sizeof mensaje);
  mensajeColor = constrain(argInt("c", 0), 0, 3);
  mensajeAncho = anchoTexto(F_GRANDE, mensaje);
  cambiaEscena(ESC_TEXTO, 0, 0);
  enviaEstado();
}

void apiTerminal() {
  interaccion();
  if (server.hasArg("abrir")) {
    detienePresentacion();
    abreTerminal();
  } else if (server.hasArg("cerrar")) {
    if (escena == ESC_TERMINAL) aCara();
  } else if (escena == ESC_TERMINAL) {
    const String c = server.arg("c");
    terminalTeclas(constrain(argInt("b", 0), 0, 64), c.c_str());
    if (server.hasArg("enter")) ejecutaEntrada();
  }
  enviaEstado();
}

void apiLienzo() {
  interaccion();
  if (server.hasArg("cerrar")) {
    if (escena == ESC_LIENZO) aCara();
  } else {
    if (server.hasArg("bg")) lienzoFondo = hex565(server.arg("bg"));
    if (server.hasArg("abrir") || server.hasArg("borrar")) {
      detienePresentacion();
      cambiaEscena(ESC_LIENZO, 0, 0);
      g->fillScreen(lienzoFondo);
      lienzoSucio = true;
    }
  }
  enviaEstado();
}

void apiTrazo() {
  interaccion();
  if (escena == ESC_LIENZO) {
    const uint16_t c = hex565(server.arg("pen"));
    const int gr = constrain(argInt("g", 5), 1, 16);
    const String pts = server.arg("pts");
    const char *p = pts.c_str();
    long px = -1, py = -1;
    while (*p) {   // formato "x,y;x,y;..."
      char *fin;
      long x = strtol(p, &fin, 10);
      if (*fin != ',') break;
      long y = strtol(fin + 1, &fin, 10);
      x = constrain(x, 0, ANCHO - 1);
      y = constrain(y, 0, ALTO - 1);
      if (px < 0) g->fillCircle(x, y, gr / 2, c);
      else lineaGruesa(px, py, x, y, gr, c);
      px = x;
      py = y;
      p = fin;
      if (*p == ';') p++;
      else break;
    }
    lienzoSucio = true;
  }
  server.send(200, "application/json", "{\"ok\":1}");
}

void apiAjustes() {
  interaccion();
  if (server.hasArg("brillo")) aj.brillo = constrain(argInt("brillo", 80), 5, 100);
  if (server.hasArg("vel")) aj.velocidad = constrain(argInt("vel", 2), 1, 3);
  if (server.hasArg("ojos")) { aj.ojos = constrain(argInt("ojos", 0), 0, N_TEMAS_OJOS - 1); preparaColores(); }
  if (server.hasArg("fondo")) aj.fondo = constrain(argInt("fondo", 0), 0, N_FONDOS - 1);
  if (server.hasArg("auto")) aj.autonomo = argInt("auto", 1);
  if (server.hasArg("luz")) { aj.luz = argInt("luz", 1); lienzoSucio = true; }
  if (server.hasArg("pres")) aj.presAlArrancar = argInt("pres", 0);
  if (server.hasArg("inv")) { aj.invertir = argInt("inv", 1); tft.invertDisplay(aj.invertir); }
  if (server.hasArg("rot")) {
    aj.rotacion = server.arg("rot") == "sig" ? (aj.rotacion + 1) & 3 : argInt("rot", 0) & 3;
    tft.setRotation(aj.rotacion);
    lienzoSucio = true;
  }
  aplicaBrillo();
  marcaAjustes();
  enviaEstado();
}

// Portal cautivo: cualquier dirección desconocida lleva al panel
void rutaNoEncontrada() {
#if PORTAL_CAUTIVO
  server.sendHeader("Location", String("http://") + IP_AP.toString() + "/", true);
  server.send(302, "text/plain", "");
#else
  server.send(404, "text/plain", "no encontrado");
#endif
}

// ═════════════════════════════════════════════════════════════
//  SETUP Y LOOP
// ═════════════════════════════════════════════════════════════

void renderFrame() {
  const uint32_t e = t - escenaT0;
  switch (escena) {
    case ESC_ARRANQUE: renderArranque(e); break;
    case ESC_WIFI:     renderWifi(e); break;
    case ESC_CARA:     renderCara(); break;
    case ESC_LOGO:     renderLogo(e); break;
    case ESC_SERVICIO: renderServicio(e); break;
    case ESC_RADAR:    renderRadar(e); break;
    case ESC_ATAQUE:   renderAtaque(e); break;
    case ESC_TEXTO:    renderTexto(e); break;
    case ESC_TERMINAL: renderTerminal(); break;
    case ESC_LIENZO:
      if (!lienzoSucio) return;   // el dibujo ya está en el framebuffer
      lienzoSucio = false;
      break;
  }
  if (hayFB) ram.envia(tft);
}

uint32_t avanzaReloj(uint32_t dtReal) {
  const uint32_t pct = aj.velocidad == 1 ? 65 : aj.velocidad == 3 ? 160 : 100;
  const uint32_t acc = dtReal * pct + restoReloj;
  restoReloj = acc % 100;
  t += acc / 100;
  return acc / 100;
}

void setup() {
  Serial.begin(115200);
  cargaAjustes();

  // Retroiluminación con PWM (20 kHz: no parpadea en cámara)
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PIN_TFT_BL, 20000, 8);
#else
  ledcSetup(0, 20000, 8);
  ledcAttachPin(PIN_TFT_BL, 0);
#endif
  escribeLuz(0);

  SPI.begin(PIN_TFT_SCK, -1, PIN_TFT_MOSI, PIN_TFT_CS);
  tft.init(240, 240, TFT_SPI_MODO);
  tft.setSPISpeed(TFT_SPI_HZ);
  tft.setRotation(aj.rotacion);
  tft.invertDisplay(aj.invertir);
  tft.fillScreen(C_MARINO);
  tft.cp437(true);
  aplicaBrillo();
  preparaColores();

  // Framebuffer de 240×240 (112 KB en 4 franjas) antes de arrancar el WiFi
  Serial.printf("[OrnitoSIRT] memoria libre: %u B · bloque mayor: %u B\n", (unsigned)ESP.getFreeHeap(),
                (unsigned)ESP.getMaxAllocHeap());
  hayFB = ram.reserva();
  if (hayFB) {
    g = &ram;
    g->cp437(true);
  } else {
    Serial.println("[OrnitoSIRT] Sin memoria para el framebuffer: modo directo");
  }

  for (int i = 0; i < 2; i++) ojoAct[i] = ojoObj[i] = FORMA[A_NORMAL][i];
  mensajeAncho = anchoTexto(F_GRANDE, mensaje);

  iniciaWifi();
  server.on("/", HTTP_GET, rutaRaiz);
  server.on("/api/estado", HTTP_GET, enviaEstado);
  server.on("/api/cara", HTTP_GET, apiCara);
  server.on("/api/accion", HTTP_GET, apiAccion);
  server.on("/api/mirar", HTTP_GET, apiMirar);
  server.on("/api/demo", HTTP_GET, apiDemo);
  server.on("/api/texto", HTTP_GET, apiTexto);
  server.on("/api/terminal", HTTP_GET, apiTerminal);
  server.on("/api/lienzo", HTTP_GET, apiLienzo);
  server.on("/api/lienzo/trazo", HTTP_GET, apiTrazo);
  server.on("/api/ajustes", HTTP_GET, apiAjustes);
  server.on("/favicon.ico", HTTP_GET, [] { server.send(204); });
  server.onNotFound(rutaNoEncontrada);
  server.begin();

  Serial.printf("\n[OrnitoSIRT] v%s · WiFi \"%s\" · http://%s\n", VERSION_FIRMWARE, nombreWifi,
                IP_AP.toString().c_str());
  Serial.printf("[OrnitoSIRT] framebuffer: %s · memoria libre: %u B · QR: %s\n", hayFB ? "sí" : "NO",
                (unsigned)ESP.getFreeHeap(), qrTam ? "sí" : "no");

  ultimaInteraccion = millis();
  relojAnt = millis();
  cambiaEscena(ESC_ARRANQUE, 3600, 0);
}

void loop() {
#if PORTAL_CAUTIVO
  dns.processNextRequest();
#endif
  server.handleClient();

  const uint32_t ahora = millis();
  const uint32_t periodo = hayFB ? 1000 / FPS_OBJETIVO : 120;   // sin framebuffer, menos fps
  if (ahora - relojAnt < periodo) {
    delay(1);
    return;
  }
  const uint32_t dt = avanzaReloj(min(ahora - relojAnt, (uint32_t)200));
  relojAnt = ahora;

  gestionaEventos(ahora);
  actualizaCara(dt);
  actualizaTerminal(ahora);
  compruebaFinEscena();
  if (aj.luz) renderFrame();
}
