**Idiomas:** [Português (BR)](FUNCTION_MAP.md) · [English](FUNCTION_MAP.en.md) · **Español**

# PIU.EXE (DOS, build "Dec 18 2000") — mapa de símbolos

Direcciones = VAs del LE original (también las de `piu_le.exe` abierto en REA).
No se puede renombrar en REA (Ghidra/Windows sin mutación) — **este archivo es la fuente de los nombres**.
Confianza: C = confirmado en el pseudocódigo, I = inferido (necesita corroboración).

## Layout (LE, 4 objetos)
| Obj | VA base | Tamaño | Contenido |
|---|---|---|---|
| 1 | 0x10000 | 0x10 | stub (descartado en el PE sintético) |
| 2 | 0x20000 | 0xE45F0 | código (juego 0x20000–0xFB…, runtime Watcom/Glide/zlib hasta ~0x104600) |
| 3 | 0x110000 | 0x48 | datos |
| 4 | 0x120000 | 0x9060D0 | datos/bss (52 páginas con contenido) |

## Boot (C)
- `0xFC004` `_cstart` (Watcom/DOS4GW; INT21h, DPMI) → `0xFF54C`, `0xFF4FD`, `0xFF597`
- `0xFF4FD` `__CMain` → llama a **`game_main` 0x21235**
- `0xFF597` ejecutor de finalizadores (tabla 0x15350C..0x153518, entradas de 6 bytes)
- `0x21235` **game_main**: `game_init 0x20CC0` → `game_loop 0x20F37` → `game_shutdown 0x20D7C`

## game_init 0x20CC0 (C, orden de llamada)
`0xFBEB2, 0xFBE3F, 0x20C5B, [DAT_0012B010=1 running], 0xFBF3A, 0xFBF93, 0x28297, 0x29218, 0x299D6, 0x20727, 0x2E61C, 0x21FE1, 0x264B1, 0x29E3B, 0x2D78A (si !=0 → state=0x1C service), 0x2A19B, 0x2FB34, 0x2F9C7, 0x2F95F(0,0x406E0000), 0x2FA4C, 0x266C0(&0x15374C), 0x20B84`
Nombres: aún sin asignar (0xFB… = probable init de runtime/Glide — I).

## game_shutdown 0x20D7C (C)
`0x2056F, 0xFBE91, 0xFBFA3, 0x2FB9A, 0x294B9, 0x282EA, 0x26516, 0x2E65B`
(refleja la init en orden inverso — I)

## game_loop 0x20F37 (C)
Globales: `running`=0x12B010, `game_state`=0x153710, `key`=0x153714 (scancode<<8), `input_flags`=0x153718, `debug_overlay`=0x153734, `dbg2`=0x15372C.
Por cuadro: `0xC76BC` (begin frame) → `0x20DCD` (lectura de entrada, devuelve scancode<<8 | flags) → teclas:
- 0x2D00 + flag 0x80000 → sale del bucle (X; I)
- 0x2900 → `0x26742`; 0x4200 → alterna `debug_overlay` (F8; I); 0x0100 → `game_state=0` (ESC; I); 0x4300 → `0x2087F` (F9; I)
- `0x26759()` decide `0x20E0E` vs `0x26948`; `0x27D08` (si state!=2); `0xC4FE0`
- **despacho por `game_state`** (tabla abajo) → `0x205C4` (overlay) → `0xC4F2C`, `0x20C1E`, `0xDB26E` (end frame/flip)

### Estados (game_state → handler) (C)
| state | handler | obs |
|---|---|---|
| 0,3,6-9,12,15,0x12,0x15,0x17-0x1B,0x1E | fallback 0x26611 / 0x20E7A | sin handler propio |
| 1 | 0x247E2 | |
| 2 | 0x255D9 | (0x27D08 no corre en el state 2) |
| 4 | 0x25639 | |
| 5 | 0x257CF | |
| 10 | 0x2B763 | |
| 11 | 0x2D0BF | |
| 13 | 0x285AB | |
| 14 | 0x2863A | |
| 16 | 0x28442 | |
| 17 | 0x28559 | |
| 19 | 0x284E0 | |
| 20 | 0x286A9 | |
| 22 | 0x28843 | |
| 23 | 0x288A4 | |
| 28 (0x1C) | 0x2E46A | pantalla de servicio/test (entrada) |
| 29 (0x1D) | **0x2E4D4** | pantalla de servicio: switch en `DAT_0012CB80` (9 subestados: 0x2D847, 0x2D954, 0x2DB18, 0x2DC59, 0x2DCDC, 0x2DFC0, 0x2E167, 0x2E2BC, 0x2E450); dibuja "PUMP IT UP (R%d/v%d.%02d)" y "Dec 18 2000" vía 0x26658(x,y,str) |
| 31 (0x1F) | 0x25E90 | |
| 32 (0x20) | 0x25F27 | |

Funciones utilitarias ya vistas: `0x26658(x,y,fmt,...)` = draw text (I); `0xFB6E2(n)` = stack check de Watcom (prólogo; ignorar).

## Músicas (STAGE.CFG) (C, fuente: archivo)
Rangos de ID: 1xx = 1st, 2xx = 2nd, 3xx = 3rd, 4xx = 4th, 5xx = de esta versión (informado por el usuario).

## Próximos objetivos
1. Parser de `stage.cfg` (string @0x120077) y formato del `.NOT`.
2. Cargador de BGA/audio/TITLE; capa Glide (gr* → OpenGL).
3. Handlers de estado de gameplay (identificar qué state = selección, juego, resultado).

---
## Actualización 2 — STAGE.CFG y convención de llamada

### Convención de llamada (C — observado en 0x20D37: `mov eax,0x120077; call 0x2A19B`)
Convención de registros de Watcom (`-3r`): args en **EAX, EDX, EBX, ECX**, el resto en la pila. El decompilador de Ghidra **pierde esos args**
(muestra `FUN_xxx(void)`). Confirmar siempre con `read_bytes`/ensamblador antes de asumir la firma. `0xFB6E2(n)` = `__CHK` (stack check), ignorar.

### Carga del STAGE.CFG
- `0x2A19B` **stage_cfg_load(const char *path)** (C): `fopen`(0xFBA01) → `while fgets`(0xFCACF) → tokeniza (0xFC6A5 → `DAT_006E1780`) → `strcmp`(0xFC570) con 3 palabras clave → handlers
  `0x2A039`, `0x2A0CA`, `0x2A16A` (I: `T`, `MODE`, `S`, el orden exacto aún no verificado) → `fclose`(0xFBC75) → bucle final: `DAT_006E163A[i*2] = DAT_006E163C[i*2] - DAT_006E1638[i*2]` (duración = fin − inicio, int16 stride 4; I)
- Contadores globales: `0x12C380`, `0x12C384`, `0x12C388` puestos a cero al inicio (I: nº de T / modos / entradas).
- Llamada en `game_init` con `"stage.cfg"` (@0x120077) en EAX.

### Formato STAGE.CFG (C — leído del archivo E:\06_pc\game\STAGE.CFG)
```
T <id>:<bpm>:<título>        105 músicas (id 1xx=19, 2xx=24, 3xx=30, 4xx=15, 5xx=17)
MODE<TAB><nombre>            EASY HARD DOUBLE REMIX REMIXDB CRAZY REMIXCZ MENU END
<TAB>S<TAB><nivel><TAB><id>  entradas de la sección (EASY=61 HARD=69 DOUBLE=70 REMIX=18 REMIXDB=15 CRAZY=37 REMIXCZ=4 MENU=5)
```
IDs: 1xx=1st, 2xx=2nd, 3xx=3rd, 4xx=4th, 5xx=esta versión. Los 5xx aparecen en EASY (509–517) y HARD/DOUBLE, etc.
Cada id se enlaza a `STEP/<id>_<DB|E1|E2|H1|H2…>.NOT` (patrón visto: 101_DB.NOT, 101_H1.NOT, 103_E1.NOT…).

---
## Actualización 3 — parser del STAGE.CFG cerrado + cargador de steps

### 0x2A19B stage_cfg_load (C) — handlers
El nombre del MODE **no se lee**: los modos son **posicionales** (orden en el archivo: 0 EASY,1 HARD,2 DOUBLE,3 REMIX,4 REMIXDB,5 CRAZY,6 REMIXCZ,7 MENU,8 END; máx. 9).
- `0x2A0CA` línea **T** (I: keyword no leída): `song[DAT_0012C380]`: stride 40 B @0x6DFC28: `+0 int id` (atoi 0xFC4F5), `+4 float bpm` (strtod 0xFC8DC), título vía 0xFBE3F; `DAT_0012C380++`.
- `0x2A039` línea **S**: entrada `DAT_0012C384` stride 4 B @0x6DF6B0: `[+1]=level`; si level==0 → `[+0]=0,[+2]=0`; si no `[+0]= (level>=12)`, y si >=12 `level-=10`; `[+2]=u16 id`. `DAT_0012C384++`.
- `0x2A16A` línea **MODE**: `mode_start[DAT_0012C388]` (u16, stride 4 @0x6E1638) = `DAT_0012C384`; `DAT_0012C388++` (tope 9). Fin: `mode_len[i]=start[i+1]-start[i]` (@0x6E163A).
(Cuáles de las 3 strings 0xFC570 compara cada handler aún no se ha leído de los bytes; la asignación T/S/MODE es por la forma de los handlers.)

### Cargador de step `0x21361` (C)
`sprintf(path,"STEP\%s", DAT_003D585A /*nombre*/)` → fopen → 3 freads (cabecera; ver NOT_FORMAT.es.md) → fclose → `0x21251` (descifra+expande) → recorre 0x800 filas×10 cols cambiando '1'→'9'. Fallo: `0x266C0("fail %s %s")`.
Strings de recursos: `AUDIO\%03d.AUD`, `AUDIO\D%03d.AUD`, `BGA\%03d.DAT`, `T%03d`(+.PNZ en TITLE), `.\cap\p5_%03d.TGA` (capturas), `font.tga`, `event.tga`, `BAR01.TGA`, `CD01.TGA`.
Formatos de datos en disco: `.NOT` (steps), `.AUD`, `.DAT` (BGA, + BGA_extracted/ ya extraído), `.PNZ` (TITLE; probable PNG comprimido — la build incluye libpng/zlib).

---
## Actualización 4 — renderizado de las flechas (0x22840)
`0x22840` **draw_arrows(player)** (C; player en EAX: columna base 0 = P1, 5 = P2). Flags de opción del jugador: `DAT_003D0C1C` (P1) / `DAT_003D5800` (P2).
- Multiplicador de velocidad `mult` (C): `flags&8`→8; si no `flags&4`→4; si no `flags&2`→2; si no 1.
- Otras flags (I): `0x10`/`0x100` eligen variante de sprite (`0x22750` / `0x2279F`); `0x200` y `DAT_0012B028&0x40` mezclan flags con las de `DAT_0015C94C/950` (efecto: espejo/cambio de lado — no confirmado).
- Ventana: `row0 = DAT_003D0C18*DAT_001558D4/60 - 2*DAT_001558D4` (mín. 0); recorre `[row0, row0 + 10*DAT_001558D4)`. `DAT_003D0C18` = tiempo de la música (C: usado como contador de 1/60; unidad por confirmar).
- Y de la flecha = `DAT_003D0C14*mult/240 - (row*60/DAT_001558D4)*mult`.
- 5 pasadas por columna, con translate X: col0→0, col1→48, col2→96, col3→144, col4→192 (C, floats 0/48/96/144/192). Orden de recorrido en el código: 0,2,4,1,3.
- Helpers (I): `0xC6EF8` translate, `0xC4FE0` push matrix, `0xC4F2C` pop, `0xC7944` set color, `0x2EE67` dibuja el sprite de la flecha (args de columna en registro, perdidos en el decompilador). Termina con llamada de cola `0x224FB`.
- Consumidores de `0x15594C`: 0x214A1 (57 B), 0x21762 (291 B), varios tramos en 0x21479–0x216EF (incluye el expand/cargador), 0x22840.

