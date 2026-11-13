**Idiomas:** [Português (BR)](CODES.md) · [English](CODES.en.md) · **Español**

# Códigos secretos (flechas del Arcade)

Sprites extraídos de `BGA/00.DAT` (`ARROW01.PNG` / `ARROW03.PNG`, cuadro de animación con todos los colores rellenos, 54×54) con `tools/make_arrow_sprites.py`, guardados en `docs/img/arrows/`. Pad: **UL** = arriba-izquierda, **UR** = arriba-derecha, **C** = centro, **DL** = abajo-izquierda, **DR** = abajo-derecha (teclas de prueba: `Q E S Z C` en P1, `7 9 5 1 3` en P2).

<img src="img/arrows/UL.png" width="28" alt="UL"> UL · <img src="img/arrows/UR.png" width="28" alt="UR"> UR · <img src="img/arrows/C.png" width="28" alt="C"> C · <img src="img/arrows/DL.png" width="28" alt="DL"> DL · <img src="img/arrows/DR.png" width="28" alt="DR"> DR

## Arcade (`PIU.EXE`)

Introducir la secuencia en la pantalla de **modo** o de **músicas**, con las últimas 8 teclas (tabla `0x81219AA` del ELF, `func_2a72b`). Repetir el código apaga el efecto (alterna). Cada efecto de velocidad reemplaza a los otros.

| Efecto | Secuencia (8 toques) |
|---|---|
| **CANCEL** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **VANISH** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **MIRROR** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> |
| **8X** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **4X** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **2X** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **RANDOM VELOCITY** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> |
| **NONSTEP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **SYNCHRO** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **COUPLE** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **UNION** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **RANDOM VANISH** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **RANDOM STEP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> |
| **REMIXDB** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **REMIXCZ** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> |

REMIXDB solo con el modo REMIX y 1 jugador; REMIXCZ solo con el modo REMIX en la rueda de músicas. La fila 16 de la tabla (`10 4 1 10 8 4 2 8`) no se usa.

## PC (`pumpy.exe`)

Introducir en la pantalla de **modo** o de **músicas** (historial de 5 toques en `0x42DFF0` y de 3 toques en `0x42E020`; cada pantalla tiene su propio búfer). Las flechas de abajo son las del Arcade; la PC usa arte propio.

| Efecto | Secuencia |
|---|---|
| **CANCEL** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> |
| **VANISH** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **MIRROR** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **RANDOM VELOCITY** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **NONSTOP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **SYNCHRO** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **COUPLE** | <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **UNION** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **RANDOM VANISH** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> |
| **SPEED UP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UL.png" width="28" alt="UL"> |
| **SPEED DOWN** | <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UR.png" width="28" alt="UR"> |

SPEED UP: cada repetición sube 2X → 4X → 8X → RANDOM STEP → nada; SPEED DOWN hace el camino inverso.

## Notas

Confirmado en el código: las secuencias y el bit de cada efecto. Los **nombres** vienen de la lista del usuario (inferido). Aplicados hoy en el juego: 2X/4X/8X, MIRROR, RANDOM STEP, VANISH (inferido) y RANDOM VANISH (inferido); los demás solo activan la bandera.
