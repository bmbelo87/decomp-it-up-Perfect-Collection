**Idiomas:** [Português (BR)](PC_DIFF.md) · [English](PC_DIFF.en.md) · **Español**

# PC_DIFF.md - Build PC (`pumpy.exe`, 16/04/2001) x Build Arcade (`PIU.EXE`, 18/12/2000)

Objetivo: dos ejecutables en el proyecto - `piu` (datos Arcade) y `piu_pc` (datos PC), misma base de código con `#ifdef PIU_BUILD_PC`.
Fuentes: `E:\06_pc\pumpy_unpacked.exe` (volcado desempaquetado, ver Actualización 48), `game/PIU.EXE` + ELF `game/piu` (direcciones DOS), `tools/cmp_bga.py`, `tools/map_pc_dos.py`.
Leyenda: **[C]** confirmado (leído en el binario/dato) - **[I]** inferido - **[E]** empírico.

## 1. Conclusión principal [C]
`pumpy.exe` es **el mismo código fuente del arcade portado a Win32** (MSVC 6, OpenGL, DirectInput/DirectSound, MP3 por DirectShow). El orden de las funciones y las strings coinciden casi 1:1 con el DOS
(ej. PC `0x403bef/0x403cb0/0x403da0` = DOS `0x28469/0x285d4/0x284e0`, intro/81/82). El menú de servicio, la EEPROM (ahora `c:\pump.ini`), `stage.cfg`, "run step_name", el cargador de sprites (`.SPR/.SP2`, `TYPE/NUM/TILE/ANI/PATTERN`) y el cargador RES existen iguales. Por tanto el `game.c` actual sirve de base; el trabajo es: (a) datos distintos, (b) escenas con arte nuevo, (c) capa de plataforma.