---
## Actualización 5 — cuadro de gameplay (0x24DE6) y tiempo
`0x24DE6` **gameplay_frame** (C; handler de estado de juego; termina con cola `0x224FB`). No se llama directo desde el switch: uno de los handlers de `game_state` (candidatos 0x247E2/0x255D9/0x25639) lo llama — por confirmar.
- `0x205B4` **get_music_pos** = `return DAT_00153738` (C).
- Flags de modo `DAT_0012B028` (ushort): bit0 = P1 activo, bit1 = P2 activo (C); `0x40` = demo/attract (I); `0x80|0x200` = modo doble/co-op (I, `&0x280` copia datos P1→P2); byte2 `&0x10` = modo especial (I).
- Cálculo de tiempo por cuadro (C):
  `pos = max(get_music_pos(), DAT_0012B030)`; `DAT_0012B02C = max(pos,0)`;
  `DAT_003D0C14 (scroll) = round( DAT_0012B02C − offset[DAT_003D0BF4]*0xF0/100 )`; `offset[]` = `DAT_001558B4` = **int[8] en file 0x18..0x38 del .NOT** (101_*: offset[0]=161) ⇒ **unidad de 0xF0 (=240) por unidad base; offset en centésimas**;
  `DAT_003D0C18 = scroll / 0xF0`.
  Debug (`DAT_0015372C`): imprime `"%02d:%02d:%02d"` con `pos/0x3840`, `(pos/0xF0)%60`, `(pos%0xF0)*100/0xF0` ⇒ **pos está en 1/240 s** (0x3840 = 14400 = 60 s × 240) — **C**. Así `DAT_003D0C18` = segundos enteros y `DAT_003D0C14` = 1/240 s.
  (Conflicto abierto: con 1/240 s, `row0 = c18*DAT_001558D4/60 − 2*N` da pocas filas/s; la fórmula de Y en 0x22840 puede usar `DAT_001558D4` como otra magnitud (ej. píxeles/pulso). Reconciliar leyendo 0x22840 en ensamblador antes de portar.)
- Fin de música: si `pos` llega al final → `DAT_00153710 = 10` (hasta 2 veces vía `DAT_0012B024`) o `= 4` (I: pantalla de resultado / próxima etapa).
- `DAT_003D0BEC`++ por cuadro (contador de animación); `DAT_003D0BD0`++ (cuadros de juego); `DAT_0012B034` = flag de "primer cuadro" (inicializa audio/video: 0x294B9, 0x2949F, 0x205A4, 0x2058F).
- Vida/gauge (I): P1 en `DAT_003D5808..`, P2 `DAT_003D5831..` (valores de prueba 500/400/300/200/100/900 puestos cuando `DAT_00153714==0x8600`, cheat/prueba tipo F12; float `DAT_003D580C`=1.0 = barra llena; fin de juego si gauge < 1 y `!(flags&0x40)`).
- Orden de dibujo por cuadro (C): bg `0x2643C` → (P1) `0x22674`, (P2) `0x226AD` (o `0x226F9` en doble) → flechas `0x22DCD`/`0x22DF1`/`0x22E20` (por jugador, si opción `&0x80` == 0) → `0x22FD8` → barras (translate X=176.0 / 572.0, Y=16.0; `0x21E47`, `0x21885`) → `0x23A4B` (por jugador; I: juicio/combos) → `0x21915`, `0x20134`.

## Transformaciones de opciones sobre la tabla de notas (C)
- `0x214A1` **copy_p1_to_p2**: para las 0x800 filas copia cols 0–4 → cols 5–9 (`0x155951 + r*14 + c = 0x15594C + r*14 + c`, c<5).
- `0x21762` **mirror_chart**: para cada fila, in situ y **en secuencia** (¡el orden importa!): si col src=='9' entonces dst='9' con (src→dst): 0→8, 1→9, 2→7, 3→5, 4→6, 5→3, 6→4, 7→2, 8→0, 9→1. Solo escribe '9', nunca borra. Reproducir el orden exacto.
- Otras transformaciones entre 0x214D9–0x216EF (refs a 0x15594C) aún no leídas.
- El juicio NO referencia `0x15594C` directamente (usa puntero/offset) — encontrarlo en 0x23A4B/0x22DCD.

## Actualización 6 — conflicto de escala resuelto (C)
Ver NOT_FORMAT.es.md §"Escala de tiempo y scroll". Tick por interrupción: `0x20514` (incrementa `0x15373C`, `0x153740`, `0x153738`; llama a `0x26CE5`, `0x29335`, y cada 2 ticks `0x27C23`/`0x20424`/`0x27D08`). `0x20514` es la ISR/callback de timer a 240 Hz.
`0x24C61–0x24D63` **stage_start** (C): lee el id del nombre en `0x3D6380`, carga `BGA\%03d.DAT` (fallback `BGAN.DAT` para ids 0x74–0x77 y 0xDB–0xE0), espera `0x2057F() >= 0x4B0` (1200 ticks = 5 s), `0x27B2A(0)`, pone a cero el tick (`0x2058F`) y `set_music_pos(0)` (`0x205A4`), `game_state = 2`.

---
## Actualización 7 — entrada (pads), vida y estados
### 0x27D08 poll_pads (C) — llamado por la ISR 0x20514 (state==2, cada 2 ticks) y por game_loop (state != 2)
`0xFC73A()` lee la placa de I/O (inp() en puerto; recomp: cambiar por teclado/gamepad). Bytes leídos: `3D64FC..3D6502`; `AH` = byte de servicio (activo en bajo).
Máscara de botones actual `DAT_003D6514` (`+0x153720`), pulsados en este tick `DAT_003D6510` (OR en `DAT_00153718`), soltados `DAT_003D650C` (`0x15371C`), anterior `DAT_003D6518`.
| Bit | Origen (C) |
|---|---|
| P1 0x08 / 0x10 | `[64FC] & 0x0F` / `& 0xF0` |
| P1 0x04 / 0x01 | `[64FD] & 0x0F` / `& 0xF0` |
| P1 0x02 | `[64FE] & 0x0F` |
| P2 0x800 / 0x1000 | `[6500] & 0x0F` / `& 0xF0` |
| P2 0x400 / 0x100 | `[6501] & 0x0F` / `& 0xF0` |
| P2 0x200 | `[6502] & 0x0F` |
| 0x100000 / 0x10000 / 0x20000 / 0x40000 | `AH&4==0` / `AH&2==0` / `AH&0x40==0` / `AH&0x80==0` (servicio/moneda/test — I) |
En `game_state==2` (y `DAT_0012B028 byte2 & 0x10 == 0`) o `==0x1D`: para cada bit P1 (0x01..0x10) pulsado/soltado llama a `0x27BE9`; para P2 (0x100..0x1000) llama a `0x27C1C` (**candidatos a juicio/efecto de pad**; el índice del pad pasa en registro — perdido en el decompilador; leer ensamblador).
Mapeo columna→bit probable: col0=0x01, col1=0x02, col2=0x04, col3=0x08, col4=0x10 (I, por orden de bits; confirmar en 0x27BE9).

### 0x23A4B draw_life_gauge(player) (C; player en EDX)
P1 en X=32.0/Y=444.0, P2 en X=352.0. Vida: `DAT_003D5810` (P1) / `DAT_003D5839` (P2); parpadea cuando `< 0xB4 (180)`. Estado "en música" = `game_state == 2` (C: `stage_start` define 2; la ISR solo corre poll/lógica de juego en ese estado).

### Mapa resumido de estados confirmados
- `2` = en música (stage_start 0x24C61.., ISR 240 Hz, poll_pads, gameplay_frame) — C/I
- `0x1D` = pantalla de servicio/test (0x2E4D4) — C
- `10`, `4` = destinos al final de la música (I: resultado / próxima etapa)


## Actualización 8 — Juicio (confirmado en ensamblador, capstone)

Corrección: 0x27BE9/0x27C1C NO son juicio; solo ponen bits de moneda/servicio en el puerto 0x2A4 (vía 0xFC743). Ghidra no definió funciones en 0x22E5F–0x24D64 ("not_in_procedure"); leer por capstone.

- apply_judgment 0x23229 (EAX=player 0/1, DL=nota '5'..'9'): switch por jump table 0x23215 (DL-0x35):
  '5' Perfect(0x234D5): contador +0x12 (0x3D5822), combo +0x0C++, vida +=(+0x04*12/1000), +0x14 combo-bonus; combo>3 => +0x3E8 en 0x3D5808
  '6' Great(0x2354A): contador +0x14 (0x3D5824), combo++, bonus +0x1F4
  '7' Good(0x235B8): contador +0x16 (0x3D5826), combo=0
  '8' Bad(0x235E6): contador +0x18 (0x3D5828), combo=0, vida -= 0x32
  '9' Miss(0x2362A): contador +0x1A (0x3D582A es el puntero de fila, ver abajo; contador de miss en otro offset), combo=0, vida -= f(vida)
  Struct del jugador: base 0x3D5810, stride 0x29. +0x00 vida(0x3D5810), +0x04 (0x3D5814), +0x0C combo(0x3D581C), +0x10 (0x3D5820), +0x1A fila actual (16.16, 0x3D582A), +0x43 (P2 fila 0x3D5853).
- Celdas de la tabla 0x15594C (stride 14 = 1 fila; 10 columnas; P1 0..4, P2 5..9): '0' vacío, '1' nota, '9' nota pendiente, '5'..'8' = Perfect..Bad ya juzgada. Nota de la fila = max(char) de las columnas (0x22F21 P1 col 0-4, 0x22F5D P2 col 5-9, 0x22F9C todas).
- Pulsación: en el cuadro, para cada bit de 0x153718 (P1) / 0x153719 (P2) (pulsado en este cuadro), si la celda de la fila actual 0x3D582A>>16 == '9' => se cambia por la nota calculada (BL) = clasificación por distancia:
  d = |(fila*60/N) - 0x3D0C18| (0xFC4D5=abs); d<=T0 => '5'; d<=T1 => '6'; d<=T2 => '7'; si no '8'.
  Umbrales (0x24BA6.., por BPM del slot b=[0x1558A4]): Tk = round((b*Mk/120 + Ck) * 0.5) con (M,C)=(8,4)->0x3D0BE4 Perfect, (16,5)->0x3D0BD8 Great, (24,6)->0x3D0BF0 Good, (32,7)->0x3D0BDC Bad. Unidad: la misma de la posición de scroll en 0x22840 (60 px/pulso).
- Mapeo columna<->bit P1: 0x01->[+0], 0x02->[+4], 0x04->[+2], 0x08->[+1], 0x10->[+3]; P2: 0x01->[+5(0x155951)]... (ver código 0x242E7).
- Después de que la fila pasa (0x3D582A>>16 > 0x1557EC = última fila juzgada): nota=max de la fila (0x22F9C); si '5'..'8' => apply_judgment; las celdas aún '9' pasan a Miss ('9'). Demo (flag 0x40): nota aleatoria '5'..'7' vía 0x23E2A (rand%10: <7 Perfect, <9 Great, si no Good).
- Fila actual: 0x23ED4 (por cuadro) calcula 0x3D0BD4 = round((12B02C - tiempo)*bpm/240 ...)*N/60+1 (clamp 0..2047, monótona) y la guarda en 0x3D582C; helpers 0x23E69 (recalcula el slot de BPM 0x3D0BF4 comparando 0x1558C4/0x1558C8 = puntos de cambio de BPM del 3er array).
- Resets 0x23155/0x23194/0x231D6: las celdas != '0' vuelven a '1' (columnas 0-4, 5-9, 0-9) — usadas cuando el jugador falla (stage break/continue).


