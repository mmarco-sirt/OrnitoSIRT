"""
OrnitoSIRT · generador de recursos gráficos
===========================================

Convierte el logo de SIRT (SVG oficial), unos iconos y la tipografía Inter
(la de sirt.com) en mapas de transparencia de 4 bits que el firmware pinta
con antialiasing. Genera  firmware/OrnitoSIRT/recursos.h

Solo hace falta ejecutarlo si se cambian tamaños, pesos o caracteres.
El firmware ya incluye el recursos.h generado.

Uso:
    pip install pillow
    python herramientas/generar_recursos.py
"""

import math
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

AQUI = Path(__file__).resolve().parent
FUENTE_TTF = AQUI / "fuente" / "Inter.ttf"
SALIDA = AQUI.parent / "firmware" / "OrnitoSIRT" / "recursos.h"

SS = 8  # supermuestreo para formas vectoriales

# Caracteres disponibles en pantalla: ASCII imprimible + español
CARACTERES = [chr(c) for c in range(0x20, 0x7F)] + list("¡¿áéíóúüñÁÉÍÓÚÜÑçÇ·ºª°€àèòÀÈÒ…")

# (nombre en C, tamaño px, peso Inter)
FUENTES = [
    ("F_PEQ",    15, 500),   # textos pequeños
    ("F_MED",    22, 600),   # títulos
    ("F_FINA",   24, 300),   # eslóganes (estilo web SIRT, Inter ligera)
    ("F_GRANDE", 42, 600),   # mensajes en marquesina
]

# Logo SIRT · copiado de https://www.sirt.com/wp-content/uploads/logo-sirt-vectorial.svg
LOGO_PATH = (
    "M120,11.3v67.8c0,4.6-3.8,8.4-8.4,8.4s-8.4-3.8-8.4-8.4V11.3l0,0c0-4.6,3.8-8.3,8.4-8.3S120,6.7,120,11.3 L120,11.3z "
    "M162.2,3.1c-14,0-25.3,11.3-25.3,25.3v50.7c0,4.6,3.8,8.4,8.4,8.4c4.6,0,8.4-3.8,8.4-8.4V28.4c0-4.6,3.8-8.5,8.4-8.5 "
    "h29.3c4.6,0,8.4-3.8,8.4-8.4S196,3,191.4,3L162.2,3.1 "
    "M288.6,3.1h-67.3c-4.6,0-8.4,3.8-8.4,8.4s3.8,8.4,8.4,8.4h16.9l0,0 c4.6,0,8.4,3.8,8.4,8.4v50.8c0,4.6,3.8,8.4,8.4,8.4"
    "s8.4-3.8,8.4-8.4V28.3c0-4.6,3.8-8.4,8.4-8.4l0,0h16.9c4.6,0,8.4-3.8,8.4-8.4 S293.2,3.1,288.6,3.1L288.6,3.1z "
    "M60.7,36.8L60.7,36.8L60.7,36.8H27.3c-4.7,0-8.5-3.8-8.5-8.5s3.8-8.5,8.5-8.5h49.2 c4.6,0,8.4-3.8,8.4-8.4S81.1,3,76.5,3"
    "H27.3l0,0C13.3,3.1,2,14.5,2,28.4s11.3,25.3,25.3,25.3h34.1c4.7,0,8.5,3.8,8.5,8.5 s-3.8,8.5-8.5,8.5H10.6c-4.6,0-8.4,3.8-8.4,8.4"
    "s3.8,8.4,8.4,8.4h50.1C74.6,87.4,86,76.1,86,62.1S74.7,36.8,60.7,36.8L60.7,36.8z"
)

# (nombre en C, ancho px)
LOGOS = [("LOGO_GRANDE", 200), ("LOGO_PEQ", 84)]
ICONOS = [("ICONO_ESCUDO", 96), ("ICONO_SERVIDOR", 96), ("ICONO_NUBE", 96), ("ICONO_ESCUDO_G", 120)]


