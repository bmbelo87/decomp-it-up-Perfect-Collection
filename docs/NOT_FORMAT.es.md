**Idiomas:** [Português (BR)](NOT_FORMAT.md) · [English](NOT_FORMAT.en.md) · **Español**

# Formato STEP/*.NOT (PIU.EXE DOS, build Dec 18 2000)

Validado contra los **472** archivos de `E:\06_pc\game\STEP` (tools/not_parse.py): tamaño == 0x88 + 4*count en el 100%, filas estrictamente crecientes, fila mayor 1560 (< 2048).
Fuente en el binario: cargador `0x21361` (abre `STEP\%s`, nombre + ".NOT"@0x1200A4), descifra `0x29949`, expande `0x21251`.

## Layout (little-endian)
| Off | Tam | Campo |
|---|---|---|
| 0x00 | u32 | `count` — nº de registros |
| 0x04 | u32 | 0 (reservado) |
| 0x08 | f32[4] | **BPM por slot** (0x08..0x17; `DAT_001558A4[slot]`). 101_DB: [146.05,120,120,120,…]; los steps con cambio de BPM tienen varios valores (501_h2: 106.0, 131.9, 120…). `slot = DAT_003D0BF4` (C) |
| 0x18 | s32[4] | **offset de sincronía por slot, en centésimas de s** (`DAT_001558B4[slot]`) (C: usado en `pos − off*240/100`). 101_*=161, 102_*=530.  |
| 0x1C..0x37 | | ceros |
| 0x38 | u32 | `DAT_001558D4` = **N = filas por pulso** (C, derivado de la fórmula de Y). 2 o 4 en los 472 steps |
| 0x3C | u32 | siempre 4 (visto) — `DAT_001558D8` |
| 0x40 | u32 | varía por step (1,2,5,8,10,12,23…) — semántica desconocida |
| 0x44 | char[0x44] | título (terminado en NUL; **cp949** en los steps coreanos) |
| 0x88 | `count` × 4 B | registros **cifrados** |

## Cifrado (C — pseudocódigo de 0x29949)
```
key = 0xD2; for i in range(n): buf[i] ^= key; key = (key + 0x95) & 0xFF
```
Aplicado a todo el cuerpo (desde 0x88). La cabecera no está cifrada.

## Registro (C — 0x21251)
`struct { u16 row; u16 mask; }`
- `row`: índice de fila/tick (0..2047), creciente.
- `mask`: 13 bits leídos del **bit 15 → bit 3** (col = 15 − bit). Bit activo ⇒ `table[row*14 + col] = '1'`.
- Columnas **0–9**: 10 pads (P1 0–4, P2 5–9; los steps single solo usan 0–4). Columnas **10–12**: control/rejilla (casi siempre activas: 194/194 en 103_E1) — significado no confirmado (I: líneas de pulso/compás).
- Los bits 2..0 nunca se usan (0 en todos los archivos probados).
- Tras expandir, el cargador cambia `'1'`→`0x39` ('9') en las columnas 0–9 de las 0x800 filas (`0x15594C`, stride 14). **Atención:** el orden relevante es expandir (graba '1') y luego recorrer cambiando '1'→'9'; ver 0x21361 antes de reproducir.

## Nombres de archivo (C — strings @0x120691..)
`%d_H1 %d_E1 %d_DB %d_H4 %d_H2 %d_E2 %d_H3` (+ ".NOT"); en disco también aparecen en minúsculas (`501_db.NOT`, `h1..h4`, `e1,e2`, `db`).
Existe una ruta de depuración `.STF` (`run %d_e2.stf -demo %d`, `run step_name -d(ouble)`) — no hay `.STF` en el disco.

## Pendientes
- Semántica de 0x0C/0x10/0x14, 0x18 y 0x38–0x40; tiempo por fila (ticks → ms) — encontrarlo en la función que consume `0x15594C`.
- Comparar con el formato de PREX 3 (E:\14_px3) para reutilizar el lector.

## Mapeo cabecera → memoria (C — cargador 0x21361, leído del ensamblador)
- `fread(0x155874, 1, 8)`  ⇒ file[0x00..0x08) → `count`=0x155874, 0x155878
- `fread(0x1558A4, 1, 0x80)` ⇒ file[0x08..0x88) → 0x1558A4+(off−8). Ej.: 0x38→0x1558D4; título (0x44) → 0x1558E0
- `fread(0x1537EC, 1, count*4)` ⇒ registros (cifrados) → descifra in situ (0x29949) → expande (0x21251)
- Nombre del archivo: `strcpy(0x3D585A, nombre)`; `0x2FC2D(nombre, ".NOT")` (append); `sprintf(0x1557F4, "STEP\%s", nombre)`; `fopen(path, "rb"@0x1200B1)`.
- Convención Watcom: `fread(buf=EAX, size=EDX, n=EBX, fp=ECX)`.

## Estado de la tabla de notas (C — 0x22840)
`0x15594C + row*14 + col`: byte. Se dibuja solo para valor **> 0x32** (el cargador convierte '1'→'9', 0x39). Valores ≤ 0x32 = nota ya consumida/inexistente (inferido: el juicio reescribe la celda).

> **Nota de layout:** BPM y offsets no pueden ocupar los mismos bytes. Por las direcciones (`0x1558A4 + slot*4` para BPM y `0x1558B4 + slot*4` para offset) los f32[]/s32[] son arrays de **4 entradas cada uno** (0x08..0x17 = 4 BPM; 0x18..0x27 = 4 offsets; 0x28..0x37 = tercer array de 4, ver abajo). `slot` ∈ 0..3.

## Escala de tiempo y scroll (C — ensamblador de 0x25064–0x25121 y 0x22840)
- Tick del juego a **240 Hz** (`0x20514`: `inc [0x153738]`; `music_pos` = ticks desde el inicio del stage).
- `c14 = round( (pos − off[slot]*240/100) * bpm[slot] )` (FPU: `fild; fmul [0x1558A4+slot*4]; frndint; fistp [0x3D0C14]`).
  ⇒ **14400 unidades de c14 por pulso** (240 ticks/s × 60 s / bpm × bpm). `c18 = c14/240` = 60 × pulsos.
- Fila visible: `row = c18*N/60 = pulso × N`. Ventana: de `row0 = c18*N/60 − 2N` a `row0 + 10N` (de 2 pulsos atrás a 10 adelante).
- Y de una flecha: `Y = c14*mult/240 − (60*row/N)*mult` = `60*mult` px por pulso. `mult` = 1/2/4/8 (opción de velocidad).
- Compás: 4·N filas (marcadores de las columnas 10–12 cada 4N filas en 101_DB con N=2 ⇒ 8).