## Actualización 9 — Formatos de archivo (confirmado, 100% de los archivos validan por Adler-32)

Checksum en todo = Adler-32 (0x298D1/0x2992E, mod 0xFFF1).
- Cargador genérico 0x289B0 (path EAX, buffer EDX): fread u32 H, luego hasta 0x300000 bytes; descifra 0x2896A; comprueba adler.
  AUDIO\%03d.AUD (0x29263), AUDIO\D%03d.AUD (0x29295), TITLE\%s.PNZ (0x2A533) usan este cargador.
  Cifrado: p[i] = bitrev8(c[i]) XOR KEY[(H+i)&0x3FF]; KEY = 1024 bytes en 0x12B13C (datos iniciales, obj4); H == adler32(p).
  Resultado: .PNZ = PNG; .AUD = MP3 (ID3v2). tools/piu_unpack.py (322/322 OK).
- Contenedor BGA\*.DAT "RES " (res_load 0x29A45; res_fopen 0x26290 lee el directorio de las 50 entradas de 0x1000-ish ...): cabecera 0x18 {magic,ver=2,count,datasize,?,adler(datos cifrados)};
  directorio count*28 cifrado con XOR(0xD2,+0x95) (0x29949) {name[16],size,offset,adler(plain)}; datos con la misma XOR. Contenido: .PNG(3380), .SPR(3583), .BGA(114 = script del BGA), .SP2(5). tools/piu_res.py (117/117 OK).
- Aún abiertos: formatos .BGA, .SPR, .SP2; ¿TGA (cap\p5_%03d.TGA, font.tga) en disco? (no hay); capa Glide.


## Actualización 10 — .BGA / .SPR / .SP2 (ensamblador + tools/piu_bga.py: 114/114 .BGA y todos los .SPR/.SP2 se parsean)

BGA (cargador 0x2628F -> res_load 0x29A45, entrada "<nombre>.bga"): cabecera de 16B {'BGA\0', u32 @obj+4 (120/146/180...: parece BPM, no confirmado), u32 @obj+8, u32 @obj+0}.
  Después HASTA 50 capas (el cargador lee 50 pero el archivo puede tener menos: 102.BGA tiene 40; fin = EOF): name[64] (.spr/.tga de la capa), u32 N, N keyframes de 44 B. Tamaño = 16 + 68*L + 44*sum(N), L<=50.
  Keyframe (render 0x25FA5; 0x2643C(obj,frame) recorre las 50 capas; 0x2647A(obj,layer,frame) solo una):
    s16 time, s16 active (0 => no dibuja/termina), f32 x, y, pivx, pivy, scale, rot, r, g, b, a  (colores x 0x12B0B0/0x12B0B4 = 1.0)
    Antes de la 1ª key no se dibuja nada; entre las keys i-1 e i interpola linealmente (0x25F88: a+(b-a)*t, t=(f-t0)/(t1-t0)) todos los campos; tras la última key => devuelve 1 (fin).
    Matriz: translate(0,480) -> translate(x,-y) -> pivot/rotate(rot, Z) -> scale(s,s,1) (Y invertido, pantalla 640x480); glColor(r,g,b,a); sprite dibujado por 0x2EFAD(sprite=layer+0x44, t).
SPR/SP2 (texto, CRLF): "TYPE <TILE|ANI|PATTERN a w h|IMAGE>" / "NUM n" (2 archivos omiten la palabra NUM) / n líneas "T archivo.tga x y w h u0 v0 u1 v1" (píxeles, textura 256x256; ".tga" se mapea al .PNG del mismo nombre en el RES).
  0x2EFAD (dibujo por tipo, campo u16 en +0x20): tipo 0/1 (TILE/IMAGE) dibuja todas las entradas; 2 (ANI) elige el cuadro floor(t*n); 3 (PATTERN a w h) w*h tiles repitiendo/desplazando los cuadros con parámetro a (inferido).
  0x2EEFF dibuja 1 quad: vértices de pantalla (words en +0x24,+0x26,+0x28,+0x2A; Y=480-y), UV float en +0x2C..+0x38.
  SP2 (arrow541..545): líneas de 11 tokens "T tag archivo x y w h u v w h" (tag extra; fuente probablemente u,v,w,h) -- no confirmado.
Abierto: u32 @obj+8/@obj+0 de la cabecera BGA, enlace BGA<->tiempo de la música, parámetro a del PATTERN, font.tga/event.tga.


## Actualización 11 — Esqueleto C (src/)
- `src/piu.h` API; `piu_data.c` (Adler-32, cifrados, RES, .NOT, .BGA, interpolación 0x25FA5); `piu_play.c` (reloj 240 Hz, fila actual, ventanas por BPM, classify, apply_judgment); `piu_key.h` (KEY 1024 B extraída del exe); `main_test.c` (`build/piu_test.exe ../game`).
- Validación: AUD 213/213, PNZ 109/109, DAT 117/117 (todas las entradas y .BGA), NOT 472/472. Autoplay 101_DB (bpm 146.1, N=2): ventanas P/G/Go/B = 7/12/18/23 px; 92 notas; error 0 y 6 => 92 Perfect; error 20 => mezcla (P37 G18 Go29 B8).
- Pendiente para fidelidad: vida inicial/límites (0x23A4B), `[0x3D0BE0]` (modificador de life_rate; -700 puesto en 0x24B2D..), cálculo de puntuación final, hold/long notes (¿máscara columnas 10-12?), pantalla de resultado, Glide->OpenGL, SDL2 + ventana, audio MP3, entrada.


## Actualización 12 — Juego jugable (src/game.c)
SDL2+OpenGL 1.x. `PiuReconstructed.exe`/`jogar.bat`. Selección (STAGE.CFG + TITLE/Txxx.PNZ), juego (BGA del .DAT con keyframes/SPR, flechas de ARROW01/03.PNG, juicio 0x23229/ventanas 0x24BA6, fila perdida=Miss, vida 500 inicial, fail en 0), resultado. Cuadro del BGA = pos/4 (60 fps; 0x250C5). Selftest con autoplay: 172/172 Perfect en 101_H1; sin entrada: Misses y fail. Pendiente: gamepad, previa de música, sprites/efectos originales (SP2 ARROW541-545), hold/long notes, pantallas originales.


## Actualización 13 - flujo attract/boot (confirmado en PIU.EXE)
- Bucle principal FUN_00020F37; estado en DAT_00153710: 0 idle, 0x0A/0x0B selección (0x2B763/0x2D0BF), 0x0D/0x0E logo, 0x10/0x11 intro, 0x13/0x14 título, 0x16/0x17 pantalla 03.DAT (0x28843/0x288A4, 1920 cuadros), 0x1C setup (0x2E46A), 0x1D (0x2E4D4), 0x01 juego (0x247E2).
- Tabla de nombres en 0x12B0C9 {ptr nombre, fn}: exit, idle, run, setup, sel, intro, logo, title.
- idle 0x20E7A: crédito (FUN_204BA; 99 si free play) -> estado 0x13; si no n=DAT_001537D8++%4: 0 logo(81.DAT), 1 intro(01.DAT), 3 FUN_2D14D (no reconstruido).
- Contador de cuadros DAT_003D651C; FUN_2643C(EAX=bga, EDX=cuadro) dibuja las 50 capas. Logo/intro: cuadro = contador ANTES del ++; título: cuadro = contador % 180 (IDIV 0xB4 en 0x2874D).
- el logo termina en 500 (0x1F4); la intro en 0x3B1; el título vuelve a idle con contador >= 0x708 sin crédito/start. START exige crédito (FUN_2047F = DAT_00153730 / DAT_006E191A); 60 cuadros (0x3C) después del START (o P1+P2) -> 0x2B763 (selección).
- DAT_006E1919 != 0 y DAT_006E1914 == 0 -> pasa por la pantalla 03.DAT (0x28843) antes de la selección (los valores vienen de configuración en runtime; se asume 0).
- Texto del título: printf("%d%02d",3,0x34) en (601,458) negro y (600,459) blanco = "352" (versión). Overlay FUN_20134: CREDIT(S)/INSERT COIN(S)/PRESS CENTER STEP de FONT.PNG (00.DAT), alfa parpadeante |(t%200-100)*0.01|+0.2 (floats en 0x12000C/0x120010). Posiciones en pantalla: INFERIDAS.
- Moneda: FUN_20424 (bit 0x20000 de la entrada, estado != 0x1D): DAT_00153730++. ESC (0x100) -> estado 0. Alt+X sale.
- Pendiente: FUN_2D14D (demo/ranking), selección real 0x2D0BF, pantalla 03.DAT, 83.DAT/84.DAT, setup 0x2E46A, resultado real.

## Actualización 14 - selección real (estado 0xB, FUN_0002D0BF) - ANÁLISIS PARCIAL
- Submáquina DAT_006E1798 (0..7): 0 init FUN_2BB5E (carga BGA\099.DAT, toca AUDIO\%03d.AUD vía FUN_29263), 1 intro FUN_2BC14, 2 selección FUN_2BD93, 3 FUN_2C4AB, 4 FUN_2C63F (BGA\098.DAT), 5 FUN_2C81C, 6 FUN_2C840 (grande), 7 FUN_2CDFA. Las fases 2 y 6 llaman a FUN_2CFC2/FUN_2ABDA.
- BGA/099.DAT = pantalla "Music Select" (título, TIME, rieles, botones SHIFT, "1P/2P PLAYER PRESS CENTER STEP TO JOIN"); 99.DAT/98.DAT = menu_bg + botones de modo (czp, btlp, dbp, ezp, hdp...); 091..096.DAT = botones por nivel (NB/HB/...); 090/091(2 dígitos).DAT = barras de título (BARnn.PNG = hojas 256x256 con 8 barras, CDnn.PNG = portadas redondas).
- Orden de las barras: por id de música, solo para músicas que tienen barra (BAR01 = 109..115,201; BAR02 = 202..210...). Mapeo exacto id->(hoja,fila) NO confirmado.
- STAGE.CFG tiene secciones "MODE EASY ..." con líneas "S nivel id" separadas por línea en blanco (páginas) - el original usa esto para las listas.
- Lista horizontal con paso 0x42 (66 px) en FUN_2BD93; temporizador de selección 0x1E - (FUN_2057F/0xF0) (30 s); teclas por jugador: bits 8/0x10/1/2 = izq/der/..., modo en DAT_006E1778 (1..6 -> flags 0x10,0x20,0x100,0x80,0x40,0x400 en DAT_0012B028).
- 83.DAT = BGA de la pantalla de juego (ctx2, bar, t_perf/t_great/t_good/t_bad/t_miss/t_combo, ngb, lin; 0..2790 cuadros); 84.DAT = ending; 03.DAT = pantalla de ayuda (1935 cuadros).
- Herramienta: piu.exe -view <nombre> -frame N -selftest 1 -shot x.ppm renderiza cualquier BGA.

## Actualización 15 - selección real: fase 2 = elección de MODO (FUN_0002bd93) [observado]
- FUN_0002bc14 (fase 1->2): dibuja 099 (FUN_0002643c), pone a cero DAT_006e17a4 (contador de animación), DAT_006e1798=2, DAT_006e1784=7.
- FUN_0002bd93 (fase 2): lista vertical de modos, ítem = 0x42 px, el scroll anima 0x16 cuadros (3 px/cuadro).
  - DAT_006e1788 = dirección (0/1 parado, 2 sube, 3 baja); DAT_006e179c = ítem actual; DAT_006e178c = total de ítems.
  - Entrada: DAT_00153718 (P1: bits 1,2,8,0x10) y byte alto (P2); DAT_00153720/21 bit1/bit2 = repetición.
  - Ítem (DAT_006e1788+DAT_006e179c) -> DAT_006e1778: 0->4 (o 5 si 2 jugadores), 1->1, 2->2, 3->3, 4->6.
  - Confirma: bits en DAT_0012b028 |= 0x10(modo1) 0x20(2) 0x100(3) 0x80(4) 0x40(5) 0x400(6); DAT_006e1798=3 (próxima fase).
  - Temporizador: 30 s (0x1e - tiempo/0xF0) -> autoconfirma el ítem actual.