# ───────────────────────── SVG path → polígonos ─────────────────────────
def poligonos_svg(d, pasos=24):
    fichas = re.findall(r"[MmLlHhVvCcSsZz]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?", d)
    i, cmd = 0, None
    x = y = 0.0
    x0 = y0 = 0.0
    ctrl = None
    polis, actual = [], []

    def num():
        nonlocal i
        v = float(fichas[i])
        i += 1
        return v

    def cubica(p0, p1, p2, p3):
        for k in range(1, pasos + 1):
            t = k / pasos
            u = 1 - t
            actual.append((
                u**3 * p0[0] + 3 * u * u * t * p1[0] + 3 * u * t * t * p2[0] + t**3 * p3[0],
                u**3 * p0[1] + 3 * u * u * t * p1[1] + 3 * u * t * t * p2[1] + t**3 * p3[1],
            ))

    while i < len(fichas):
        if re.match(r"[A-Za-z]", fichas[i]):
            cmd = fichas[i]
            i += 1
            if cmd in "Zz":
                if actual:
                    polis.append(actual)
                actual = []
                x, y = x0, y0
                ctrl = None
                continue
        rel = cmd.islower()
        c = cmd.upper()
        if c == "M":
            nx, ny = num(), num()
            x, y = (x + nx, y + ny) if rel else (nx, ny)
            if actual:
                polis.append(actual)
            actual = [(x, y)]
            x0, y0 = x, y
            cmd = "l" if rel else "L"
            ctrl = None
        elif c == "L":
            nx, ny = num(), num()
            x, y = (x + nx, y + ny) if rel else (nx, ny)
            actual.append((x, y))
            ctrl = None
        elif c == "H":
            nx = num()
            x = x + nx if rel else nx
            actual.append((x, y))
            ctrl = None
        elif c == "V":
            ny = num()
            y = y + ny if rel else ny
            actual.append((x, y))
            ctrl = None
        elif c == "C":
            v = [num() for _ in range(6)]
            if rel:
                v = [v[k] + (x if k % 2 == 0 else y) for k in range(6)]
            cubica((x, y), (v[0], v[1]), (v[2], v[3]), (v[4], v[5]))
            ctrl = (v[2], v[3])
            x, y = v[4], v[5]
        elif c == "S":
            v = [num() for _ in range(4)]
            if rel:
                v = [v[k] + (x if k % 2 == 0 else y) for k in range(4)]
            c1 = (2 * x - ctrl[0], 2 * y - ctrl[1]) if ctrl else (x, y)
            cubica((x, y), c1, (v[0], v[1]), (v[2], v[3]))
            ctrl = (v[0], v[1])
            x, y = v[2], v[3]
    if actual:
        polis.append(actual)
    return polis


def a_4bits(img):
    """Imagen 'L' → (ancho, alto, bytes empaquetados a 4 bits, 2 píxeles/byte)."""
    w, h = img.size
    px = [round(v / 17) for v in img.tobytes()]
    if len(px) % 2:
        px.append(0)
    datos = bytes((px[k] << 4) | px[k + 1] for k in range(0, len(px), 2))
    return w, h, datos


def recorta(img):
    caja = img.getbbox()
    return img.crop(caja) if caja else img


# ───────────────────────── Logo ─────────────────────────
def render_logo(ancho):
    polis = poligonos_svg(LOGO_PATH)
    xs = [p[0] for pl in polis for p in pl]
    ys = [p[1] for pl in polis for p in pl]
    minx, maxx, miny, maxy = min(xs), max(xs), min(ys), max(ys)
    esc = ancho / (maxx - minx)
    alto = math.ceil((maxy - miny) * esc)
    img = Image.new("L", (ancho * SS, alto * SS), 0)
    dib = ImageDraw.Draw(img)
    for pl in polis:
        dib.polygon([((px - minx) * esc * SS, (py - miny) * esc * SS) for px, py in pl], fill=255)
    return img.reduce(SS)


# ───────────────────────── Iconos (dibujados en una caja 0..1) ─────────────────────────
def lienzo(lado):
    img = Image.new("L", (lado * SS, lado * SS), 0)
    return img, ImageDraw.Draw(img), lado * SS


def linea_gruesa(dib, pts, grosor, S, color):
    pts = [(x * S, y * S) for x, y in pts]
    dib.line(pts, fill=color, width=int(grosor * S), joint="curve")
    r = grosor * S / 2
    for x, y in (pts[0], pts[-1]):
        dib.ellipse((x - r, y - r, x + r, y + r), fill=color)


