// =====================================================================
//  Cubo modular · v3 · pantalla 1,3"  (basado en la electrónica de clawd-mochi, MIT)
//  Carcasa nueva y paramétrica. Unidades: mm.
//
//  Ejes: X = ancho (izq->dcha), Y = fondo (frente y=0 -> detrás),
//        Z = alto (suelo z=0). El frente es la cara de la pantalla.
//
//  Elegir pieza:  openscad -D 'parte="carcasa"' -o carcasa.stl cubo.scad
//  Piezas: carcasa, tapa_trasera, pico, cola, patas, gorra,
//          base_accesorio, prueba_encaje, ensamblaje (solo vista)
// =====================================================================

parte = "ensamblaje";

/* ---------- Cuerpo ---------- */
W      = 52;     // ancho exterior
H      = 56;     // alto exterior (sin patas)
D      = 44;     // fondo total (carcasa + tapa trasera)
CH     = 2;      // chaflán de las aristas exteriores
pared  = 2.4;    // paredes laterales
frente = 2.0;    // pared frontal (la de la pantalla)
suelo  = 3.0;    // suelo
techo  = 2.4;    // techo
T_tapa = 2.4;    // grosor de la tapa trasera
D_c    = D - T_tapa;   // fondo de la carcasa

/* ---------- Pantalla ST7789 1,3" (240x240) ---------- */
// Zona activa del panel: 23,40 x 23,40 mm. La ventana deja 0,3 mm de margen.
// (Para volver a 1,54": ventana = 28.4)
ventana   = 24.0;
ventana_z = 32;    // altura del centro de la ventana
bisel     = 1.0;   // bisel exterior de la ventana

/* ---------- Enchufe de accesorios (estándar común) ---------- */
enchufe      = 20;    // lado nominal del tapón
holgura      = 0.2;   // holgura por lado (ajustar con prueba_encaje)
enchufe_prof = 3;     // profundidad del hueco
llave        = 4;     // esquina cortada (detrás-derecha) que fija la orientación
enchufe_y    = 24;    // posición en fondo del centro del enchufe
iman_d       = 6.3;   // imán Ø6 x 3 mm (+holgura)
iman_h       = 3.2;
piel         = 0.8;   // plástico entre imán y fondo del hueco

/* ---------- Pico y cola (encaje a presión) ---------- */
pico_w = 38;  pico_l = 12;  pico_h = 9;  pico_z = 5;
lengua_w = 22; lengua_h = 4; lengua_l = 3.5;
cola_w = 26;  cola_l = 22;  cola_h = 5;  cola_z = 12.2;
cola_lengua_w = 16; cola_lengua_h = 4; cola_lengua_l = 3.4;
ajuste = 0.2;          // holgura por lado en ranuras de pico/cola/tapa

/* ---------- ESP32-C3 Super Mini ---------- */
esp_w = 18;  esp_l = 22.52;
usb_w = 9.6; usb_h = 5; usb_z = suelo + 3;   // centro del hueco USB-C

/* ---------- Patas ---------- */
pata_w = 18; pata_l = 12; pata_h = 3; pata_sep = 13; pivote_d = 3;

$fn = 48;
e = 0.01;

// ---------------------------------------------------------------
module caja_chaflan(s, c) {
    hull() {
        translate([c, 0, c]) cube([s[0]-2*c, s[1], s[2]-2*c]);
        translate([0, c, c]) cube([s[0], s[1]-2*c, s[2]-2*c]);
        translate([c, c, 0]) cube([s[0]-2*c, s[1]-2*c, s[2]]);
    }
}

// Perfil cuadrado con la esquina (+X,+Y) cortada. Centrado en el origen.
module perfil_llave(lado, corte) {
    a = lado/2;
    polygon([[-a,-a],[a,-a],[a,a-corte],[a-corte,a],[-a,a]]);
}

// Un único imán centrado. La esquina cortada fija la orientación.
module huecos_iman(z0) {
    translate([0, 0, z0]) cylinder(d = iman_d, h = iman_h);
}

// Hueco hembra (en cabeza o en prueba). z_sup = cara superior.
module enchufe_hembra(z_sup) {
    translate([0, 0, z_sup - enchufe_prof])
        linear_extrude(enchufe_prof + 1) perfil_llave(enchufe + 2*holgura, llave);
    huecos_iman(z_sup - enchufe_prof - piel - iman_h);
}