## 2. Correspondencia de funciones PC -> DOS (-> `game.c`)
| PC (pumpy) | DOS (PIU.EXE) | Papel | Estado |
|---|---|---|---|
| 0x401310 | 0x2628F | cargador `.BGA` (`fail - %s %s`) | igual (formato BGA idéntico: 108/108 se parsean con `tools/piu_bga.py`) |
| 0x401ad0 | 0x21FE1 / 0x264B1 | carga `font.tga` | igual |
| 0x402340 / 0x40238F | 0x289B0 / 0x28A3C | `[%s] file not found`, decode | igual |
| 0x403BEF | 0x28469 | intro `bga\01.dat` | **alterado** (01.dat: 57 ítems cambiaron, +11/-4) |
| 0x403CB0 | 0x285D4 | logo `bga\81.dat` | igual (81.dat idéntico) |
| 0x403DA0 | 0x284E0 | título `bga\82.dat` | **alterado** (arte nuevo: PC 49 ítems, 47 solo PC; ver 3) |
| 0x4044A9 | 0x28843 | `bga\03.dat` | casi igual (solo `03.BGA` y `HELP.PNG`) |
| 0x4046B0 / 0x40B880 | 0x2A533 | `TITLE\%s.PNZ` | igual |
| 0x404710 | - | `AUDIO\84.MP3` + `credits` | **nuevo** (créditos en MP3) |
| 0x405090 | 0x2087F | captura `.\cap\p5_%03d.TGA` | igual (debug, muerto [I]) |
| 0x405220 | 0x20AB2 | HUD debug `%02d%c%02d,%5.1f %d/%d` | igual |
| 0x405400 | 0x20D38 | lee `stage.cfg` | igual (Stage.cfg con 3 líneas en orden distinto: 112/226/328) |
| 0x405720 / 0x405740 | 0x20F0F / 0x21170 | `idle %d` / `idle...` | igual |
| 0x405AA0..0x405F00 | - | MP3 por DirectShow (`RenderFile`, `AUDIO\%03d.MP3`, `AUDIO\D%03d.MP3`) | **nuevo** (arcade: `.AUD`/PIU.BIN) |
| 0x406040 | 0x21361 | carga `STEP\%s.NOT` | igual |
| 0x4069E0.. | (gameplay) | gameplay/juicio | **por verificar** (usuario: muy parecido; ver 5) |
| 0x409170..0x409259 | 0x24879..0x24CD2 | `run step_name -d`, `-demo/-demo2/-s1/-s2`, `BGA\%03d.DAT` | igual (argumentos de línea de comandos del original; muerto en el juego normal [I]) |
| 0x40AD00 / 0x40AD48 | 0x29A45 / 0x29A8C | `res_load`, `Memory insufficient` | igual (RES v3: `count`/`datasize` intercambiados) |
| 0x40C240 | 0x2BB5E / 0x2C6AC | MODE SELECT (`02.TGA`; PC lista `MM_BTL/CZ/DB/EZ/HD/NR.TGA`) | **alterado** (99.dat: PC 13 ítems x Arcade 87; ver 3) |
| 0x40D680 | (2B6BC..) | `sel_music_init() - ErrMode`, `bga\099.DAT`, `bga\90.DAT` | igual [I] (no hallada por string en el ELF) |
| 0x40DCE0 | 0x2CF60 / 0x2CE7D | rueda -> `run %s %s %d`, nombres `%d_DB/E1/E2/H1..H4.STF` | igual (pero `Step/` solo tiene `.NOT`; `.STF` = muerto [I]) |
| 0x40E970 | 0x2B633 | barras `BGA\90.DAT` | igual (90.dat idéntico) |
| 0x40EA30 | 0x2C689 / 0x2C663 | `LOAD%d`, `LOAD4`, `02.TGA`, `07.TGA` | **alterado** (099.dat: +43/-41 ítems) |
| 0x40F5B0 | 0x2D14D | demo `run %d_e2.stf -demo %d` | igual |
| 0x40F7E0 .. 0x410280 | 0x2E4FF, 0x2D68B, 0x2D7CA, 0x2D847, 0x2D98D.., 0x2DB18, 0x2DC59, 0x2DCDC, 0x2DFC0, 0x2E167, 0x2E2BC | SERVICE: versión `PUMP IT UP (R%d/v%d.%02d)` (fecha **Apr 14 2001**), EEPROM, GAME SETTING, I/O TEST, EEPROM TEST, SCREEN TEST, GAME OPTION, COIN OPTION, SOUND TEST, DATE SETTING | igual (EEPROM en `c:\pump.ini`; `Lock OK:%d Err:%d`) |
| 0x411080 .. 0x4115CF | 0x2F54B.. | parser de sprites `sp2/spr/tga`, `NUM/TYPE/TILE/ANI/PATTERN` | igual |
| 0x4118D0 | - | `piukey.cfg` (teclas) | **nuevo** |
| 0x411A30 | - | DirectSound + `eff\*.wav` (16 archivos) | **nuevo** (arcade: voz del chip, 35 WAV) |
| 0x411DD0 | - | informe `%s\%d%d%d.txt` (`PUMP IT UP Fusion v%s ... Windows %d.%d.%d`, build `Apr 15 2001`) | **nuevo** |
| 0x412080 | - | `AUDIO\083.MP3` + `bga\083.dat` (GO_*.SPR) | **nuevo** (Game Over propio) [I] |
| 0x4015D0 .. 0x4016F0 | - | comprobación de la alfombra (Actualización 48) | **nuevo**; no reproducir |
| 0x4027C0 .. 0x402970 | - | driver `gwiopm.sys` | **nuevo**; no reproducir |
| 0x402E10 .. 0x4035BD | - | ventana/contexto OpenGL | **nuevo** (equivale al SDL del `piu_pc`) |

## 3. Diferencias de datos (ver `E:\06_pc\comparacao_BGA_arcade_vs_pc.txt`)
- Contenedor RES v3 = v2 con `count`/`datasize` intercambiados; mismo cifrado [C]. Las imágenes PC tienen una leve cuantización (dif. máx. 4) [I: reducción de color].
- 98 de 118 `.dat` con contenido idéntico (músicas 101..517 casi todas); `83.dat`, `90.dat`, `97.dat`, `81.dat` idénticos.
- Ausentes en la PC: `091`-`096`, `098`, `91`, `98` (solo Arcade). Presente solo en la PC: `083.dat` (+ `AUDIO\083.MP3`).
- Pantallas con arte/estructura distintos: **`00.dat`** (+`00.BGA`, `MAH01..06`, flechas rediseñadas), **`01.dat`** (intro), **`82.dat`** (título: `00..20.PNG`, `FORPC.SPR`, `LOGO_01/02`, `PCDF`), **`99.dat`** (menú: 13 ítems, sin `MENU_BG*`/`MMB*`), **`099.dat`** (modo/rueda: `A01..F01.SPR`, `BB/CB/DB/HB/NB` rediseñados), `03.dat` (`HELP`), `413.dat` (sin `HFONT1/2`), `109/506/511` (solo el `.BGA`).
- `Stage.cfg`: posiciones de 112/226/328 distintas; `STEP/` 472 `.NOT`, `AUDIO/` 215 `.mp3` (+ `D4xx.MP3`).