def bezier(p0, p1, p2, p3, n=30):
    out = []
    for k in range(n + 1):
        t = k / n
        u = 1 - t
        out.append((
            u**3 * p0[0] + 3 * u * u * t * p1[0] + 3 * u * t * t * p2[0] + t**3 * p3[0],
            u**3 * p0[1] + 3 * u * u * t * p1[1] + 3 * u * t * t * p2[1] + t**3 * p3[1],
        ))
    return out


def icono_escudo(lado):
    img, dib, S = lienzo(lado)
    contorno = [(0.5, 0.03), (0.88, 0.17), (0.88, 0.47)]
    contorno += bezier((0.88, 0.47), (0.88, 0.75), (0.68, 0.89), (0.5, 0.97))[1:]
    contorno += bezier((0.5, 0.97), (0.32, 0.89), (0.12, 0.75), (0.12, 0.47))[1:]
    contorno += [(0.12, 0.17)]
    dib.polygon([(x * S, y * S) for x, y in contorno], fill=255)
    linea_gruesa(dib, [(0.31, 0.50), (0.45, 0.64), (0.70, 0.36)], 0.10, S, 0)
    return img.reduce(SS)


def icono_servidor(lado):
    img, dib, S = lienzo(lado)
    for k in range(3):
        y0 = 0.08 + k * 0.30
        dib.rounded_rectangle((0.08 * S, y0 * S, 0.92 * S, (y0 + 0.24) * S), radius=0.06 * S, fill=255)
        yc = y0 + 0.12
        for xc in (0.70, 0.80):
            r = 0.035
            dib.ellipse(((xc - r) * S, (yc - r) * S, (xc + r) * S, (yc + r) * S), fill=0)
        dib.rounded_rectangle((0.18 * S, (yc - 0.025) * S, 0.52 * S, (yc + 0.025) * S), radius=0.025 * S, fill=0)
    return img.reduce(SS)


def icono_nube(lado):
    img, dib, S = lienzo(lado)
    for cx, cy, r in ((0.32, 0.56, 0.19), (0.55, 0.43, 0.26), (0.76, 0.59, 0.17)):
        dib.ellipse(((cx - r) * S, (cy - r) * S, (cx + r) * S, (cy + r) * S), fill=255)
    dib.rounded_rectangle((0.10 * S, 0.54 * S, 0.93 * S, 0.80 * S), radius=0.13 * S, fill=255)
    # flecha de subida (migración a cloud)
    linea_gruesa(dib, [(0.52, 0.71), (0.52, 0.47)], 0.07, S, 0)
    linea_gruesa(dib, [(0.41, 0.56), (0.52, 0.45), (0.63, 0.56)], 0.07, S, 0)
    return img.reduce(SS)


GENERADORES_ICONO = {
    "ICONO_ESCUDO": icono_escudo,
    "ICONO_ESCUDO_G": icono_escudo,
    "ICONO_SERVIDOR": icono_servidor,
    "ICONO_NUBE": icono_nube,
}


# ───────────────────────── Fuentes ─────────────────────────
def render_fuente(tam, peso):
    f = ImageFont.truetype(str(FUENTE_TTF), tam)
    f.set_variation_by_axes([max(14, min(32, tam)), peso])
    asc, desc = f.getmetrics()
    glifos, bits = [], bytearray()
    for ch in sorted(CARACTERES, key=ord):
        lienzo_g = Image.new("L", (tam * 3, tam * 3), 0)
        ox, oy = tam, tam * 2
        ImageDraw.Draw(lienzo_g).text((ox, oy), ch, font=f, fill=255, anchor="ls")
        caja = lienzo_g.getbbox()
        avance = round(f.getlength(ch))
        if caja is None:
            glifos.append((ord(ch), len(bits), 0, 0, 0, 0, avance))
            continue
        recorte = lienzo_g.crop(caja)
        w, h, datos = a_4bits(recorte)
        glifos.append((ord(ch), len(bits), w, h, caja[0] - ox, caja[1] - oy, avance))
        bits += datos
    return f, glifos, bytes(bits), asc, asc + desc


def ancho_texto(f, s):
    return round(f.getlength(s))