// Tapón macho estándar que llevan todos los accesorios (debajo de z=0).
tapon_h = enchufe_prof - 0.2;
module tapon() {
    difference() {
        translate([0, 0, -tapon_h]) linear_extrude(tapon_h + e)
            perfil_llave(enchufe, llave + 0.4);
        huecos_iman(-tapon_h - e);
    }
}

// ---------------------------------------------------------------
module cuerpo_exterior() { caja_chaflan([W, D, H], CH); }

module carcasa() {
    boss = enchufe + 2*holgura + 2*2.4;
    z_boss = H - enchufe_prof - piel - iman_h;
    difference() {
        union() {
            difference() {
                intersection() { cuerpo_exterior(); translate([-1, -1, -1]) cube([W+2, D_c+1, H+2]); }
                translate([pared, frente, suelo]) cube([W - 2*pared, D_c, H - suelo - techo]);
            }
            // Refuerzo bajo el enchufe, con rampa a 45° para imprimir sin soportes
            translate([W/2, enchufe_y, 0]) hull() {
                translate([-boss/2, -boss/2, z_boss]) cube([boss, boss, H - techo - z_boss + e]);
                translate([-boss/2, -boss/2 - (H - techo - z_boss), H - techo]) cube([boss, boss + (H - techo - z_boss), e]);
            }
            // Guías para la ESP32 en el suelo (pegada con cinta)
            translate([W/2, D_c, suelo - e]) {
                for (s = [-1, 1]) translate([s*(esp_w/2 + 0.2 + 0.5) - 0.5, -(esp_l + 0.4) - 1, 0]) cube([1, esp_l + 1.4, 1.5]);
                translate([-(esp_w/2 + 1.2), -(esp_l + 0.4) - 1, 0]) cube([esp_w + 2.4, 1, 1.5]);
            }
        }
        // Ventana de la pantalla con bisel exterior
        translate([W/2, 0, ventana_z]) hull() {
            translate([-ventana/2, frente/2, -ventana/2]) cube([ventana, frente/2 + 1, ventana]);
            translate([-ventana/2 - bisel, -1, -ventana/2 - bisel]) cube([ventana + 2*bisel, 1 + e, ventana + 2*bisel]);
        }
        // Ranura del pico
        translate([W/2 - lengua_w/2 - ajuste, -1, pico_z - ajuste]) cube([lengua_w + 2*ajuste, frente + 2, lengua_h + 2*ajuste]);
        // Enchufe de accesorios + imanes (los imanes se meten desde dentro)
        translate([W/2, enchufe_y, 0]) enchufe_hembra(H);
        // Agujeros de las patas
        for (s = [-1, 1]) translate([W/2 + s*pata_sep, D_c/2, -e]) cylinder(d = pivote_d + 0.3, h = 2.6);
    }
}

module tapa_trasera() {
    lab = 1.6; lab_l = 4;
    ox = W - 2*pared - 2*0.15; oz = H - suelo - techo - 2*0.15;
    difference() {
        union() {
            intersection() { cuerpo_exterior(); translate([-1, D_c, -1]) cube([W+2, T_tapa + 1, H+2]); }
            // Labio que entra a presión en la carcasa (con hueco abajo para la ESP32)
            translate([pared + 0.15, D_c - lab_l, suelo + 0.15]) difference() {
                cube([ox, lab_l + e, oz]);
                translate([lab, -1, lab]) cube([ox - 2*lab, lab_l + 2, oz - 2*lab]);
                translate([ox/2 - 13, -1, -1]) cube([26, lab_l + 2, lab + 2]);
            }
        }
        // Hueco USB-C
        translate([W/2 - usb_w/2, D_c - lab_l - 1, usb_z - usb_h/2]) cube([usb_w, T_tapa + lab_l + 2, usb_h]);
        // Ranura de la cola
        translate([W/2 - cola_lengua_w/2 - ajuste, D_c - 1, cola_z - ajuste]) cube([cola_lengua_w + 2*ajuste, T_tapa + 2, cola_lengua_h + 2*ajuste]);
    }
}

