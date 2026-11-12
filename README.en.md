<p align="center">
  <img src="LogoIA.png" alt="PiuReconstructed" width="720">
</p>

**Languages:** [Português (BR)](README.md) · **English** · [Español](README.es.md)

# PiuReconstructed — Pump It Up (PIU.EXE, DOS, "Dec 18 2000") recompiled in C

How to play: double-click `jogar.bat` (uses the `..\game` folder). Or: `PiuReconstructed.exe -game <game folder>`.

## Downloads (Releases)
Binaries for **Windows (x64)**, **Linux (x64)** and **macOS** are built by GitHub Actions on every `v*` tag (*Releases* tab). The original game data (`PIU.EXE`, `BGA/`, `STEP/`, `AUDIO/`...) is **not** included: point to your own folder with `-game <folder>` (default: `../game`).
- Linux: needs `libsdl2-2.0-0` (and an OpenGL driver). macOS: `brew install sdl2`.

Each tag publishes **two editions** per platform: `PerfectCollectionArcade-<platform>` (`piu`, recompilation of the 18/12/2000 `PIU.EXE`; Arcade data via `-game <folder>`) and `PerfectCollectionPC-<platform>` (`piu_pc`, recreation of the 16/04/2001 `pumpy.exe`; put the exe in the PC game folder next to `Stage.cfg`, or use `jogar_pc.bat`).

## Controls
| | |
|---|---|
| Selection | Up/Down song (PgUp/PgDn ±10), Left/Right level, **Enter** play, **1–8** speed, **Esc** quit, F11 fullscreen |
| P1 (5 pads) | `Z` ↙, `Q` ↖, `S` **center**, `E` ↗, `C` ↘ |
| P2 / Double | Numpad `1` ↙, `7` ↖, `5` **center**, `9` ↗, `3` ↘ (in single stages the numpad also plays all 5 columns) |
| Title | `S` (or Enter/Space) = START P1 (PRESS CENTER STEP), Num `5` (or Num Enter/Num +) = START P2 |
| Mode select | `Z`/`C` (P2 Num 1/3, blue) scroll the list; `Q`/`E` (P2 Num 7/9, up) pick the left/right item and pressing the same one again enters it (as in the original code); Enter/Space = shortcut |
| In game | Esc goes back to selection |
| Arcade | **F1** SERVICE (opens the service menu and selects), **F2** TEST (moves), **F3** Coin Clear, **F4** coin (F5 too), Alt+X quits |

## Status (Phase 1: fidelity)
- Faithful (confirmed in the assembly): 240 Hz timing, scroll, BPM change, Perfect/Great/Good/Bad windows per BPM, judgment rules, combo/score/life, `.NOT`, `.AUD`(MP3), `.PNZ`(PNG), `.DAT`(RES/BGA/SPR/SP2) formats.
- Functional reconstruction (the original was not fully decompiled): selection menu, result/grade screen, exact arrow sprites, hit effects, long notes (if any), game modes/options, transitions and attract/title screens.
- Rendering: OpenGL 1.x (SDL2). See docs/ for the address map of the original PIU.EXE.

## Build
`cmake -S . -B build_msvc -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static`  
`cmake --build build_msvc --config Release` (produces `piu.exe`; `piu_test.exe` validates every file).  
Self-test: `piu.exe -game ../game -song 101 -diff 1 -selftest 25 -shot shot.ppm` (autoplay; `-noauto` disables it).

## Screen flow (Update 13) - same as PIU.EXE
`idle -> logo (BGA/81.DAT, 500 frames) -> idle -> intro (BGA/01.DAT, up to 945 frames) -> title (BGA/82.DAT, 180-frame loop) -> selection`.
Origin: main loop `0x20F37` (state in `DAT_00153710`). Without credit the game stays in attract; **5 (or F5) = coin**,
**Enter/Space = START P1**, **Num Enter/Num + = START P2**; about 1 s after starting it goes to selection. ESC returns to idle (as in the original), Alt+X quits.
Options: `-freeplay` (infinite credit), `-coins N`, `-skip` (straight to selection).

## PC build ("Perfect Collection", `pumpy.exe`)
Second target `piu_pc` (`PIU_BUILD_PC`), with the PC data (`PUMPY_0416`): `piu_pc.exe -game <PC folder>`. Differences and addresses: [docs/PC_DIFF.en.md](docs/PC_DIFF.en.md).