# ───────────────────────── Salida C ─────────────────────────
def bytes_c(nombre, datos):
    lineas = []
    for k in range(0, len(datos), 24):
        lineas.append("  " + ",".join(f"0x{b:02X}" for b in datos[k:k + 24]) + ",")
    return f"static const uint8_t {nombre}[] PROGMEM = {{\n" + "\n".join(lineas) + "\n};\n"


def main():
    partes = [
        "// ═══════════════════════════════════════════════════════════════════",
        "//  recursos.h · GENERADO por herramientas/generar_recursos.py",
        "//  No editar a mano: cambia el script y vuelve a ejecutarlo.",
        "//  Tipografía Inter (SIL Open Font License 1.1) · Logo © SIRT",
        "// ═══════════════════════════════════════════════════════════════════",
        "#pragma once",
        "#include <stdint.h>",
        "#include <pgmspace.h>",
        "",
        "// Mapas de transparencia de 4 bits (2 píxeles por byte, nibble alto primero)",
        "typedef struct { uint16_t cp; uint32_t off; uint8_t w, h; int8_t dx, dy; uint8_t avance; } GlifoAA;",
        "typedef struct { const uint8_t *bits; const GlifoAA *glifos; uint16_t n; uint8_t ascenso, alto; } FuenteAA;",
        "typedef struct { const uint8_t *bits; uint16_t w, h; } ImagenAA;",
        "",
    ]
    total = 0
    informe = []

    for nombre, ancho in LOGOS:
        w, h, datos = a_4bits(render_logo(ancho))
        partes.append(bytes_c(f"{nombre}_BITS", datos))
        partes.append(f"static const ImagenAA {nombre} = {{ {nombre}_BITS, {w}, {h} }};\n")
        total += len(datos)
        informe.append(f"{nombre}: {w}x{h} ({len(datos)} B)")

    for nombre, lado in ICONOS:
        img = GENERADORES_ICONO[nombre](lado)
        w, h, datos = a_4bits(img)
        partes.append(bytes_c(f"{nombre}_BITS", datos))
        partes.append(f"static const ImagenAA {nombre} = {{ {nombre}_BITS, {w}, {h} }};\n")
        total += len(datos)
        informe.append(f"{nombre}: {w}x{h} ({len(datos)} B)")

    medidas = {}
    for nombre, tam, peso in FUENTES:
        f, glifos, bits, asc, alto = render_fuente(tam, peso)
        partes.append(bytes_c(f"{nombre}_BITS", bits))
        filas = ",\n".join(
            f"  {{0x{cp:04X},{off},{w},{h},{dx},{dy},{av}}}" for cp, off, w, h, dx, dy, av in glifos
        )
        partes.append(f"static const GlifoAA {nombre}_GLIFOS[] PROGMEM = {{\n{filas}\n}};\n")
        partes.append(
            f"static const FuenteAA {nombre} = {{ {nombre}_BITS, {nombre}_GLIFOS, {len(glifos)}, {asc}, {alto} }};\n"
        )
        total += len(bits) + len(glifos) * 12
        informe.append(f"{nombre}: {tam}px/{peso} asc={asc} alto={alto} ({len(bits)} B)")
        medidas[nombre] = f

    SALIDA.write_text("\n".join(partes), encoding="utf-8")
    print(f"Escrito {SALIDA} · ~{total / 1024:.1f} KB de recursos")
    for linea in informe:
        print("  " + linea)
    # Anchos de los textos fijos, para comprobar que caben en 240 px
    pruebas = {
        "F_GRANDE": ["OrnitoSIRT", "¡ALERTA!", "SOC 24x7"],
        "F_MED": ["Ciberseguridad", "Infraestructura IT", "Amenaza bloqueada", "¡Hola! Soy OrnitoSIRT", "Escanéame"],
        "F_FINA": ["Expertos en servicios", "tecnológicos avanzados"],
        "F_PEQ": ["SOC 24x7 · Respuesta a incidentes", "Redes · Datacenter · Backup",
                  "Migración · Arquitecturas seguras", "WiFi: OrnitoSIRT · clave ornitosirt"],
    }
    for nombre, textos in pruebas.items():
        for t in textos:
            print(f"  {nombre:9s} {ancho_texto(medidas[nombre], t):4d}px  {t}")


if __name__ == "__main__":
    main()