module pico() {
    // Coordenadas de montaje: cara trasera del pico en y=0, se extiende hacia -Y
    translate([W/2 - pico_w/2, -pico_l, pico_z]) difference() {
        caja_chaflan([pico_w, pico_l + CH, pico_h], 1.5);
        translate([-1, pico_l, -1]) cube([pico_w + 2, CH + 1, pico_h + 2]);  // cara trasera plana
        for (s = [-1, 1]) translate([pico_w/2 + s*6 - 1.5, 2.5, pico_h - 0.6]) cube([3, 1.6, 1]);  // fosas
    }
    translate([W/2 - lengua_w/2, -e, pico_z]) cube([lengua_w, lengua_l + e, lengua_h]);
}

module cola() {
    translate([W/2 - cola_w/2, D, cola_z]) difference() {
        translate([0, -CH, 0]) caja_chaflan([cola_w, cola_l + CH, cola_h], 1.5);
        translate([-1, -CH - 1, -1]) cube([cola_w + 2, CH + 1, cola_h + 2]);
    }
    translate([W/2 - cola_lengua_w/2, D - T_tapa - (cola_lengua_l - T_tapa), cola_z]) cube([cola_lengua_w, cola_lengua_l + e, cola_lengua_h]);
}

module pata() {
    caja_chaflan([pata_w, pata_l, pata_h], 1);
    translate([pata_w/2, pata_l/2, pata_h - e]) cylinder(d = pivote_d, h = 2.3);
}

module gorra() {
    // Accesorio de ejemplo. Visera hacia -Y (frente).
    cw = 34; cl = 30; chh = 13;
    tapon();
    intersection() {   // copa con chaflán solo arriba
        translate([-cw/2, -cl/2, -3]) caja_chaflan([cw, cl, chh + 3], 3);
        translate([-cw, -cl, 0]) cube([2*cw, 2*cl, chh + 1]);
    }
    translate([-cw/2, -cl/2 - 13, 0]) hull() {
        translate([3, 0, 0]) cube([cw - 6, 13 + e, 2.4]);
        translate([0, 3, 0]) cube([cw, 10 + e, 2.4]);
    }
}

module base_accesorio() {
    // Plantilla para diseñar accesorios nuevos: tapón + plataforma de 26 x 26
    tapon();
    translate([-13, -13, 0]) cube([26, 26, 2]);
}

module prueba_encaje() {
    // Bloque pequeño con el mismo hueco que la cabeza, para calibrar la holgura
    s = 28; h = enchufe_prof + piel + iman_h;
    difference() {
        translate([-s/2, -s/2, 0]) cube([s, s, h]);
        enchufe_hembra(h);
        huecos_iman(-e);
        translate([0, -s/2 + 0.6, h/2]) rotate([90, 0, 0]) linear_extrude(1)
            text(str(holgura), size = 4, halign = "center", valign = "center");
    }
}

// --------------------- Orientaciones de impresión ---------------------
if (parte == "carcasa")        rotate([90, 0, 0]) carcasa();   // frente sobre la cama
if (parte == "tapa_trasera")   translate([0, 0, D]) rotate([-90, 0, 0]) translate([0, 0, 0]) tapa_trasera();
if (parte == "pico")           translate([0, 0, -pico_z]) pico();
if (parte == "cola")           translate([0, 0, -cola_z]) cola();
if (parte == "patas")          for (i = [0, 1]) translate([i*(pata_w + 5), 0, 0]) pata();
if (parte == "gorra")          translate([0, 0, 15]) rotate([-90, 0, 0]) gorra();
if (parte == "base_accesorio") translate([0, 0, 2]) rotate([180, 0, 0]) base_accesorio();
if (parte == "prueba_encaje")  prueba_encaje();

if (parte == "ensamblaje") {
    translate([0, 0, pata_h]) {
        color("#031428") carcasa();
        color("#0b2140") tapa_trasera();
        color("#C1D42F") pico();
        color("#031428") cola();
        color("#C1D42F") translate([W/2, enchufe_y, H]) gorra();
    }
    color("#C1D42F") for (s = [-1, 1]) translate([W/2 + s*pata_sep - pata_w/2, D_c/2 - pata_l/2, 0]) pata();
}