## 4. Plataforma [C]
OpenGL (opengl32) en vez de Glide; DirectInput (`piukey.cfg`) en vez de lectura de pad; DirectSound `eff\*.wav` + DirectShow MP3; EEPROM en `c:\pump.ini`; comprobación de alfombra + driver `gwiopm`. El `piu_pc` usa SDL2 (ya equivalente); teclado = `piukey.cfg` (formato por descifrar).

## 5. Gameplay y Dance Grade (hipótesis del usuario: muy parecidos) - verificar
Nada lo contradice hasta ahora [I]: mismos `.NOT`, mismo `83.dat`, mismos nombres de rank (`eff\A..F.wav` = RANK_A..F), `piu_play.c` ya usa el formato `.NOT`. Falta: comparar números concretos (ventanas de juicio, puntuación/combo, barra de vida) en las cercanías de `0x4069E0` con `piu_play.c` y marcar igual/distinto.

## 6. Plan propuesto (un commit por ítem)
1. [hecho] Dos objetivos (`piu`, `piu_pc`) + CI + lectura RES v3 + carpeta `Eff` + rutas sin distinguir mayúsculas.
2. Nombrar la IAT en Ghidra y confirmar la tabla de la sección 2 (anotar direcciones PC en `docs/FUNCTION_MAP.md`).
3. Título/attract PC (`82.dat`, `01.dat`, `00.dat`).
4. MODE SELECT PC (`99.dat`, `MM_*`) y rueda (`099.dat`, `A01..F01.SPR`).
5. Audio: MP3 por id (`Audio/%03d.mp3`, `D%03d`), efectos `Eff/*`.
6. Gameplay/Grade: validar igualdad; ajustar lo que diverja.
7. Entrada: descifrar `piukey.cfg`.
8. Game Over PC (`083.dat`/`083.mp3`).

## 7. Título PC (82.dat) - implementado [C en el ensamblador 0x403E20; I donde se indica]
- Fondo `82.BGA` corre en `cnt%300` (Arcade `%180`); capas: `1020+cnt%120` siempre, `900+cnt%120` con selección, y por selección 7->300, 1->420, 9->540, 3->660, demás->780 (+cnt%120).
- Menú de 4 esquinas (confirmado visualmente en los cuadros del `82.BGA`): arriba-izq **START** (7), arriba-der **OPTION** (9), abajo-izq **CREDITS** (1), abajo-der **EXIT** (3); texto "SELECT YOUR MENU".
- Lógica: 1er toque en la esquina = selecciona; 2º toque en la misma = confirma. START con crédito = entra (`scnt=0x3C`); EXIT = `WM_CLOSE`; CREDITS = fundido de 30 cuadros y escena 0x26; OPTION = escena 0x1C (`083.dat`/`083.mp3`, 5 ítems - **aún no reproducida**; lo que llamé "Game Over" en la sección 2 es, en realidad, la pantalla de opciones [I]).
- Las burbujas `PRESS CENTER STEP` / `PLEASE INSERT COIN` vienen del `00.BGA` de la PC (cuadros +0/+30 con crédito, +60/+90 sin; `ovc%30`).
- Versión `printf("%d%02d", 4, 1)` = **401** en (601,458) negro y (600,459) blanco (Arcade 352).
- [I] qué tecla física del pad PC corresponde a cada bit (piukey.cfg aún sin descifrar): en `piu_pc` P1 = Z C Q E, P2 = numérico 1 3 7 9 (bits 1 2 8 0x10). Prueba empírica pendiente.
- Prueba: `piu_pc.exe -game <carpeta PC> -title -freeplay -selftest 3 -shot f.ppm`.

## 8. Reglas empíricas de la build PC (usuario)
- `PRESS CENTER STEP` no aparece en la PC (sobra del `00.BGA`): burbujas desactivadas en `piu_pc`. [empírico]
- Flujo: `81.DAT` -> `82.DAT` (título) directo. Modo: `099.DAT` con los discos (`MM_*`) de `99.DAT`. [empírico]
- Sin SERVICE (F1), TEST (F2), COIN CLEAR (F3), COIN 1/2 (F4/F5): el menú de servicio nunca es accesible; entradas libres, **siempre FREE PLAY**. [empírico] (`svc_apply` fija `g_freeplay=1`; `-service` ignorado.) Nota: el código del menú existe en pumpy.exe (0x40F7E0+), pero el juego no lo alcanza.

