**Languages:** [Português (BR)](CODES.md) · **English** · [Español](CODES.es.md)

# Secret codes (Arcade arrows)

Sprites extracted from `BGA/00.DAT` (`ARROW01.PNG` / `ARROW03.PNG`, animation frame with every colour filled, 54×54) by `tools/make_arrow_sprites.py`, saved in `docs/img/arrows/`. Pad: **UL** = up-left, **UR** = up-right, **C** = centre, **DL** = down-left, **DR** = down-right (test keys: `Q E S Z C` for P1, `7 9 5 1 3` for P2).

<img src="img/arrows/UL.png" width="28" alt="UL"> UL · <img src="img/arrows/UR.png" width="28" alt="UR"> UR · <img src="img/arrows/C.png" width="28" alt="C"> C · <img src="img/arrows/DL.png" width="28" alt="DL"> DL · <img src="img/arrows/DR.png" width="28" alt="DR"> DR

## Arcade (`PIU.EXE`)

Enter the sequence on the **mode** or **music** screen, using the last 8 keys (ELF table `0x81219AA`, `func_2a72b`). Repeating a code switches the effect off (toggle). Each speed effect replaces the others.

| Effect | Sequence (8 taps) |
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

REMIXDB only with REMIX mode and 1 player; REMIXCZ only with REMIX mode on the music wheel. Table row 16 (`10 4 1 10 8 4 2 8`) is unused.

## PC (`pumpy.exe`)

Enter on the **mode** or **music** screen (5-tap history at `0x42DFF0` and 3-tap history at `0x42E020`; each screen has its own buffer). The arrows below are the Arcade ones; the PC uses its own art.

| Effect | Sequence |
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

SPEED UP: each repetition goes 2X → 4X → 8X → RANDOM STEP → none; SPEED DOWN goes the other way.

## Notes

Confirmed in code: the sequences and the bit of each effect. The **names** come from the user's list (inferred). Applied in game today: 2X/4X/8X, MIRROR, RANDOM STEP, VANISH (inferred) and RANDOM VANISH (inferred); the others only set the flag.
