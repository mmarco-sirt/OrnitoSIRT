# Cubo modular · v2

Carcasa nueva para la electrónica del proyecto clawd-mochi: ESP32-C3 Super Mini + pantalla ST7789 de 1,54″.
El código original es MIT (mantened su aviso de licencia). Estas piezas son un diseño nuevo, sin logos.

Cambios respecto a v1: sin logos grabados y un solo imán por lado.

## Piezas (carpeta `stl/`, ya orientadas para imprimir)

| Archivo | Color | Cantidad | Soportes |
|---|---|---|---|
| carcasa.stl | Azul marino | 1 | No (frente sobre la cama) |
| tapa_trasera.stl | Azul marino | 1 | No (cara exterior sobre la cama) |
| cola.stl | Azul marino | 1 | No |
| pico.stl | Lima | 1 | No |
| patas.stl | Lima | 1 (son 2 patas) | No |
| gorra.stl | Lima | 1 | No (impresa de espaldas) |
| base_accesorio.stl | — | — | Plantilla para diseñar accesorios nuevos |
| prueba_encaje.stl | Cualquiera | 1 | Imprimir PRIMERO para calibrar |

Ajustes sugeridos: PLA o PETG, capa 0,2 mm, 3 perímetros, 15 % de relleno.

## Orden recomendado
1. Imprime `prueba_encaje.stl` y `base_accesorio.stl` (unos minutos). Si el tapón entra demasiado duro o flojo, cambia `holgura` en `cubo.scad` y vuelve a exportar.
2. Imprime la carcasa y prueba la pantalla en la ventana ANTES de pegar nada.
3. Imprime el resto.

## Material extra
- 1 imán de neodimio Ø6 × 3 mm en la cabeza y 1 en cada accesorio.
- Pegamento instantáneo (cianoacrilato) para los imanes.
- Cinta de doble cara (pantalla y ESP32), como en el original.

## Imanes y polaridad (importante)
- Cabeza: el imán entra desde DENTRO de la carcasa, por debajo del enchufe, centrado.
- Accesorio: entra por la cara inferior del tapón, a ras.
- La orientación la fija la esquina cortada del enchufe; el imán solo sujeta.
- Usa SIEMPRE la misma polaridad en todas las cabezas (por ejemplo, N hacia arriba) y la contraria en todos los accesorios. Si no, algún accesorio se repelerá.
- Truco: haz una "plantilla" con un imán marcado con rotulador y úsala para comprobar cada imán antes de encolar.

## Montaje
1. Pega la pantalla por dentro, centrada en la ventana, con cinta de doble cara.
2. La ESP32 va en el suelo, entre las guías, con el USB-C mirando hacia atrás.
3. Cablea según la tabla del proyecto original (VCC a 3,3 V, nunca a 5 V).
4. Encaja la tapa trasera a presión. El USB-C queda accesible por el hueco inferior.
5. Encaja el pico, la cola y las patas (un punto de pegamento si quedan flojos).

## Medidas a comprobar en la primera unidad
- **Ventana:** está hecha para la zona activa del panel (27,72 mm) y centrada a 32 mm de altura. Los módulos de 1,54″ varían de tamaño de PCB, así que medid el vuestro.
- **Hueco USB-C:** depende de cómo apoye vuestra ESP32 en el suelo. Ajustad `usb_z` si no alinea.
- Todas las cotas están al inicio de `cubo.scad`. Se abre con OpenSCAD (gratis) y se exporta cada pieza cambiando `parte`.

Medidas exteriores: 52 × 44 × 56 mm (+3 mm de patas).