## 9. Efectos de sonido de la PC (`eff\*.wav`, cargados en 0x411A30 vía DirectShow) [C]
Objeto -> archivo: `0xB66334`=01-1, `0xB66338`=start2 (moneda, 0x404C80), `0xB6633C`=2-1, **`0xB66340`=3-2**, `0xB66344`=4-1, `0xB66350`=7-1, `0xB66354`=8-1, `0xB66358`=5-1, `0xB66374`=10-2, `0xB66384`=start1, `0xB6638C`=start3, `0xB66390..0xB663A0`=A,B,C,D,F.
Título: la selección de esquina **y** la confirmación tocan `3-2` (Stop, posición 0, Run en `0xB66340`); el `piu_pc` usa el índice 3 (`3-2`), no el `10-2` del Arcade (SFX_MOVE).

## 10. Selección de modo PC (escena 0xB, despachador 0x40F4C0; fases en DAT 0xB62250) - análisis [C en el ensamblador; parte aún por reproducir]
- **Fases** (índice de la tabla 0x40F574): 0 `0x40C240` carga (`bga\099.DAT`, `bga\99.DAT`, texturas `MM_BTL/CZ/DB/EZ/HD/NR/GD.TGA`, `bga\097.DAT`); 1 `0x40C350` intro (099.BGA capa 6 cuadro 630+cnt%30; cuadro 120+min(t/2,30)); 2 `0x40CFD0` **elección de modo**; 3 `0x40D620` confirmación (097.BGA); 4 `0x40D680` init de la rueda (`sel_music_init`, `bga\099.DAT`+`bga\90.DAT`); 5 `0x40D840` intro de la rueda; 6 `0x40D9D0` rueda de músicas; 7 `0x40DCE0` inicia (`run %s %s %d` -> gameplay); 8 `0x40EA30` LOAD; 9-11 `0x40EC10/0x40EC50/0x40F3C0` fin/resultados.
- **La PC reutiliza la MISMA pantalla 099.BGA** para modo y música: cuadro 0 = encabezado "MUSIC SELECT", cuadro 60 = "MODE SELECT" (usado en 0x40CFD0), cuadro 150 = guía (GUIDE / SHIFT LEFT / CONFIRM / SHIFT RIGHT), 210-750 = barras de músicas (C01..C14), 630-660 = barras de modo (BB=BATTLE, CB=CRAZY, NSB=NONSTOP, DB=DOUBLE, HB=HARD, NB=NORMAL). No existen las 091-096/098 del Arcade.
- **Elección de modo = carrusel horizontal de discos** (`0x40C6C0`, 9 posiciones -4..+4, escala/alfa por distancia; texturas `MM_*` de 99.dat, lista de 5: NORMAL, HARD, NONSTOP REMIX, CRAZY, DOUBLE/BATTLE según jugadores - `0x40D346`: 2 jugadores cambia el ítem 2 por BATTLE). `MM_GD` = marco vacío. Entrada: bit 1 (abajo-izq) = shift izquierda (dir 2), bit 2 (abajo-der) = shift derecha (dir 3), bit 4 (centro) = confirma el disco central (dir 0); P2 usa `dh`. Sonido del shift: `eff\10-2.wav` (obj 0xB66374); confirmación: `eff\4-1.wav` (obj 0xB66344). Tiempo restante `99 - clock/240` en (584,437) (`0x40D4D0`).
- Textos (burbujas 1P/2P `0x40C3D0`): cuadros base de la 099.BGA por estado (crédito / FREE / modo double): "1P PLAYER PLEASE SELECT YOUR MODE!", "STEP CENTER ARROW TO CONFIRM", "PLEASE WAIT", "1P PLAYER NOT READY PLEASE INSERT COIN(S)".
- **Falta**: constantes de punto flotante del carrusel (0x42A324/0x42A354/0x42A390/0x42A394/0x42A298/0x42A31C/0x42A360) y el dibujo del disco (`0x40C600`). Una captura de la pantalla real de la PC aceleraría la validación.