- Conclusión: el orden real es 099 -> elección de MODO -> (fase 3) música. La lista de músicas solo viene en la fase 3.

## Actualización 16 - búsqueda de SFX en PIU.EXE (resultado negativo parcial)
- PIU.EXE: 0 apariciones de RIFF/WAVE/fmt/.WAV/VOC/ID3/OggS; sin bloque PCM (entropía de código ~6 hasta 0xF0000, después datos bajos).
- Únicos formatos de archivo cargados: AUDIO\%03d.AUD (0x120336) y AUDIO\D%03d.AUD (0x120345) vía FUN_00029263 (solo arma el nombre; id en EAX), BGA\*.DAT (SPR/PNG/BGA/SP2).
- BGA\*.DAT (234 DATs/entradas): solo .SPR .PNG .BGA .SP2 -> ningún audio dentro.
- AUDIO\001/002/003.AUD = MP3 de 128 kbps de 18,2 s / 29,2 s / 36,3 s (músicas de pantalla, no SFX). FUN_28843 (ayuda, estado 0x17) toca 003 (MOV EAX,3 antes de 0x29263) -> 003 = música de la ayuda. 001 probablemente intro (FUN_28442 pasa el id en EAX; no confirmado).
- Menú de service del EXE (strings @0x1056065): "SOUND TEST", "EFFECT SOUND", "DEMO SOUND", "AUDIO" -> existe opción de efecto de sonido, pero la fuente del sonido no está en archivo ni incrustada de forma detectable.
- Conclusión: no se encontró ningún SFX en archivo ni incrustado. Hipótesis (no confirmada): los efectos venían del hardware de sonido del gabinete arcade.

## Actualización 17 - SFX: son de HARDWARE (placa de sonido), no de archivo [observado + inferido]
- FUN_00030fac(EAX=on/off, EDX=idx, EBX=voz) = "toca efecto": graba un descriptor de 16 bytes (tabla @0x7238A0 + idx*16) en registros de un chip de sonido por el puerto [0x724AB8] (FUN_00030085 = escritura en puerto; pares dirección/dato en base+0..3, +0x20..23, +0x40..43, +0x60..63) y termina con key-on (reg1 | 0x80).
- Llamadores confirmados: moneda (FUN_00020424: FUN_30fac), selección fase 2 (FUN_2bd93: mover ítem y confirmar), entre otros.
- Init: FUN_00028297 = init del chip (27cd4/27b2a, espera 500 ms, detecta con FUN_3037c) y luego FUN_3155e lee PIU.BIN en registros de 16 bytes (tamaño>>4 = 35) para la tabla de voces, después FUN_30fa5.
- PIU.BIN (560 bytes) = 35 descriptores de voz (direcciones encadenadas: el final de uno = inicio del siguiente) -> el PCM en sí NO está en archivo ni en el EXE: está en la ROM de la placa de sonido arcade.
- Conclusión: no existen WAVs de esta versión para extraer; los 35 efectos solo pueden sustituirse por muestras externas.

## Actualización 18 - SFX: carpeta EFFECTS + ELF Linux (piu, R5/v3.52, no stripped) [observado]
- game/EFFECTS tiene 35 WAV = las 35 voces de PIU.BIN. Orden/índice = tabla `effects_file_names` (@0x08113500 del ELF): 0 01-1,1 01-11,2 2-1,3 3-2,4 4-2,5 5-1,6 6-2,7 7-1,8 8-1,9 9-5,10 9-A,11 9-B,12 9-C,13 9-D,14 9-F,15 TIME_LIMIT,16 10-2,17 13-1,18 12-1,19 14,20 START1,21 START2,22 START3,23..27 RANK_A,B,C,D,F,28..31 NOTGOOD1-4,32..34 GOODJOB1-3.
- El ELF es una traducción del código DOS: símbolos jump_XXXXX / func_XXXXX = direcciones VA del PIU.EXE (ej.: func_294b9, jump_2c14d). ac_sound_play_effect(EAX=0, EDX=índice, EBX=canal) = FUN_00030fac.
- Llamadas (VA DOS: índice/canal): 0x20470 moneda 21/-; 0x2c14d,2c181,2c249,2c27a (mover I/D en el modo) y 0x2cb76,2cbaa,2cc72,2cca3 (mover I/D en la música) 16/4 (10-2); 0x2c486 y 0x2cdc3 confirmar 4/3 (4-2); 0x2882e,288f2,2acda,2acf9,2d048,2d0xx pulsaciones de pad 3/2 (3-2); 0x2a91b 2/1 (2-1, 5 s); 0x25e44 rank 27/6 (RANK_F, los demás ranks entran por el mismo punto); 0x24f50,24f9a 7/7 (7-1); 0x234b5 9/2 (9-5); 0x2e151 parar efecto (eax=1).
- game.c: sfx_load/sfx_play (8 canales, uno nuevo en el mismo canal corta el anterior). Conectados: moneda, mover, entrar a la música, join en el título, rank del resultado. NO conectados (punto de llamada aún no rastreado): "1 crédito completo", NOTGOOD/GOODJOB, TIME_LIMIT, 2-1, 7-1, 9-x.

