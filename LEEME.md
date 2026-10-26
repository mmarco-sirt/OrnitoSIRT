# OrnitoSIRT

La mascota de SIRT: un pájaro-cubo con pantalla de 1,3″ que pone caras, presenta los servicios de SIRT y se controla desde el móvil.

ESP32-C3 Super Mini + pantalla ST7789 1,3″ (240×240). Basado en [clawd-mochi](https://github.com/yousifamanuel/clawd-mochi) (MIT).

```
OrnitoSIRT/
├── firmware/
│   ├── OrnitoSIRT/                 ← código fuente (Arduino IDE)
│   │   ├── OrnitoSIRT.ino          ← lógica: caras, escenas, web
│   │   ├── config.h                ← pines, WiFi y tiempos  ← EMPEZAD AQUÍ
│   │   ├── pagina_web.h            ← panel de control del móvil
│   │   ├── tipos.h                 ← tipos de datos
│   │   ├── pantalla_ram.h          ← imagen en RAM (sin parpadeos)
│   │   └── recursos.h              ← logo, iconos y fuente Inter (generado)
│   ├── binarios/
│   │   ├── OrnitoSIRT_v1.0.bin     ← firmware listo para cargar
│   │   └── cargar_firmware.bat     ← carga con doble clic
│   └── clawd_mochi_original/       ← firmware original, para probar el montaje
├── capturas/                       ← todas las pantallas (renderizadas del firmware real)
├── hardware/                       ← carcasas 1,3″ y 1,54″ (STL + OpenSCAD)
├── herramientas/
│   ├── generar_recursos.py         ← regenera recursos.h
│   └── fuente/                     ← Inter (licencia OFL)
├── LICENSE-clawd-mochi-MIT.txt
└── LEEME.md
```

## 1. Cargar el firmware

Hay tres maneras. Las dos primeras no necesitan compilar nada.

### A. Con doble clic (Windows)
1. Conecta el cubo por USB-C (cable **de datos**).
2. Ejecuta `firmware/binarios/cargar_firmware.bat` y escribe el puerto (por ejemplo `COM3`).

Usa el `esptool` que instala el Arduino IDE con el soporte ESP32.

### B. Desde el navegador (cualquier PC, sin instalar nada)
1. Abre https://espressif.github.io/esptool-js/ en Chrome o Edge.
2. **Connect** → elige el puerto del cubo.
3. Flash Address `0x0` → archivo `OrnitoSIRT_v1.0.bin` → **Program**.

### C. Desde Arduino IDE (para modificar el código)
1. Arduino IDE 2.x + soporte ESP32 (File → Preferences → Additional boards manager URLs):
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   y en Boards Manager instala **esp32 by Espressif Systems** (probado con 3.3.12).
2. Librerías (Library Manager): `Adafruit GFX Library` y `Adafruit ST7735 and ST7789 Library`.
3. Abre `firmware/OrnitoSIRT/OrnitoSIRT.ino` con esta configuración en Tools:

| Ajuste | Valor |
|-|-|
| Board | ESP32C3 Dev Module |
| USB CDC On Boot | **Enabled** (imprescindible) |
| CPU Frequency | 160 MHz |
| Partition Scheme | Default 4MB with spiffs |
| Upload Speed | 921600 |

**Si no aparece el puerto o falla la carga:** mantén **BOOT**, pulsa y suelta **RESET**, suelta BOOT y repite.

## 2. Primer uso

1. Enchufa el cubo. Primero aparece el logo de SIRT y después un **QR**.
2. Escanea el QR con la cámara del móvil: se conecta al WiFi **OrnitoSIRT** (clave `ornitosirt`) y se abre el panel de control solo.
   Si no se abre, entra en **http://192.168.4.1**. Algunos móviles preguntan si quieres seguir en una red sin Internet: di que sí.
3. OrnitoSIRT te saluda. Ya puedes jugar con él.

## 3. Qué sabe hacer

| | |
|-|-|
| **12 expresiones** | normal, feliz, enfadado, triste (con lágrima), sorpresa, sospecha, cansado, amor (corazones que laten), euforia `> <`, mareado (espirales), dormido (Zzz) y vigilante (barrido tipo cámara de seguridad) |
| **Acciones** | parpadeo, guiño, saltito, sí, no, curiosear |
| **Mirada** | arrastra el dedo por el panel y los ojos lo siguen |
| **Vida propia** | parpadea, mira alrededor y hace gestos espontáneos. A los 5 minutos sin uso bosteza, se duerme y baja el brillo. Al tocar el panel se despierta |
| **Modo presentación** | bucle para stands: saludo → logo y eslogan → Ciberseguridad → Infraestructura IT → Cloud → radar → carrusel de caras → ataque simulado → QR. Se puede activar al arrancar |
| **Escanear la red** | radar del SOC que barre la red y termina en "Red segura" |
| **Simular un ataque** | sorpresa → **¡ALERTA!** con interferencias → escudo SIRT "Amenaza bloqueada" → "¡Protegido!" |
| **Mensaje** | texto grande en marquesina, con tildes, ñ, ¡ y ¿ |
| **Terminal** | consola en pantalla. Órdenes: `ayuda`, `hola`, `quien`, `sirt`, `escanear`, `soc`, `ip`, `cara`, `limpiar`. Probad `sudo` 😉 |
| **Lienzo** | dibuja con el dedo y aparece en la pantalla |
| **Ajustes** (se guardan) | brillo, velocidad, color de ojos, fondo, vida propia, pantalla on/off, presentación al arrancar, inversión de color, orientación |

Al conectarse un móvil nuevo, OrnitoSIRT da un saltito y dice "¡Hola!".

## 4. Identidad de marca

Colores sacados del kit oficial de https://www.sirt.com/:

| Uso | Color |
|-|-|
| Primario (lima) | `#C6D32B` · en el logo `#C1D42F` |
| Secundario (verde) | `#2AB17E` |
| Acento (petróleo) | `#076471` |
| Fondo (marino, como la carcasa) | `#070F26` |
| Fondo terminal | `#00191D` |
| Texto claro | `#E6E7E9` |

- **Ojos:** degradado lima → verde (también disponibles en lima, verde o blanco).
- **Logo:** el SVG oficial, con su degradado lima → verde → petróleo.
- **Tipografía:** Inter, la de la web, con antialiasing y caracteres españoles.
- **Color de alerta:** `#F25C3B`. Es el único que no es de marca y solo aparece en la demo de ataque.

## 5. Si algo no va bien

| Problema | Solución |
|-|-|
| Pantalla negra | Revisa el cableado. VCC a **3,3 V**, nunca 5 V. BLK al GPIO 3 |
| Imagen girada | Panel → Ajustes → **Girar** (se guarda), o `TFT_ROTACION` en `config.h` |
| Colores en negativo | Panel → Ajustes → **Inversión de color**, o `TFT_INVERTIR` en `config.h` |
| Rayas o ruido en la imagen | Baja `TFT_SPI_HZ` a `27000000` en `config.h` |
| No aparece el WiFi o se corta | Acércate. `WIFI_POTENCIA_REDUCIDA` ya viene activado (las Super Mini van mejor así) |
| El panel no se abre solo | Entra en http://192.168.4.1 |
| Varios cubos en la misma sala | `WIFI_SUFIJO_MAC 1` → cada uno se llama `OrnitoSIRT-XXXX` |

La pantalla se inicia en modo SPI 3, que funciona con los módulos de 1,3″ de 7 pines (sin CS) y con los de 8 pines. Si el vuestro no tiene CS, deja el GPIO 4 sin conectar.

## 6. Dónde se toca cada cosa

| Qué | Dónde |
|-|-|
| Pines, nombre y clave WiFi (**cambiadla antes de entregar**), tiempos | `config.h` |
| Forma de los ojos de cada expresión | tabla `FORMA` en `OrnitoSIRT.ino` |
| Guion del modo presentación | tabla `PRESENTACION` en `OrnitoSIRT.ino` |
| Textos de los servicios | `renderServicio()` en `OrnitoSIRT.ino` |
| Órdenes de la terminal | `ejecutaEntrada()` en `OrnitoSIRT.ino` |
| Panel web del móvil | `pagina_web.h` |
| Tamaños de fuente, logo e iconos | `herramientas/generar_recursos.py` (ver abajo) |

Cualquier texto en pantalla admite tildes y ñ directamente.

### Regenerar `recursos.h`
Solo hace falta si cambiáis tamaños o pesos de letra, iconos o caracteres:
```
pip install pillow
python herramientas/generar_recursos.py
```

### Espacio
El firmware ocupa el 94 % de la partición de 1,2 MB. Si al añadir cosas no cabe, en Tools → Partition Scheme elegid **Minimal SPIFFS (1.9MB APP with OTA)**. OrnitoSIRT no usa SPIFFS.

## 7. Licencias y créditos

- Código base: [clawd-mochi](https://github.com/yousifamanuel/clawd-mochi) © 2026 Yousuf Amanuel, licencia MIT (`LICENSE-clawd-mochi-MIT.txt`, conservadlo).
- Tipografía Inter © The Inter Project Authors, SIL Open Font License 1.1 (`herramientas/fuente/OFL-Inter.txt`).
- Logo y colores: © SIRT.

Recomendado: usad **Git** en esta carpeta para poder volver atrás si un cambio rompe algo.