## 11. Selección de modo PC - implementada (`pm_*` en game.c, `-mode 1` para probar) [C]
Carrusel de 9 posiciones (x por tabla literal de 0x40C742..0x40CBD0, y=333 desde abajo, disco 230x176 de la región 10..240 x 0..177 de `MM_*`, alfa `1-|p|*0.25`, escala `1-|p|*0.1`, `p = slot + 0.05*s`), burbujas 1P/2P (`0x40C3D0/0x40C4AA`: capas 42/40/43/44/39 y 36/34/37/38/33, cuadro `cnt%90 + {390,570,300,210,660}`), guía (capas 27/28/29 en el cuadro 45), barra del modo (capa 5/4/2/3/1/6, cuadro 630+). Referencia visual: captura del usuario de la selección de música (mismo carrusel, portadas en lugar de los discos).
**Pendiente:** tras confirmar, la PC va a la rueda de músicas en la propia `099.dat` (fases 4-6); el `piu_pc` aún cae en el flujo del Arcade (098/091..096 inexistentes en la PC). El tiempo/posición de los dígitos (584,437) y el sonido exacto de shift/confirmación son inferidos.

## 12. Rueda de músicas PC - implementada (pw_* en game.c, `-wheel N` para probar) [C en el ensamblador 0x40EA30/0x40EC10/0x40EC50/0x40E050/0x40C150; I donde se indica]
Mismo esqueleto que la elección de modo: fondo = capa de color del modo (6 NB, 5 HB, 4 DB, 3 NSB/REMIX*, 1 CB, 2 BB) en el cuadro 630+, 099.BGA cuadro 0 (encabezado MUSIC SELECT), flechas 27/28/30/31 en el cuadro 90+, carrusel de 9 portadas 220x162 (CDxx de 90.dat, recorte 2x2 de 128x96, mismas posiciones que el de modos), dígito del nivel (07.TGA, celdas 24x24, tabla 0x42E090) en (320,179), burbujas 1P/2P, tiempo, FREE PLAY. Centro dos veces (o tiempo) confirma: misma salida del Arcade (oculta la música, elige el paso por las flags, pantalla LOAD y gameplay). Previa `D%03d.mp3` tras 30 cuadros apuntando al centro (0x40F0B4). Fase 9 (0x40EC10) = 30 cuadros sin dibujar nada. Lista = secciones del STAGE.CFG como en el Arcade.
**Inferido:** tiempo de 30 s (la PC usa 0x404E20), textos de la guía (capas 27/28/29 en el cuadro 45), pulsos del disco seleccionado (0x40E7C0) no reproducidos.

## 13. DANCE GRADE de la PC - se desvanece [C 0x40A210]
Escena 5 (0x40A210): contador `0x80B754`; 83.BGA en el cuadro 600+cnt%480; a partir de 381 cuadros el alfa global (0x42D034, vía 0x4012F0(1.0, (400-cnt)*0.05)) cae a 0 y la **pantalla entera** se desvanece (en el Arcade solo los números); en cnt>400 (o >250 con centro de P1/P2) sale y viene la pantalla `TITLE\LOAD1..4.PNZ` (LOADn = etapas jugadas; LOAD1 pasa a LOAD4 tras 27/12/2000, 0x40EA30/0x40F7D0). `piu_pc` usa el factor `g_afade` en `bga_draw_n`.

## 14. Elección de modo: sonidos y animación previa a la confirmación (corrección)
- [empírico] desplazar (abajo-izq/abajo-der) y la 1ª confirmación (centro) tocan **3-2** (eff-2.wav, canal 2); la 2ª confirmación (entra al modo) toca **4-1**. Antes el desplazamiento usaba el 10-2 del Arcade (demasiado bajo).
- [C] 0x40CDA2: pad arriba-izq/arriba-der (bits 8/0x10, cualquier jugador) mantenido dibuja las capas 19/20 del 099.BGA en el cuadro 150+cnt%120 (feedback de pulsar). El resto de la animación del centro (capas 47/23/27, disco x1,1) ya existía en `pm_guide`.

## 15. Pulso del disco/portada apuntado (centro 1x)
- [C] 0x40CE20 (modo, llamado en 0x40D0EA) y 0x40E7C0 (música, 0x40EDCB), solo con dir 0/1: 3 copias del disco/portada con p = cnt%50 (cnt = cuadros desde la pulsación): (escala 1+p/150, alfa 1-p/30, p<60), (1+p/100, 1-p/20, p<40), (1+p/50, 1-p/10, p<20), todo x1,1 (glScalef 1,1 anterior a la llamada), centradas en (320,333) en el modo y (320,325) en la música. Misma tabla del FUN_2B459 del Arcade.
- [C] el carrusel de la rueda de músicas usa y=325 (y hacia arriba) en 0x40E050, no 333 como el de modos; corregido (155 desde arriba).
- [I] rueda de músicas: desplazar y la 1ª confirmación (centro) también tocan 3-2 (como en la elección de modo); antes quedaban mudos/10-2.