## Actualización 19 - menú de SERVICE reconstruido [observado, salvo donde se marca]
- Despachador FUN_0002E4D4: DAT_0012CB80 = pantalla (0 menú FUN_2D847, 1 I/O FUN_2D954, 2 EEPROM FUN_2DB18, 3 SCREEN FUN_2DC59, 4 GAME OPTION FUN_2DCDC, 5 COIN OPTION FUN_2DFC0, 6 SOUND TEST FUN_2E167, 7 DATE FUN_2E2BC, 8 EXIT FUN_2E450). Encabezado "PUMP IT UP (R%d/v%d.%02d)" (4,4) con 5,3,0x34 (= R5/v3.52, igual al archivo `version` del ELF) y "Dec 18 2000" (0x224,4); pie (0xEC,0x24) "MOVE - TEST BUTTON" + (0xEC,0x10) "SELECT - SERVICE BUTTON", o solo "EXIT - SERVICE BUTTON" en las pantallas 1-3. Coordenadas y-hacia-arriba, fuente de 8 px de avance (FUN_00026658 = texto).
- Botones: DAT_00153718 == 0x10000 TEST (mover, cursor DAT_0012CB84 en el menú / DAT_0012CB88 en las filas), == 0x20000 SERVICE (seleccionar).
- Menú: título "GAME SETTING" (0x114,0x1C2); ítems x=0x114, y=0x160-0x14*i; "Lock OK:%d Err:%d" (0x114,0x98) (contadores de FUN_29818).
- Colores (FUN_000C7E84): fila seleccionada rojo (0x12CB98), normal blanco (0x12CBE0), valor por defecto verde (0x12CBB0).
- GAME OPTION (valores x=0x194): GAME MODE 6e1914 NORMAL/EVENT; LEVEL 6e1915 0..2 (1.EASY 2.NORMAL 3.HARD, por defecto 1); STAGE BREAK 6e1916 0..4 (OFF / "%d STAGE", por defecto 2); LANGUAGE 6e1917 0..2 (ENGLISH, ENGLISH, CHINESE); DEMO SOUND 6e1918 OFF/ON; SHOW HELP 6e1919 OFF/ON; DEFAULT SETTING FUN_2D6B5 (14=0,15=1,16=2,17=0,18=0,19=0,1b=1); SAVE AND EXIT FUN_2D750; EXIT.
- COIN OPTION: COIN SETTING 6e191a 0..10 (0 = "FREE", si no "1 CREDIT / %2d COINS" en x=0x174), DEFAULT SETTING FUN_2D70F, SAVE AND EXIT, EXIT.
- SOUND TEST: AUDIO (#%02d, 6e1920) y EFFECT SOUND (#%02d, 6e191c), EXIT; TEST detiene el sonido (FUN_294B9) y mueve; SERVICE toca (FUN_2E0ED audio / FUN_2E12A efecto).
- DATE: YEAR(%4d, 6e192c=2000), MONTH(%2d, 6e1928 solo 11/12), DAY(%2d, 6e1924; 1..30 si mes 11, 1..31 si 12), EXIT guarda (FUN_2D3AB).
- I/O TEST: "1.TEST  BUTTON" (0x104,400) "2.SERVICE BUTTON" (0x17C) "3.CLEAR BUTTON" (0x168) "4.COIN" (0x154), estado por FUN_2D640 (no leído; aquí: el botón pulsado se pone rojo - inferido). EEPROM TEST: WRITE/READ/RESULT con "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345" y SUCCESS/FAIL.
- INFERIDO: pantalla SCREEN TEST (FUN_2D194/2D259 no leídas: barras de color), valor de AUDIO #id, archivo de persistencia PIU_EEPROM.BIN (el original graba en EEPROM), coin por defecto 1.
- game.c: svc_open/svc_press/svc_draw; F1 abre/SERVICE, F2 TEST, F3 coin clear, F4 moneda; `-service N` abre directo la pantalla N. COIN SETTING ahora controla g_cpc/g_freeplay.

## Actualización 20 - músicas de pantalla [observado]
- FUN_00028442 (inicio de la intro 01.DAT): FUN_294B9 (detiene el audio); si hay crédito -> título; si no, si DAT_006e1918 (DEMO SOUND) != 0: FUN_00029263(EAX=1) + FUN_0002949F = AUDIO\001.AUD.
- FUN_0002BB5E (entrada de la selección / elección de modo, tras PRESS CENTER STEP): FUN_294B9 y AUDIO\002.AUD incondicional (EAX=2), carga BGA 099 (strings 0x1205F9..), pone a cero DAT_006E17A4/1778, 1788=-1, 1798=179C=1.
- FUN_00028843 (ayuda 03.DAT, estado 0x17): AUDIO\003.AUD (EAX=3).
- game.c: la intro toca 001 si DEMO SOUND (valor inicial por defecto encendido: inferido, el DEFAULT SETTING literal del código pone 0); la selección toca 002; los demás estados llaman a music_stop.

## Actualización 21 - pantalla de elección de MODO (estado 0xB fase 2) reconstruida [observado]
- Init FUN_0002BB5E: FUN_294B9, BGA 099, FUN_2B609(1) carga las 14 hojas BAR01..14 (90.DAT) en la tabla 0x12C3A1 (14 filas x 40 bytes: [0]=textura, [1..8]=ids de las 8 barras, [9]=índice), AUDIO\002.AUD. Lista = STAGE.CFG "MODE MENU": ids 2,3,4,5,6 (DOUBLE, NORMAL, HARD, NONSTOP REMIX, CRAZY); con 2 jugadores el ítem 0 pasa a ser el id 1 (BATTLE) (FUN_2BC14: DAT_006E0EE8 hi16 = 1). Barras de BAR14: 516,517,1..6 = Slam, Space Fantasy, BATTLE, DOUBLE, NORMAL, HARD, NONSTOP REMIX, CRAZY.
- Bucle FUN_2BD93 por cuadro (60 Hz): tiempo = 30 - FUN_2057F/0xF0; (1) capa de resalte del modo (DAT_6E1778 0..6 -> capas 0,3,1,2,4,6,5) en el cuadro 600+f%90 (FUN_2647A = 1 capa; FUN_2643C = 50); (2) BGA entero en el cuadro 30; (3) número del tiempo en (584,429) amarillo (FUN_2A672); (4) flechas: capas 0x1B/0x1C y 0x1E/0x1F en el cuadro 90+anim; (5) translate(local) con local = anim*3-66 (dir 2) o -anim*3+66 (dir 3) durante 22 cuadros; FUN_2B192 dibuja las barras en (213,218)+66*i; FUN_2B459 pulsa el resalte en (292|358,319); BGA en el cuadro 150 desplazado en -(cur-1)*66; (6) capa 0x2E/0x2F en el cuadro f%30.
- Barras (FUN_2AE66/2AF37): glRotatef(45), quad 194x32 (x 30..224) con u 0.113..0.875, v = barra*0.125; resalte centrado (-97,-16). Estas rutinas usan y HACIA ARRIBA (480-y) - el BGA no.
- Pulso: p=(anim/2)%50; copias escala p/150+1 alfa 1-p/30 (p<60), p/100+1 y 1-p/20 (p<40), p/50+1 y 1-p/10 (p<20).
- Entrada (DAT_00153718): bits 1/2 = desplaza izquierda/derecha (dir 2/3; cur-- / cur++, cur 0..3, solo con anim>3; límite -> anim=22), bits 8/0x10 = elige ítem cur+0 / cur+1 (dir 0/1). Confirma al pulsar el MISMO botón 2 veces (prev==dir) o con tiempo < 1 s; ítem (dir+cur) -> modo 4|5 (Double/Battle), 1, 2, 3, 6 -> DAT_0012B028 |= 0x80|0x40, 0x10, 0x20, 0x100, 0x400; próxima fase 3 (lista de músicas).
- No leídos: FUN_2A672 (número del tiempo), FUN_2B958, FUN_21915, FUN_20134 (HUD de jugadores/créditos). El mapeo teclas->bits es inferido (Q/Left, E/Right, Z/Up, C/Down, Enter = confirma).
- game.c: S_MODE (mode_enter/mode_step/draw_mode, bga_draw_n); `-mode 1|2` abre la pantalla con 1 o 2 jugadores.

## Actualización 22 - teclas y bits del pad (ELF Linux `piu`) [observado]
- ELF: player1keys @0x081134E0 = Q,E,S,Z,C (GLFW 0x51,0x45,0x53,0x5A,0x43) -> bits 0..4 del pad (io_input_kbd_p1: `esi ^= 1<<i`); player2keys @0x081134CC = KP7,KP9,KP5,KP1,KP3 (0x147,0x149,0x145,0x141,0x143). serviceKeys G,H,J (+K, L en P2) activan bits de service/coin (0x200, 0x4000, 0x8000, 0x400).
- CORRECCIÓN: la tabla de teclas del ELF es del PORT, no de los bits del DOS (no sirve para inferir el orden de los bits). Por la observación del usuario en la pantalla de modo: bits 1/2 = abajo-izq/abajo-der (Z/C) desplazan la lista (dir 2/3); bits 8/0x10 = arriba-izq/arriba-der (Q/E) seleccionan cur+0/cur+1 (dir 0/1) y repetir el mismo confirma. Orden de los bits del pad en el DOS: 1=DL 2=DR 4=centro 8=UL 0x10=UR (inferido). El centro (S) no se comprueba en esta pantalla.

## Actualización 23 - interpolación de capas BGA: el pivote NO se interpola [observado]
- FUN_00025FA5 (0x260D9..0x2624F): entre las keys i-1 e i interpola solo x, y, escala, rotación y color r,g,b,a (FUN_25F88 = lerp); el pivote (px,py, +0xC/+0x10) viene de la key anterior, sin interpolar. Matriz: translate(0,480), translate(x,-y), translate(px,-py), rotate, scale, translate(-px,py), translate(0,-480) = y hacia arriba, equivalente al y hacia abajo usado en game.c.
- Efecto: el anillo de zoom/fade de las burbujas SHIFT (099.BGA capas 0x1C y 0x1F, m09_1/m09_2.SPR, 1,0 -> 1,2 con alfa 1 -> 0, pivote (135,392)) salía de lugar porque se interpolaba el pivote de la última key (0,0, inactiva).
- piu_data.c: piu_bga_eval pasa a usar el pivote de la key anterior.

## Actualización 24 - fase 3: animación del modo elegido (097.DAT) y fase 4/5 [observado]
- FUN_0002BB5E carga el BGA 097 en el objeto 0x2FF58C. FUN_0002C4AB (fase 3, 30 cuadros): FUN_2643C(0x2FF58C, cuadro = DAT_006E17A4 + tabla[DAT_006E1778]) con tabla @0x12CB25 = {0,0,120,360,240,480,600} (modo 1 NORMAL 0, 2 HARD 120, 4 DOUBLE 240, 3 REMIX 360, 5 BATTLE 480, 6 CRAZY 600; la 097.BGA tiene 6 tramos de 120 cuadros: nzb/hdb/dbb/nrb/btb/czb). Al llegar a 30: FUN_294B9 (detiene la música 002) y DAT_006E1798 = 4.
- FUN_0002C63F (fase 4): carga el BGA 098 (objeto 0x15C95C) y el fondo de color del modo en el objeto 0x2FF58C: DAT_006E1778 -> 0:091, 1:091 (NORMAL rosa), 2:092 (HARD naranja), 3:094 (REMIX azul), 4:093 (DOUBLE verde), 5:095 (BATTLE rojo), 6:096 (CRAZY morado); FUN_2B609(0), FUN_2B70B (lista por el grupo DAT_006E1784 según el modo), DAT_006E1798 = 5.
- game.c: S_MCONF (mconf_step/draw_mconf) y, después, S_SELECT con 098 + fondo 09x bajo la lista antigua; `-conf N` abre la animación del modo N.

## Actualización 25 - fase 5: rueda de músicas, FUN_0002C840 [observado; tramos marcados como inferidos]
- Misma máquina que la elección de modo (DAT_006E1788 = dir: 0/1 apuntan al cartel izquierdo/derecho, 2/3 desplazan; DAT_006E17A4 = anim; DAT_006E1794 = dir anterior; DAT_006E179C = posición; DAT_006E178C = tamaño de la lista). Entrada aceptada con anim > 3 (bits 8/0x10 -> dir 0/1, 1/2 -> dir 2/3). Desplazar: anim==0 -> pos -= 2 / += 2 (circular, FUN_2B09D). Confirma: el mismo puntero dos veces (prev == dir < 2) o 30 s (FUN_2057F/0xF0): DAT_006E17A0 = lado, FUN_30FAC(0, 4, 3), DAT_006E1798 = 7.
- Dibujo: fondo 09x (objeto 0x2FF58C) girando cnt*0.06 grados alrededor de (320,240), cuadro (cnt*2)%90; 098 (0x15C95C) en el cuadro 0; flechas SHIFT capas 0x1B/0x1C (dir 2) y 0x1E/0x1F (dir 3) en el cuadro 90 + anim (anim < 90); rueda FUN_2B0DD: translate (213,218), 8 ítems +66 y 8 ítems -66, desplazamiento local anim*6-0x84 (dir 2) / -anim*6+0x84 (dir 3) durante 22 cuadros; cada ítem es una barra BARxx de la 90.DAT (FUN_2AFFF, misma tabla 0x12C3A1 de la elección de modo, ids de música 101..517).
- Con dir 0/1: capas 0x23/0x2B/0x2C de la 098 en el cuadro 0x96, dos carteles (FUN_2B266(0,0), (1,0), (dir,anim)) y BPM "%5.1f" en (0x128,0x63) (FUN_2C5CA, tabla 0x6DFC28: id -> float).
- Cartel (FUN_2AE17/FUN_2AD1B): tabla 0x12C5D1, 30 filas de 40 bytes (handle + 4 ids): fila r = hoja CD(r+1).PNG de la 90.DAT (CD01..CD26; 256x256, 2x2 recortes de 128x96: u = (j%2)*0.5, v = (j/2)*0.375). Dibujado 200x142 centrado en (161,320) / (481,320) con y hacia arriba; pulso como en las burbujas: p = (anim/2)%50, escala p/S0+1, alfa 1-p/S1 mientras p < S2, S = {150,30,60},{100,20,40},{50,10,20} (0x1205B3..).
- Lista (FUN_2B70B): entradas de 4 bytes en 0x6DF6B0 (byte0 = válido, byte1 = nivel, hi16 = id), llenadas por FUN_2A039 a partir de un archivo de texto (no localizado) y, sin limitación (DAT_0012B024==0), FUN_2B695 marca todas como válidas. Por modo (DAT_006E1784) solo entran las barras [DAT_006E1636, +DAT_006E1638) de la tabla.
- No reconstruido: el valor de LEVEL (byte1 de cada entrada, "%d" en (0x130,0x46) vía FUN_26658), la lista por grupo/modo (aquí entran todas las músicas del STAGE.CFG), pre-escucha de la música (FUN_29295/FUN_2949F tras 30 cuadros apuntando), FUN_2A672 (número del tiempo), FUN_2B958, FUN_21915, FUN_20134 y la fase 7 (elección de dificultad).
- game.c: S_WHEEL (wheel_enter/wheel_step/draw_wheel, CD_TBL[26][4]); `-wheel MODO [-wdir 0|1|2|3]` abre directo; después de confirmar cae en la lista antigua (dificultad) con la música elegida.

## Actualización 26 - listas por modo (STAGE.CFG) y posición del PRESS CENTER STEP [observado / inferido donde se indica]
- FUN_0002C63F: DAT_006E1784 (sección del STAGE.CFG, posicional) = 0 EASY si flag 0x10 (NORMAL), 1 HARD si 0x20, 2 DOUBLE si 0x80, 3 REMIX si 0x100, 4 REMIXDB si 0x200, 6 REMIXCZ si 0x800, 5 CRAZY si 0x400; BATTLE (0x40) depende de DAT_0012B024 (0 EASY, 1 HARD, 2 REMIX). FUN_2B70B copia las entradas de la sección (rango [DAT_006E1636 + g*4, cuenta DAT_006E1638 + g*4]) a la lista de la rueda.
- Línea "S nivel id" = entrada de 4 bytes: byte1 = nivel (impreso "%d" en (0x130,0x46) en la rueda), hi16 = id. Las líneas en blanco separan páginas por serie (solo agrupación).
- Inferido: no encontré quién activa las flags 0x200 (REMIXDB) y 0x800 (REMIXCZ). Por los datos REMIXDB = REMIX jugado en double (solo 119 y las demás hasta 415) y REMIXCZ = REMIX jugado en crazy (225, 327, 330, 415). En la reconstrucción, REMIX usa siempre la sección REMIX; DB/CZ aún no se seleccionan.
- FUN_00020134 (estado 0x14): FUN_21885(x, y=0x16 hacia arriba, sprite): PRESS CENTER STEP = sprite 0x25 en x=0xE (P1) / 0x198 (P2); INSERT COIN = sprite 0x24 en x=0x30 / 0x1A8. game.c ajustado.

## Actualización 27 - REMIXDB / REMIXCZ: códigos secretos (hallados en el ELF Linux) y fases reales [observado]
- Numeración real de las fases del estado 0xB (despachador `jump_2d0dd` del ELF, tabla 0x8059B3B; DAT_006E1798): 0 FUN_2BB5E, 1 FUN_2BC14, 2 FUN_2BD93 (modo), 3 FUN_2C4AB (097), 4 FUN_2C63F (carga 098/fondo/lista), 5 FUN_2C81C (solo pone fase=6 y pone a cero el tiempo), 6 FUN_2C840 (**rueda de músicas**), 7 siguiente. (La Actualización 25 llamaba a la rueda "fase 5".)
- FUN_2B958 (tramo 0x2ACDA en el ELF) en cada toque del pad llama a FUN_2A72B (`func_2a72b`): memcmp de las 8 últimas teclas (historial DAT en 0x86D6CC2) con 15 secuencias de 8 bytes (tabla 0x81219AA; bits 1 DL, 2 DR, 4 centro, 8 UL, 0x10 UR), despacho por tabla 0x8057331. Se ignora si la flag BATTLE (0x40) está activa.
- Con la flag REMIX (0x100): `DL UL DL C UR DR UR C` (func_2a88a) y (flag&3)!=3 (solo 1 jugador): flag &= 0xF00F, |= 0x200 (REMIXDB), grupo (DAT_006E1784) = 4, rehace la lista (FUN_2B70B). `C DL C DR C DL C DR` (func_2a8c4) y fase == 6: flag |= 0x800 (REMIXCZ), grupo = 6, rehace la lista. En teclas (Z=DL, C=DR, Q=UL, E=UR, S=centro): REMIXDB = Z Q Z S E C E S; REMIXCZ = S Z S C S Z S C.
- Otros códigos de la misma tabla (no implementados): `C x8` limpia flags (&0xFFF); varios alternan bits de opción de juego (DAT 0x83CAEAA: 0x10, 0x20, 0x8, 0x4, 0x2, 0x40, 0x80, 0x100/0x200) y bits 0x1000/0x2000/0x4000 de la flag de modo.
- game.c: `wheel_code` (historial de 8 toques, solo con modo REMIX) cambia la sección y rehace la lista.

## Actualización 28 - previa de la música en la rueda (empírico)
- Al apuntar a un cartel en la rueda de músicas (UL/UR = Q/E, numérico 7/9) toca la previa `AUDIO\Dxxx.AUD` de la música apuntada (xxx = id del STAGE.CFG); repetir el mismo lado (confirma) no la reinicia; salir de la rueda a la selección detiene la previa. **Inferido/empírico**: la rutina original que dispara la previa aún no se ha localizado en PIU.EXE (no confirmado si desplazar también cambia la previa).
- game.c: `music_load_pfx("D", id)` en `S_WHEEL`.

## Actualización 29 - fase 7: de la rueda directo al gameplay
- Confirmado (ELF func_2cdfa = FUN_2CDFA, fase 7 de 0xB): toma la música (lista DAT_006E1782 + índice, FUN_2B09D), detiene la música (FUN_294B9), arma el nombre del paso por flags de modo (0x81206d2 / DOS DAT_0012B028) con `%d_E1/H1/H3/E2/H2/H4/DB.STF` y llama a FUN_2058F ("run %s %s %d"). No hay pantalla de selección de dificultad: el **nivel** es el modo elegido antes (EASY/HARD/REMIX/DOUBLE/CRAZY), la rueda lista solo las músicas de la sección, y confirmar entra al gameplay.
- Regla: DOUBLE/REMIXDB -> DB; 1P: HARD/REMIX -> H1, CRAZY/REMIXCZ -> H3, si no E1; 2P: HARD/REMIX -> H2, CRAZY/REMIXCZ -> H4, si no E2. Rama de flag 0x4000 (batalla, %d_H1/E1 + "-S") no tratada.
- game.c: `wheel_step` llama a `start_song(di)`; la pantalla antigua S_SELECT solo sirve de fallback si falta el archivo.

- Pantalla de carga (FUN_2A533, ELF func_2a533): antes de iniciar, sprintf "T%03d" de la música, carga TITLE\T%03d.PNZ y la dibuja a pantalla completa (ac_game_draw_title); después FUN_2058F inicia el juego. game.c: estado S_LOAD (`load_step`/`draw_load`); duración ~1,5 s y **empírica** (en el original es el tiempo de carga).

## Actualización 30 - cuadro de gameplay (ELF func_24de6 = FUN_24DE6): mapa de dibujo (parcial)
Confirmado en el ELF (aún no verificado en PIU.EXE; direcciones `func_XXXXX` = VA del DOS). Orden de dibujo de un cuadro:
1. Fondo: `func_2643c(0x8152006, frame)` (frame = tiempo>>2; 50 llamadas de `func_25fa5`, capas del BGA) - salvo que la flag 0x40 (batalla) esté en la rama de "parpadeo" (fondo lleno rojo/azul alternando cada 3 cuadros).
2. Campo(s) por jugador - `func_22674` (P1) / `func_226ad` (P2), o `func_226f9` (double, dos mitades): `glTranslatef(x, 378, 0)` con **x = 28 (P1), 348 (P2); double: 66 y 311** (y de abajo hacia arriba: línea de los receptores a 378 = 102 desde arriba). Cada campo llama a `func_221a4` (5 receptores en x = 35,83,131,179,227 e y = 35 relativos, 48 px entre columnas, escala de pulso de la tabla 0x81206EA indexada por el estado 0x814AFCE.., sprites en 0x83C7226/0x83C907E/0x83C62FA/0x83C8152/0x83C9FAA con paso de 60 bytes = objetos de sprite dibujados por `func_2EE67`), `func_22386` y `func_22502` (otras capas del campo). `func_22dcd/22df1/22e20` -> `func_22840` (flechas/notas, 498 líneas) dibujan las notas cuando el jugador no está en modo "borrado" (bit 0x80).
3. Juicio: `func_22fd8(jugador)` - contador por jugador (0x83CAED8 + 0x29*p) decrementa, `glTranslatef(0, 240, ...)`, escala de 'aparecer' y sprite del juicio (0x37 '7' y superiores).
4. Barras de vida/puntuación: `func_21e47(vida, 7)` en (176|573, 16) y `func_21885` en x=14 (P1) / x=560 (P2), y=52, tam 40 (solo si no es batalla y sin flag 0x1000).
5. `func_23a4b(jugador)` (¿combo?), `func_21915`, `func_20134` (HUD/contador) y, si DAT 0x8148DD6, línea de debug de tiempo (`func_26658`, "%d:%02d:%d").
Estado del prototipo (draw_play): usa **xs=200** para 1 jugador (incorrecto: P1 queda en x=28, P2 en 348), receptores/flechas en coordenadas propias y HUD de texto; falta leer `func_2EE67` (formato del sprite), las hojas de sprites (¿00.DAT/01.DAT/03.DAT?) y los dibujos de `func_22386/22502/22840/21885/21e47/23a4b/21915/20134`. **Próximo paso**: leer el formato de sprite y el cargador de las hojas de gameplay.

## Actualización 31 - campo de gameplay con sprites SP2 y fin de la pantalla antigua de selección
- Confirmado (ELF func_21fe1/func_221a4/func_22386/func_22502/func_22840): los sprites del campo son `BGA/00.DAT` ARROW541..545.SP2 (tiles b1/b2 receptor, n1..n6 nota, f1..f8 acierto, p1..p8 toque), uno por dirección: DL=542, UL=541, centro=545, UR=543, DR=544. Receptor b1 durante 3/4 del tiempo y b2 en el último cuarto de cada tiempo; escala de pulso al tocar (tabla 0x81206EA: 1,.97,.94,.91,.88,.85,.72,.69); p1..p8 en 16 cuadros, f1..f8 en 24; nota n1..n6 = 6 cuadros por tiempo. Campo en x0=28 (P1) / 348 (P2), double 66 y 311; receptor en (x0+35+48k, 67 desde arriba); las notas suben desde abajo.
- Inferido: qué juicios disparan los tiles f (aquí PERFECT/GREAT/GOOD); y de la línea de receptores (67) deducido de translate 378+35 en y-hacia-arriba.
- La pantalla antigua S_SELECT (lista + nivel) fue desactivada: ya no hay fallback ni retorno del resultado a ella (vuelve al attract); solo queda accesible por `-skip` de depuración.

## Actualización 32 - HUD de gameplay: vida, score, combo, juicio (ELF func_23a4b, func_21e47/21885, func_22fd8, func_21d11)
Confirmado en el ELF e igual en el DOS (FUN_22FD8 tiene los mismos desvíos). Coordenadas en y-hacia-arriba; jugador P1 `0x83CAEB2`, P2 +0x29 por jugador (struct: +4 vida float, +8 vida entera 0..1000, +0x14 combo, +0x18 racha de MISS, +0x26 último juicio, +0x24 temporizador).
- **Vida** (`func_23a4b`): marco `gauge_b` (ARROW542, dec00.tga) en (32|352, 445); v=vida/1000, n=(int)(v*256), q=n/6; el relleno `gauge__` (misma textura, v 32..64) es el rectángulo x en [253-6q, 256] (P1) / [0, 6q+2] (P2) del marco. Peligro: vida<180 enciende `gauge_D` (event.tga) amarillo cada 2 cuadros (en el original depende de la opción 0x86D6FC0; aquí siempre). El original parpadea el relleno en cuadros alternos (no reproducido: **inferido** que no importa).
- **Score** (`func_21e47(valor,7)` en (176|573, 16)): 7 dígitos de font.tga (celdas 32x39, v0=0.53125+(d/8)*0.15234375, u=(d%8)*0.125), de atrás hacia adelante, paso -24; rótulo `score_` (ARROW541) en (14|560, 52).
- **Juicio** (`func_22fd8`): 25 cuadros; centro (160|480|320 en double, 240); escala = tabla 0x812071E[25-t] para t>10 (1.52..1.0), si no 1; tile = perfec/great_/good__/bad___/miss__ (ARROW542) por el último juicio '5'..'9' ('A' = attack_, batalla).
- **Combo** (`func_21d11`): dentro de la misma escala, 70 px debajo; 3 dígitos de event.tga (32x48, v0=.375+(d/8)*.1875) con ceros a la izquierda, paso -26 desde +10; solo con combo>=4 (blanco). Luego la racha de MISS (>=4) en rojo (1,0.2,0.2) (corregido: había leído al revés el orden de los argumentos del glColor3f), continuando a la izquierda (la traslación no se deshace). El tile `combo_` existe en el SP2 pero **no se usa** en el ELF ni en el DOS durante el juego (solo los dígitos).
- NUM del ARROW542.SP2 dice 43 pero hay 45 tiles; `spr_parse` ahora cuenta las líneas T. `sp2_find` compara solo 6 letras (gauge_b/__/D colisionan): usar `sp2_find_x`.
- game.c: bloque HUD de `draw_play` (dibujo en y-hacia-arriba con `tile_up`). No tratado: iconos de opción (func_21915), tiempo/crédito (func_20134), modo batalla, 2 jugadores simultáneos.

## Actualización 33 - sprites PATTERN del BGA (ELF func_2efad tipo 3, func_2f4ef)
- Confirmado: `TYPE pattern a b c` (FUN_2F4EF): a -> [obj+0x24], b -> hi16 de [obj+0x28], c -> byte bajo. En el dibujo (tipo 3) la rejilla tiene **c columnas x b filas**, paso = (x+w, y+h) del tile 0 (translate x*columna, fila*altura hacia abajo); índice del tile = `(a != 0 ? fila*c + columna : b*columna + fila) + (int)(n*(1-frac))` módulo n. Antes solo se dibujaba un tile (la Mobius Strip, 213, muestra 2x2 tiles de 320x240 idénticos).
- Tipo 1 (TILE): los tiles se dibujan del último al primero (orden invertido, confirmado en el bucle que decrementa).
- game.c: `Spr.pb/pc/a`, `bga_draw_n`.

- Orden de dibujo de los receptores (func_221a4, confirmado): DL(x35), centro(x131), DR(x227), UR(x179), UL(x83), en tres pasadas (b1/b2, p, f). Como son sprites separados, Z/S/C quedan detrás y Q/E delante. game.c: `ORD` en `draw_play` (las notas usan el mismo orden; inferido).

## Actualización 34 - couple (2 jugadores) y sincronía del audio
- Dato (STEP/*.NOT, máscara bit15->col0 ... bit6->col9): `X1` (E1/H1/H3) solo tiene las columnas 0-4; `X2` (E2/H2/H4) tiene **las dos mitades**: con 2 jugadores el ELF carga **un único archivo** (`%d_H2.STF`, func_2cdfa) y P1 juega las columnas 0-4 y P2 las 5-9 (en 101 la mitad izquierda de H2 = H1). No son dos archivos. 1 jugador (P1 o P2) -> X1; 2 jugadores -> X2; CRAZY: H3 (1P) / H4 (2P); DOUBLE (0x280): DB.
- game.c: `g_two` (couple) separado de `g_dbl` (DB). Couple usa dos campos (x0 = 28 y 348), dos `PiuPlayer` (`g_ps[2]`), vida/score/juicio por jugador; P1 = Z Q S E C, P2 = numérico; termina cuando los dos llegan a cero. Antes H2 se trataba como double (campo en 66/311, un solo jugador).
- Sincronía: `cb_pos` es la posición ya entregada al dispositivo; la posición oída es menor. `music_pos` ahora resta 2 bloques de audio (`g_alat_ms`, ~46 ms) y el ajuste manual `-aoff MS` (positivo atrasa las flechas).

## Actualización 35 - ULTRAMANIA (514) y vida del P2
- Dato: `514_H1`, `403_H1` y `505_H3` (archivos de 1 jugador) traen notas en las columnas 5-9 (en 514, H1 = H2, las dos mitades). El original decide single/double/couple **solo por las flags de modo**; con 1 jugador solo se juegan las columnas 0-4. La heurística antigua (cualquier nota en 5-9 activa el double) fue eliminada y las columnas 5-9 solo entran en DB (double) y en el couple.
- Vida del P2: espejo del P1 (rango [0, 6q+3] con la textura invertida en x; rojo en el borde de adentro, cerca del centro).
- `start_song` ahora escribe una línea de log (stderr): música, paso, sflag, modo, sección.

- Vida parpadeando (func_23a4b, confirmado): el relleno `gauge__` solo se dibuja en los cuadros en que `0x83C627A % 2 != 0` (contador de cuadros a 60 Hz), la alerta `gauge_D` en los pares (con vida<180). game.c: `fr = now*60/1000` en el bloque HUD. Los 60 Hz se infieren del contador por cuadro del original.

## Actualización 36 - pantalla de título: créditos / FREE PLAY y PRESS CENTER STEP (ELF func_20134)
- Confirmado: el título dibuja tiles de SP2 (no un recorte de la FONT, que incluía el "(" de al lado): FREE PLAY = tile `freeply` (ARROW542) en (320, 13); con créditos: `credit` (ARROW541) en (263,22) + número en x=360 (y=25); con moneda parcial (coins % monedas_por_crédito != 0): `credit` en 239, créditos en 338, `left__` "(" en (354,21), resto en 358, `slash_` en (372,21), monedas por crédito en 388 (>=10) o 378, `right_` ")" en 400/390. Números = `func_200e3`: dígitos de FONT 16x16 (u=(d%16)/16, v=96+(d/16)*16), paso -12, de derecha a izquierda. PRESS CENTER STEP = `press_` en x=14 / 408, INSERT COIN = `insert` en 48 / 424 (y=22), alfa parpadeante.
- game.c: `draw_title_overlay` / `title_digits`.

## Actualización 37 - textos de los jugadores en el MODE SELECT (ELF func_2b958, llamada por func_2bd93)
- Confirmado: son capas del 099.BGA (mt01..mt08, MT10/MT11) dibujadas por `func_2647a(obj, capa, cuadro)` con cuadro = base + (anim % 30). Tablas 0x81221EB.. (capas 13,14,12,11,19,17,18,16,15,20) y 0x8122213.. (bases 360,420,300,240,510). P1: ya entró -> capa 13 base 360 (apuntando: 14 base 420); no entró con crédito -> 12 base 300 ("PRESS CENTER STEP TO JOIN"); sin crédito -> 11 base 240 ("NOT READY! PLEASE INSERT COIN(S)"). P2: 17 (18 apuntando) / 16 / 15. Rama double (0x280): capas 19/20 base 510 ("PLEASE WAIT") - no tratada. El número del TIEMPO (func_2a672, x=584 y=429) aún es texto provisional (los dígitos amarillos están en la 03.PNG del 099.DAT).
- game.c: bloque nuevo en `draw_mode`.

## Actualización 38 - BPM/LEVEL y TIME con la fuente original (func_26658/26521, func_2a672/2a590)
- Texto (func_26658(x, y, fmt, ...)): glifos de FONT.PNG con celdas **8x15** (32 por fila; u=(c-0x20)%32*8, v=(c-0x20)/32*15), quad de 8x15 con **y hacia arriba** (la base queda en y, el tope en y+15), paso 8, el espacio no dibuja. BPM `%5.1f` en (0x128, 0x63), nivel `%d` en (0x130, 0x46) (inferido). Antes usábamos la fuente 8x8 con la base en 480-y, que quedaba baja.
- TIME (func_2a672(584, 429, n)): 2 dígitos de **02.TGA del propio BGA (099/098)**, celdas 48x40, 5 por fila, v0 = 162 + (d/5)*40, quad 48x40 en (584 - 40*i, 429) y hacia arriba, ceros a la izquierda.
- game.c: `orig_text`, `orig_time`.

## Actualización 39 - fin de la música: STAGE CLEARED / FAILED / BREAK, DANCE GRADE y GAME OVER (83.DAT) - ELF func_24de6, func_255d9, func_25430..25570, func_257cf, func_2568a
Confirmado en el ELF (mismos desvíos en el DOS). El cuadro de gameplay `func_24de6` define la **fase** `0x83C62A6` y `func_255d9` (tabla 0x8052624) ejecuta cada una tocando cuadros del `83.BGA` con el contador de cuadros `0x83C627A` (60 Hz):
- 0 jugando; **1 FAILED**: cuadro c (hasta 180) -> fase 3; **2 CLEARED**: cuadro 360+c (hasta 240) -> estado 5 (resultado); **3 GAME OVER**: cuadro 2280+c (hasta 120) -> attract (pone flags a cero); **4 STAGE BREAK**: cuadro 1800+c (hasta 240) -> fase 3.
- STAGE BREAK (24F50/24F9A): sfx idx 7 canal 7 cuando **todos** los jugadores activos tienen vida < 1 (con STAGE BREAK activado, ~7 MISS seguidos) o racha de MISS > 50 (51 MISS, también desactivado). Fin de la música (2505A): STAGE BREAK activado -> CLEARED si la vida de algún jugador >= 0.6, si no FAILED; desactivado -> CLEARED (**inferido**). Opción = `0x81206B6` (= STAGE BREAK del GAME OPTION, 0 = OFF).
- Resultado (`func_257cf`, cuadro 600 + c%480): solo aparecen con c > 10 score (7 dígitos, x=160|588, y=16), c>30/35/40/45/50/55 PERFECT/GREAT/GOOD/BAD/MISS/MAX COMBO (3 dígitos en x=176|488, y=343,293,243,193,143,78; dígitos de event.tga 24x40, v0=192, paso -22), c>70 la nota (tile grade_s si vida>=1.0 y 0 MISS, si no A >=0.9, B >=0.8, C >=0.7, D >=0.6, F; en (118|522, 228), alfa = fade*0.5; fade = 1 hasta c=380 y después (400-c)/20). **SFX**: idx 8 canal 1 en c=30,35,...,55 (uno por número); idx 9 canal 2 en c=70 (nota); en c=80 la voz de la nota idx 0x17..0x1B (RANK_A..F, canal 6) por la vida (>=0.9/0.8/0.7/0.6). Sale con c>400 o (c>250 y botón) -> `func_2568a`.
- Próxima etapa (`func_2568a`): etapa++ ; EASY/HARD/CRAZY: continúa si <3, o ==3 con las 3 vidas >= 0.9 (extra stage); DOUBLE: ==3 exige la 1ª vida >= 1.0; BATALLA <3; REMIX/REMIXDB/REMIXCZ <2. Continúa -> rueda de músicas (estado 0xA); si no GAME OVER.
- No tratado: lámparas (func_27bb6), música del resultado (func_27b2a, no identificada), modo batalla (ATTACK), crédito/tiempo en el resultado. game.c: `end_start/end_step/draw_end`, estado `S_END`; prueba: `-end FASE -endc CUADRO`.

## Actualización 40 — Códigos (modifiers) en las pantallas de modo y de músicas

**Confirmado (ELF `piu`, `func_2a72b` P1 / `func_2a973` P2, tabla `0x81219AA`, 16 × 8 teclas; DL=1 DR=2 C=4 UL=8 UR=0x10):** las 8 últimas teclas de cada jugador se comparan con la tabla; al hallar la secuencia, el historial se pone a cero (`func_2a6c9`) y, si alguna flag cambió, suena un beep (`ac_sound_play_effect(0,2,1)`). La tabla coincide con la lista de códigos del usuario (numérico: 7=UL 9=UR 1=DL 3=DR 5=centro):

| # | Código | Efecto en las flags |
|---|---|---|
| 0 | 55555555 | pone a cero las flags de modo altas y los modificadores (cancelar) |
| 1 | 55975315 | VANISH (0x10, limpia 0x100) |
| 2 | 55159357 | MIRROR (0x20) |
| 3 / 4 / 5 | 55153555 / 55355755 / 55755595 | 8X / 4X / 2X (0x08 / 0x04 / 0x02, exclusivos) |
| 6 | 51535957 | RANDOM VELOCITY (0x40) |
| 7 | 79579579 | NONSTEP (0x80) |
| 8 / 9 | 55797955 / 55979755 | flags de modo 0x1000 / 0x2000 (exclusivas) |
| 10 | 13573159 | flag de modo 0x4000 |
| 11 / 12 | 79513579 / 79351531 | RANDOM VANISH (0x100) / RANDOM STEP (0x200) |
| 13 | 17159395 | REMIXDB (solo REMIX, 1 jugador, en la rueda) |
| 14 | 51535153 | REMIXCZ (solo REMIX, fase 6) |
| 15 | 95197537 | existe en la tabla pero no se despacha en esa función |

Entrada en las pantallas MODE SELECT y en la rueda de músicas; letras = P1, teclado numérico = P2 (solo quien entró). Los modificadores persisten durante el crédito y se ponen a cero al final del juego / al volver al título.

**Aplicado en juego:** velocidad 2X/4X/8X (ajusta `g_speed`) y MIRROR (espeja columnas por mitad; double espeja las 10). **Aún no aplicados** (solo almacenados): VANISH, RANDOM VELOCITY/VANISH/STEP, NONSTEP, SYNCHRO/COUPLE/UNION. **Inferido:** índice/canal del beep (sfx 1, canal 2) y los nombres de los bits.

## Actualización 41 — Iconos de los modificadores

**Confirmado (ELF `func_21915`, llamada por `func_2be94`, `func_2c840` y `func_24de6`, es decir, MODE SELECT, rueda de músicas y gameplay; `func_21885` = translate(x,y) + sprite + translate de vuelta):** los iconos vienen de `ARROW541.SP2` (el índice del original es el tile − 1), x = 4 (P1) / 600 (P2), y hacia arriba: speed 2X/4X/8X `accel_` (y 240) o random step `raccel` (240); vanish (200) o random vanish `v_rand` (200); mirror (160); random velocity `random` (120); nonstep `nonstp` (80). Flags de modo: `sync__` (0x1000) o `couple` (0x2000) en (4, 280), sin batalla. En double solo el jugador que entró muestra los iconos.
Prueba: `-mod 0x2f4` activa modificadores en P1/P2.

## Actualización 42 — Pantallas LOAD tras el DANCE GRADE y efectos de los modificadores

**Confirmado (ELF `func_2c63f`, 0x2C689..0x2C6A2):** al volver a la rueda de músicas, el juego muestra `TITLE\LOAD%d.PNZ` (`%d` = etapas jugadas, `DAT 0x81206CE`) por la misma rutina `func_2a533` de las pantallas T%03d; con 0 (viniendo del mode select) no hay pantalla LOAD. `func_2d3df` compara la fecha del DOS con 2000‑12‑27: después de ella, la etapa 1 usa LOAD4 en lugar de LOAD1 (LOAD1 es el anuncio "2000.12.27 Wed"). LOAD2 = Banya, LOAD3 = "TRY BONUS STAGE", LOAD4 = "TRY NEXT STAGE". Duración (~1,5 s) inferida.

**Modificadores en juego:** velocidad por defecto ahora 1X (2X/4X/8X = 2/4/8; las teclas 1–8 aún valen), MIRROR, RANDOM STEP (permutación de las columnas), NONSTEP (flechas ocultas), VANISH (las flechas desaparecen en la mitad superior de la pantalla), RANDOM VANISH, RANDOM VELOCITY (la velocidad oscila). Los cinco últimos son **inferidos** (solo los bits y los iconos están confirmados). SYNCHRO/COUPLE/UNION siguen solo almacenados.

**Limpieza (post‑Actualización 42):** eliminados los restos de prueba del inicio — la selección de músicas básica (`S_SELECT`/`draw_select`/`diffs_avail`, argumento `-skip`) y la pantalla de resultado básica (`S_RESULT`/`draw_result`). La pantalla LOAD solo aparece tras el DANCE GRADE (nunca del MODE SELECT a la rueda).

**Música al final de la fase (Actualización 43, confirmado en el ELF):** `func_24de6` solo llama a `func_294b9` (detener música) en las dos ramas del STAGE BREAK (fase 4). En FAILED/CLEARED/DANCE GRADE/GAME OVER la música de la propia canción **sigue sonando** hasta que acaba; solo `func_2c63f` (entrada de la rueda) y el attract la detienen. No existe archivo de música propio del resultado (solo 001/002/003.AUD, usados en título/modo/demo). Antes el código detenía la música al terminar la canción.

## Actualización 44 — Portadas (CD) de las músicas fuera de las 26 hojas: 112, 226, 329 y 330

**Confirmado (ELF `func_2b609` carga, `func_2ae17` dibuja; DOS igual, tabla de 30 filas × 40 bytes en ELF `0x8121C77`/DOS `piu_le.exe` 0xF11D1):** cada fila = puntero al nombre de la hoja, handle de la textura, 4 ids. El cargador recorre **30 filas** (`cmp edx,0x1e`), no 26. Las filas 26–29 tienen ids solo en posiciones específicas y **reutilizan hojas**: 26 → `CD26.TGA` slot 1 = **226** (Another Heart 2nd), 27 → `CD26` slot 1 = **112** (Another Heart), 28 → **`CD06`** slot 3 = **329** (Extravaganza 3rd step, misma imagen que 205), 29 → **`CD11`** slot 1 = **330** (Mix 2-7 3rd step). Las filas 0–25 usan `CD(n+1)`. No existen CD27–CD30 en la 90.DAT.

Causa del bug (reportado en juego: "Extravaganza 3rd" sin DISC): `CD_TBL` de `game.c` tenía solo 26 filas y el nombre era siempre `CD%02d` de la fila. Corrección: `CD_TBL[30]` + `CD_SHEET[30]`. Observación: 112/226 caen en el slot 1 de CD26, que (por inferencia) puede ser una imagen vacía/otra; el juego original dibuja lo que tenga la hoja.

**Músicas "ocultas" (resumen del análisis):** no hay campo de ocultar; `func_2b70b` solo lista entradas de la sección con byte0 ≠ 0 (`func_2b695` pone todo en 1 en la 1ª etapa; `func_2b6bc(sección,id)` pone a cero la música jugada hasta el próximo crédito). Sin sección = oculta (408 "Kiss"); las `Mix` solo existen en REMIX/REMIXDB/REMIXCZ. **Implementado (game.c):** `SEnt.vis`; `wheel_list` solo lista `vis`; `wheel_begin` con `g_stage<1` reexhibe todo (func_2b695); al elegir la música (func_2cdfa) oculta el id en la sección actual (func_2b6bc). Inferido: el reset por crédito usa `g_stage=0` (GAME OVER/ESC/attract); el punto exacto donde el ELF pone a cero `0x81206CE` no se localizó. Prueba en juego pendiente.

## Actualización 45 — F1 (SERVICE) interrumpe todo y salir del servicio reinicia el juego

**Empírico (pedido del usuario; no leído en el código):** F1 fuera del servicio funciona como cerrar la ventana activa: `svc_halt` detiene la música, descarga los BGAs (`g_eb/g_ab/g_ab2`), pone a cero jugadores (`g_sflag`), etapa, modificadores y `g_att`, y va al estado 0 (attract); mientras el servicio está abierto el juego no avanza (sin `attract_step`/`play_update`). Al salir (ESC o ítem de salida, `svc_leave`) vuelve al estado 0 con `g_att=0`: sin crédito empieza por el **81.DAT**; con crédito/FREE PLAY el attract va directo al título (`cred_avail`). Antes el servicio solo dibujaba encima y el juego/música continuaban por debajo.

## Actualización 46 — Línea bajo el PRESS CENTER STEP (y otros tiles de sprite)

**Causa (verificada por captura ampliada):** `tile_up` muestreaba el recorte exacto del tile; con `GL_LINEAR` y la ventana escalada (ej. 800x600) el borde inferior tomaba el texel del sprite vecino de la hoja, formando una línea punteada gris bajo el texto. **Corrección:** el recorte usa medio texel hacia adentro (`+0.5/w`, `-0.5/h`) en `tile_up`; vale para todos los tiles de `ARROW541/542`, sin cambiar la posición del quad.

## Actualización 47 — Cuadro de los jugadores (MODE SELECT) sin uniones visibles; textos de los jugadores también en la rueda de músicas

**Uniones (verificado por captura + aislamiento de capa):** el cuadro es la capa 9 (`m05.spr`, relleno, alfa 0,5) + capa 10 (`m06.spr`, marco), cada una en 3 pedazos (tapa izq. + medio estirado 10→172 px + tapa der. espejada). En los datos las juntas se encuentran **exactamente** (la parte sólida de la tapa termina en u=170 → x=67, donde empieza el medio; la derecha igual en x=239). En 640x480 no hay unión; con la ventana escalada `GL_LINEAR` suaviza el borde duro de la tapa y 1 píxel queda cubierto por los dos pedazos (línea oscura). **Corrección:** la capa `m05.spr` se dibuja con `GL_NEAREST` (restaurado a `GL_LINEAR` después) y sin el retroceso de medio texel. `draw_tile` ganó el retroceso de medio texel (el mismo de la Actualización 46) en las demás capas. Resta, en el marco (`m06`), una columna de 1 píxel ligeramente más clara en la unión: las piezas se superponen 16/10 px por dibujo original (bordes antialias duplicados), así que es fiel al dato.

**Rueda de músicas:** `func_2c840` llama a `func_2b958` (textos de los jugadores) justo después del TIME, luego `func_21915` (iconos) y `func_20134` (créditos/FREE PLAY) — la misma secuencia del MODE SELECT (`func_2bd93`); la 098.BGA tiene las mismas capas 11–20. Antes la rueda dejaba los cuadros vacíos. Ahora `draw_player_texts(dbl)` (compartida) los dibuja; tablas del ELF 0x81221EB (capas 13,14,12,11,19,17,18,16,15,20) y 0x8122213 (bases 360,420,300,240,510). **Rama double (flags 0x280 = DOUBLE 0x80 / REMIXDB 0x200):** el jugador que no entró usa la capa 19 (P1) / 20 (P2) base 510 (texto "PLEASE WAIT!"). **Aún no hecho:** el overlay `func_20134` (FREE PLAY/créditos y PRESS CENTER STEP abajo) solo se dibuja en el título; en el ELF el MODE SELECT y la rueda también lo llaman.


## Actualización 48 — Build PC (`pumpy.exe`, 16/04/2001): empaquetador, desempaquetado y comprobación de la alfombra

**Confirmado (leído en el archivo):** `pumpy.exe` (PE32, 152 KB) está empaquetado: secciones todas llamadas `BitArts`, entry `0x7A9000` en un stub de varias etapas, solo 17 imports de KERNEL32 (el resto se resuelve en runtime). El código/datos reales solo existen en memoria.

**Desempaquetado (empírico, vía volcado):** el juego se abrió como administrador, quedó en la pantalla de error de la alfombra y se generó un `.DMP` con el Administrador de Tareas. `tools/dmp_extract.py arch.DMP pumpy.exe salida.exe` reconstruye la imagen (base `0x400000`, `0x7B2000` bytes) -> `E:\06_pc\pumpy_unpacked.exe` (704 procedimientos en Ghidra; **no es ejecutable**: IAT con direcciones del sistema del volcado, algunas entradas apuntan a shims de AcLayers).

**Comprobación de la alfombra ("PUMPY PAD"):** strings `Initialize PUMPY PAD...` (`0x42D144`, usada en `0x401679`) y `Checking PUMPY PAD...` (`0x42D1C4`, `0x401831`). `0x4016F0` crea una ventana de clase `KEY`, ejecuta un bucle de mensajes (hasta 11 intentos, `Sleep(100)`) y `0x401990` (WM_KEYDOWN/0x111) guarda hasta 21 teclas en `0x43275C`. `0x4015D0(jugador-1)` compara esas teclas (memcmp `0x429270`) con una secuencia esperada a partir de un desplazamiento de la tabla `0x42D068` (strings `0x42D12C` P1 / `0x42D114` P2); devuelve 1 si coincide. Fallo -> "Cannot detect your controller. try again" (`0x42D15C`); éxito -> "Successfully Detected Thank you for Play" (`0x42D188`). **Inferido:** la alfombra USB se anuncia como teclado y teclea esa secuencia (desafío-respuesta).

**Driver:** `gwiopm.sys` (tipo GiveIO; `0x42D6B4`, usos en `0x40274F/0x4027E0/0x40298A`) instalado vía SCM (advapi32) -> exige administrador (error 740 al abrir sin elevación).

**Diferencias de arquitectura vs Arcade:** renderizado OpenGL (opengl32; sin Glide), DirectInput/DirectSound/winmm, MP3 vía DirectShow (`RenderFile`, `AUDIO\%03d.MP3`, `AUDIO\D%03d.MP3`), `piukey.cfg`, carpetas `bga\`, `eff\`, `title\`. RES v3 (cabecera con `datasize` antes de `count`).


## Actualización 49 — Mapa de diferencias PC x Arcade y carga de datos de la build PC

Tabla completa en `docs/PC_DIFF.es.md` (generada con `tools/map_pc_dos.py` y `tools/cmp_bga.py`). **Confirmado:** `pumpy.exe` es el mismo código del arcade portado a Win32/OpenGL; las funciones PC y DOS corresponden por orden y por strings (PC `0x403BEF/0x403CB0/0x403DA0` = DOS `0x28469/0x285D4/0x284E0`; service PC `0x40F7E0…` = DOS `0x2E4FF…`). **Nuevo en la PC:** comprobación de la alfombra, driver `gwiopm`, MP3 vía DirectShow, `eff\*.wav` por DirectSound, `piukey.cfg`, `c:\pump.ini`, `083.dat`/`083.mp3`. Implementado: `piu_res_load` lee RES v3; `piu_pc` usa `Eff/` y resuelve rutas sin distinguir mayúsculas en Linux/macOS.


## Actualización 50 — Sonido al confirmar un código secreto (Arcade)

**Empírico (usuario):** al confirmar un código en el Arcade suena `2-1.WAV` (índice 2), no `01-11` (índice 1, inferido antes de `0x2A91B`). `code_press` ahora usa `sfx_play(2, 5)`, igual que la PC.


## Actualización 51 — 5-1.WAV junto con la voz de la nota (Dance Grade)

**Empírico (usuario):** al mostrarse la nota en el Dance Grade suena `5-1.WAV` (idx 5, canal 3) junto con el WAV de la nota (RANK_x / A..F). Vale para Arcade y PC (código compartido en `PH_RESULT`, `c == 80`).