## 16. Códigos secretos y sonido de los toques (PC)
- [C] `0x40BDA0` (llamada por `0x40F4C0` en las fases 2, 6 y 10 de la escena 0xB = elección de modo y rueda de músicas): todo toque del pad (bits 1F de P1/P2) toca el **3-2** (obj 0xB66340) y ejecuta el verificador de códigos P1 `0x40B920` / P2 `0x40BB20`. El historial (5 toques en 0xB62260/70; 3 toques en 0xB63A28/38) NO se limpia al coincidir.
- [C] Tabla de 5 toques (0x42DFF0): DL DR DL DR DL = cancela todo (mfx &= 0xFFF); UL UL UR UR C = VANISH; UL DL UR DR C = MIRROR; UL DL DR UR C = RANDOM; UL UR C UL UR = NONSTOP; UL UR UL UR C = SYNCHRO (0x1000); UR UL UR UL C = COUPLE (0x2000); DL UL UR DR C = UNION (0x4000); DL C DR C DL = v_rand (0x100). No existen los códigos de REMIXDB/REMIXCZ ni 2X/3X/4X de 8 toques del Arcade.
- [C] Tabla de 3 toques (0x42E020): **Q Q Q** (UL UL UL) cicla la velocidad none>2X>4X>8X>raccel(0x200)>none en cada toque (el historial queda QQQ, así que la 4ª Q avanza de nuevo); **E E E** (UR UR UR) cicla al revés (none>raccel>8X... con el encadenamiento del original: raccel>4X).
- [C] Cambió algo -> efecto índice 2 (obj 0xB6633C = "2-1"). El Arcade usaba idx 1.
- [C] Flechas centrales: Q/E (UL/UR) mantenidas dibujan las capas 19/20 del 099.BGA (cuadro 150+cnt%120); la rueda de músicas tiene la misma guía (0x40E050) que el modo, con la portada en (320,325).

## 17. Timer
- [C] `0x40D4D0(584, 0x1B5, n)`: el timer de la PC queda en y=437 (y hacia arriba), 8 px por encima del Arcade (429). Glifos 02.TGA iguales (celdas 48x40, 5 por fila, v0=162; paso -40, 2 dígitos).

## 18. .PNZ de la PC (pantallas T%03d, LOAD1..4, CREDITS)
- [C] Mismo formato y cifrado que el Arcade (u32 H | datos; p[i] = bitrev8(c[i]) ^ KEY[(H+i)&0x3FF]; H = adler32), pero la **clave de 1024 bytes es distinta**: pumpy.exe @0x42D27C (descifra en 0x4022F0, lectura en 0x402340). 110/110 archivos de `Title/` validan (adler + PNG). Antes `piu_pc` usaba la clave del Arcade -> imagen inválida -> pantalla LOAD negra al entrar a la música y tras el DANCE GRADE. `src/piu_key_pc.h`, `piu_use_pc_key`.


## 19. Pantalla de opciones (083.DAT)

Título de la PC: UR selecciona OPTIONS y UR otra vez abre la escena 0x1C (`piu_pc`: estado `S_OPTION`). **Pedido del usuario:** idéntica al menú de Prex3 (`game_option.c`, `086.DAT`); la PC usa `083.DAT` + `083.MP3` en bucle (las mismas 50 capas). 5 ítems: nivel (3), STAGE BREAK, SHOW HELP, DEFAULT SETUP, EXIT; DL/DR mueven, el centro aplica, salir suena 2-1. Los valores son los del menú de servicio (`g_cfg.level/stage/help`, `PIU_EEPROM.BIN`) [inferido]. Verificado por captura; no jugado.

## 20. Códigos: un buffer por pantalla

**Empírico (usuario):** el historial de toques es separado por pantalla (modo / músicas), se limpia al entrar en cada una, y solo cuenta con la pantalla activa (fases 2 y 10, nunca en la animación de entrada). Antes UL UL en el título + UL en modo completaban QQQ.
