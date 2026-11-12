**Languages:** [Português (BR)](PC_DIFF.md) · **English** · [Español](PC_DIFF.es.md)

# PC_DIFF.md - PC build (`pumpy.exe`, 04/16/2001) vs Arcade build (`PIU.EXE`, 12/18/2000)

Goal: two executables in the project - `piu` (Arcade data) and `piu_pc` (PC data), same code base with `#ifdef PIU_BUILD_PC`.
Sources: `E:\06_pc\pumpy_unpacked.exe` (unpacked dump, see Update 48), `game/PIU.EXE` + ELF `game/piu` (DOS addresses), `tools/cmp_bga.py`, `tools/map_pc_dos.py`.
Legend: **[C]** confirmed (read in the binary/data) - **[I]** inferred - **[E]** empirical.

## 1. Main conclusion [C]
`pumpy.exe` is **the same arcade source code ported to Win32** (MSVC 6, OpenGL, DirectInput/DirectSound, MP3 through DirectShow). Function order and strings match the DOS build almost 1:1
(e.g. PC `0x403bef/0x403cb0/0x403da0` = DOS `0x28469/0x285d4/0x284e0`, intro/81/82). The service menu, the EEPROM (now `c:\pump.ini`), `stage.cfg`, "run step_name", the sprite loader (`.SPR/.SP2`, `TYPE/NUM/TILE/ANI/PATTERN`) and the RES loader all exist unchanged. So the current `game.c` is a valid base; the work is: (a) different data, (b) scenes with new art, (c) platform layer.

## 2. Function mapping PC -> DOS (-> `game.c`)
| PC (pumpy) | DOS (PIU.EXE) | Role | Status |
|---|---|---|---|
| 0x401310 | 0x2628F | `.BGA` loader (`fail - %s %s`) | same (identical BGA format: 108/108 parse with `tools/piu_bga.py`) |
| 0x401ad0 | 0x21FE1 / 0x264B1 | loads `font.tga` | same |
| 0x402340 / 0x40238F | 0x289B0 / 0x28A3C | `[%s] file not found`, decode | same |
| 0x403BEF | 0x28469 | intro `bga\01.dat` | **changed** (01.dat: 57 items changed, +11/-4) |
| 0x403CB0 | 0x285D4 | logo `bga\81.dat` | same (81.dat identical) |
| 0x403DA0 | 0x284E0 | title `bga\82.dat` | **changed** (new art: PC 49 items, 47 PC-only; see 3) |
| 0x4044A9 | 0x28843 | `bga\03.dat` | almost the same (only `03.BGA` and `HELP.PNG`) |
| 0x4046B0 / 0x40B880 | 0x2A533 | `TITLE\%s.PNZ` | same |
| 0x404710 | - | `AUDIO\84.MP3` + `credits` | **new** (credits in MP3) |
| 0x405090 | 0x2087F | capture `.\cap\p5_%03d.TGA` | same (debug, dead [I]) |
| 0x405220 | 0x20AB2 | debug HUD `%02d%c%02d,%5.1f %d/%d` | same |
| 0x405400 | 0x20D38 | reads `stage.cfg` | same (Stage.cfg with 3 lines in different order: 112/226/328) |
| 0x405720 / 0x405740 | 0x20F0F / 0x21170 | `idle %d` / `idle...` | same |
| 0x405AA0..0x405F00 | - | MP3 via DirectShow (`RenderFile`, `AUDIO\%03d.MP3`, `AUDIO\D%03d.MP3`) | **new** (arcade: `.AUD`/PIU.BIN) |
| 0x406040 | 0x21361 | loads `STEP\%s.NOT` | same |
| 0x4069E0.. | (gameplay) | gameplay/judgment | **to verify** (user: very similar; see 5) |
| 0x409170..0x409259 | 0x24879..0x24CD2 | `run step_name -d`, `-demo/-demo2/-s1/-s2`, `BGA\%03d.DAT` | same (original command-line arguments; dead in normal play [I]) |
| 0x40AD00 / 0x40AD48 | 0x29A45 / 0x29A8C | `res_load`, `Memory insufficient` | same (RES v3: `count`/`datasize` swapped) |
| 0x40C240 | 0x2BB5E / 0x2C6AC | MODE SELECT (`02.TGA`; PC lists `MM_BTL/CZ/DB/EZ/HD/NR.TGA`) | **changed** (99.dat: PC 13 items vs Arcade 87; see 3) |
| 0x40D680 | (2B6BC..) | `sel_music_init() - ErrMode`, `bga\099.DAT`, `bga\90.DAT` | same [I] (not found by string in the ELF) |
| 0x40DCE0 | 0x2CF60 / 0x2CE7D | wheel -> `run %s %s %d`, names `%d_DB/E1/E2/H1..H4.STF` | same (but `Step/` only has `.NOT`; `.STF` = dead [I]) |
| 0x40E970 | 0x2B633 | bars `BGA\90.DAT` | same (90.dat identical) |
| 0x40EA30 | 0x2C689 / 0x2C663 | `LOAD%d`, `LOAD4`, `02.TGA`, `07.TGA` | **changed** (099.dat: +43/-41 items) |
| 0x40F5B0 | 0x2D14D | demo `run %d_e2.stf -demo %d` | same |
| 0x40F7E0 .. 0x410280 | 0x2E4FF, 0x2D68B, 0x2D7CA, 0x2D847, 0x2D98D.., 0x2DB18, 0x2DC59, 0x2DCDC, 0x2DFC0, 0x2E167, 0x2E2BC | SERVICE: version `PUMP IT UP (R%d/v%d.%02d)` (date **Apr 14 2001**), EEPROM, GAME SETTING, I/O TEST, EEPROM TEST, SCREEN TEST, GAME OPTION, COIN OPTION, SOUND TEST, DATE SETTING | same (EEPROM in `c:\pump.ini`; `Lock OK:%d Err:%d`) |
| 0x411080 .. 0x4115CF | 0x2F54B.. | sprite parser `sp2/spr/tga`, `NUM/TYPE/TILE/ANI/PATTERN` | same |
| 0x4118D0 | - | `piukey.cfg` (keys) | **new** |
| 0x411A30 | - | DirectSound + `eff\*.wav` (16 files) | **new** (arcade: chip voice, 35 WAV) |
| 0x411DD0 | - | report `%s\%d%d%d.txt` (`PUMP IT UP Fusion v%s ... Windows %d.%d.%d`, build `Apr 15 2001`) | **new** |
| 0x412080 | - | `AUDIO\083.MP3` + `bga\083.dat` (GO_*.SPR) | **new** (own Game Over) [I] |
| 0x4015D0 .. 0x4016F0 | - | dance pad check (Update 48) | **new**; do not reproduce |
| 0x4027C0 .. 0x402970 | - | `gwiopm.sys` driver | **new**; do not reproduce |
| 0x402E10 .. 0x4035BD | - | OpenGL window/context | **new** (equivalent to the SDL layer of `piu_pc`) |

## 3. Data differences (see `E:\06_pc\comparacao_BGA_arcade_vs_pc.txt`)
- RES v3 container = v2 with `count`/`datasize` swapped; same cipher [C]. PC images have slight quantization (max diff 4) [I: color reduction].
- 98 of 118 `.dat` have identical content (songs 101..517 almost all); `83.dat`, `90.dat`, `97.dat`, `81.dat` identical.
- Missing on PC: `091`-`096`, `098`, `91`, `98` (Arcade only). Present only on PC: `083.dat` (+ `AUDIO\083.MP3`).
- Screens with different art/structure: **`00.dat`** (+`00.BGA`, `MAH01..06`, redrawn arrows), **`01.dat`** (intro), **`82.dat`** (title: `00..20.PNG`, `FORPC.SPR`, `LOGO_01/02`, `PCDF`), **`99.dat`** (menu: 13 items, no `MENU_BG*`/`MMB*`), **`099.dat`** (mode/wheel: `A01..F01.SPR`, `BB/CB/DB/HB/NB` redrawn), `03.dat` (`HELP`), `413.dat` (no `HFONT1/2`), `109/506/511` (only the `.BGA`).
- `Stage.cfg`: different positions of 112/226/328; `STEP/` 472 `.NOT`, `AUDIO/` 215 `.mp3` (+ `D4xx.MP3`).

## 4. Platform [C]
OpenGL (opengl32) instead of Glide; DirectInput (`piukey.cfg`) instead of pad reading; DirectSound `eff\*.wav` + DirectShow MP3; EEPROM in `c:\pump.ini`; dance pad check + `gwiopm` driver. `piu_pc` uses SDL2 (already equivalent); keyboard = `piukey.cfg` (format still to decipher).

## 5. Gameplay and Dance Grade (user hypothesis: very similar) - to verify
Nothing contradicts it so far [I]: same `.NOT`, same `83.dat`, same rank names (`eff\A..F.wav` = RANK_A..F), `piu_play.c` already uses the `.NOT` format. Missing: compare concrete numbers (judgment windows, score/combo, life bar) around `0x4069E0` with `piu_play.c` and mark same/different.

## 6. Proposed plan (one commit per item)
1. [done] Two targets (`piu`, `piu_pc`) + CI + RES v3 reading + `Eff` folder + case-insensitive paths.
2. Name the IAT in Ghidra and confirm the table of section 2 (note PC addresses in `docs/FUNCTION_MAP.md`).
3. PC title/attract (`82.dat`, `01.dat`, `00.dat`).
4. PC MODE SELECT (`99.dat`, `MM_*`) and wheel (`099.dat`, `A01..F01.SPR`).
5. Audio: MP3 by id (`Audio/%03d.mp3`, `D%03d`), effects `Eff/*`.
6. Gameplay/Grade: validate equality; adjust whatever diverges.
7. Input: decipher `piukey.cfg`.
8. PC Game Over (`083.dat`/`083.mp3`).

## 7. PC title (82.dat) - implemented [C in the assembly 0x403E20; I where indicated]
- Background `82.BGA` runs on `cnt%300` (Arcade `%180`); layers: `1020+cnt%120` always, `900+cnt%120` with a selection, and per selection 7->300, 1->420, 9->540, 3->660, others->780 (+cnt%120).
- 4-corner menu (visually confirmed in the `82.BGA` frames): top-left **START** (7), top-right **OPTION** (9), bottom-left **CREDITS** (1), bottom-right **EXIT** (3); text "SELECT YOUR MENU".
- Logic: 1st tap on a corner = selects; 2nd tap on the same = confirms. START with credit = enters (`scnt=0x3C`); EXIT = `WM_CLOSE`; CREDITS = 30-frame fade and scene 0x26; OPTION = scene 0x1C (`083.dat`/`083.mp3`, 5 items - **not reproduced yet**; what I called "Game Over" in section 2 is actually the options screen [I]).
- `PRESS CENTER STEP` / `PLEASE INSERT COIN` bubbles come from the PC `00.BGA` (frames +0/+30 with credit, +60/+90 without; `ovc%30`).
- Version `printf("%d%02d", 4, 1)` = **401** at (601,458) black and (600,459) white (Arcade 352).
- [I] which physical PC pad key matches each bit (piukey.cfg not deciphered yet): in `piu_pc` P1 = Z C Q E, P2 = numpad 1 3 7 9 (bits 1 2 8 0x10). Empirical test pending.
- Test: `piu_pc.exe -game <PC folder> -title -freeplay -selftest 3 -shot f.ppm`.

## 8. Empirical rules of the PC build (user)
- `PRESS CENTER STEP` does not appear on PC (leftover from `00.BGA`): bubbles disabled in `piu_pc`. [empirical]
- Flow: `81.DAT` -> `82.DAT` (title) directly. Mode: `099.DAT` with the discs (`MM_*`) of `99.DAT`. [empirical]
- No SERVICE (F1), TEST (F2), COIN CLEAR (F3), COIN 1/2 (F4/F5): the service menu is never reachable; inputs free, **always FREE PLAY**. [empirical] (`svc_apply` forces `g_freeplay=1`; `-service` ignored.) Note: the menu code exists in pumpy.exe (0x40F7E0+) but is not reachable from the game.

## 9. PC sound effects (`eff\*.wav`, loaded at 0x411A30 via DirectShow) [C]
Object -> file: `0xB66334`=01-1, `0xB66338`=start2 (coin, 0x404C80), `0xB6633C`=2-1, **`0xB66340`=3-2**, `0xB66344`=4-1, `0xB66350`=7-1, `0xB66354`=8-1, `0xB66358`=5-1, `0xB66374`=10-2, `0xB66384`=start1, `0xB6638C`=start3, `0xB66390..0xB663A0`=A,B,C,D,F.
Title: corner selection **and** confirmation play `3-2` (Stop, position 0, Run on `0xB66340`); `piu_pc` uses index 3 (`3-2`), not the Arcade `10-2` (SFX_MOVE).

## 10. PC mode select (scene 0xB, dispatcher 0x40F4C0; phases in DAT 0xB62250) - analysis [C in the assembly; part still to reproduce]
- **Phases** (index of table 0x40F574): 0 `0x40C240` load (`bga\099.DAT`, `bga\99.DAT`, textures `MM_BTL/CZ/DB/EZ/HD/NR/GD.TGA`, `bga\097.DAT`); 1 `0x40C350` intro (099.BGA layer 6 frame 630+cnt%30; frame 120+min(t/2,30)); 2 `0x40CFD0` **mode choice**; 3 `0x40D620` confirmation (097.BGA); 4 `0x40D680` wheel init (`sel_music_init`, `bga\099.DAT`+`bga\90.DAT`); 5 `0x40D840` wheel intro; 6 `0x40D9D0` music wheel; 7 `0x40DCE0` start (`run %s %s %d` -> gameplay); 8 `0x40EA30` LOAD; 9-11 `0x40EC10/0x40EC50/0x40F3C0` end/results.
- **PC reuses the SAME 099.BGA screen** for mode and music: frame 0 = "MUSIC SELECT" header, frame 60 = "MODE SELECT" (used in 0x40CFD0), frame 150 = guide (GUIDE / SHIFT LEFT / CONFIRM / SHIFT RIGHT), 210-750 = song bars (C01..C14), 630-660 = mode bars (BB=BATTLE, CB=CRAZY, NSB=NONSTOP, DB=DOUBLE, HB=HARD, NB=NORMAL). The Arcade's 091-096/098 do not exist.
- **Mode choice = horizontal carousel of discs** (`0x40C6C0`, 9 positions -4..+4, scale/alpha by distance; `MM_*` textures from 99.dat, list of 5: NORMAL, HARD, NONSTOP REMIX, CRAZY, DOUBLE/BATTLE depending on players - `0x40D346`: 2 players swaps item 2 for BATTLE). `MM_GD` = empty frame. Input: bit 1 (down-left) = shift left (dir 2), bit 2 (down-right) = shift right (dir 3), bit 4 (center) = confirms the center disc (dir 0); P2 uses `dh`. Shift sound: `eff\10-2.wav` (obj 0xB66374); confirmation: `eff\4-1.wav` (obj 0xB66344). Remaining time `99 - clock/240` at (584,437) (`0x40D4D0`).
- Texts (1P/2P bubbles `0x40C3D0`): 099.BGA base frames per state (credit / FREE / double mode): "1P PLAYER PLEASE SELECT YOUR MODE!", "STEP CENTER ARROW TO CONFIRM", "PLEASE WAIT", "1P PLAYER NOT READY PLEASE INSERT COIN(S)".
- **Missing**: floating-point constants of the carousel (0x42A324/0x42A354/0x42A390/0x42A394/0x42A298/0x42A31C/0x42A360) and the disc drawing (`0x40C600`). A capture of the real PC screen would speed up validation.

## 11. PC mode select - implemented (`pm_*` in game.c, `-mode 1` to test) [C]
9-position carousel (x from the literal table at 0x40C742..0x40CBD0, y=333 from the bottom, 230x176 disc from region 10..240 x 0..177 of `MM_*`, alpha `1-|p|*0.25`, scale `1-|p|*0.1`, `p = slot + 0.05*s`), 1P/2P bubbles (`0x40C3D0/0x40C4AA`: layers 42/40/43/44/39 and 36/34/37/38/33, frame `cnt%90 + {390,570,300,210,660}`), guide (layers 27/28/29 at frame 45), mode bar (layer 5/4/2/3/1/6, frame 630+). Visual reference: the user's screenshot of the music select (same carousel, covers instead of discs).
**Pending:** after confirming, the PC goes to the music wheel in `099.dat` itself (phases 4-6); `piu_pc` still falls into the Arcade flow (098/091..096 do not exist on PC). Time/digit position (584,437) and the exact shift/confirm sound are inferred.

## 12. PC music wheel - implemented (pw_* in game.c, `-wheel N` to test) [C in the assembly 0x40EA30/0x40EC10/0x40EC50/0x40E050/0x40C150; I where indicated]
Same skeleton as the mode choice: background = mode color layer (6 NB, 5 HB, 4 DB, 3 NSB/REMIX*, 1 CB, 2 BB) at frame 630+, 099.BGA frame 0 (MUSIC SELECT header), arrows 27/28/30/31 at frame 90+, carousel of 9 covers 220x162 (CDxx from 90.dat, 2x2 crop of 128x96, same positions as the mode one), level digit (07.TGA, 24x24 cells, table 0x42E090) at (320,179), 1P/2P bubbles, time, FREE PLAY. Center twice (or timeout) confirms: same exit as the Arcade (hides the song, picks the step by flags, LOAD screen and gameplay). Preview `D%03d.mp3` after 30 frames pointing at the center (0x40F0B4). Phase 9 (0x40EC10) = 30 frames drawing nothing. List = STAGE.CFG sections as in the Arcade.
**Inferred:** 30 s time (PC uses 0x404E20), guide texts (layers 27/28/29 at frame 45), pulses of the selected disc (0x40E7C0) not reproduced.

## 13. PC DANCE GRADE - fades out [C 0x40A210]
Scene 5 (0x40A210): counter `0x80B754`; 83.BGA at frame 600+cnt%480; from 381 frames on the global alpha (0x42D034, via 0x4012F0(1.0, (400-cnt)*0.05)) drops to 0 and the **whole screen** fades out (on the Arcade only the numbers do); at cnt>400 (or >250 with P1/P2 center) it exits and the `TITLE\LOAD1..4.PNZ` screen follows (LOADn = stages played; LOAD1 becomes LOAD4 after 12/27/2000, 0x40EA30/0x40F7D0). `piu_pc` uses the `g_afade` factor in `bga_draw_n`.

## 14. Mode select: sounds and pre-confirmation animation (fix)
- [empirical] shifting (down-left/down-right) and the 1st confirmation (center) play **3-2** (eff-2.wav, channel 2); the 2nd confirmation (enters the mode) plays **4-1**. Previously the shift used the Arcade 10-2 (too quiet).
- [C] 0x40CDA2: up-left/up-right pad (bits 8/0x10, any player) held draws layers 19/20 of 099.BGA at frame 150+cnt%120 (press feedback). The rest of the center animation (layers 47/23/27, disc x1.1) already existed in `pm_guide`.

## 15. Pulse of the pointed disc/cover (center once)
- [C] 0x40CE20 (mode, called at 0x40D0EA) and 0x40E7C0 (music, 0x40EDCB), only with dir 0/1: 3 copies of the disc/cover with p = cnt%50 (cnt = frames since the press): (scale 1+p/150, alpha 1-p/30, p<60), (1+p/100, 1-p/20, p<40), (1+p/50, 1-p/10, p<20), all x1.1 (glScalef 1.1 before the call), centered at (320,333) in mode and (320,325) in music. Same table as the Arcade's FUN_2B459.
- [C] the music wheel carousel uses y=325 (y up) at 0x40E050, not 333 like the mode one; fixed (155 from the top).
- [I] music wheel: shifting and the 1st confirmation (center) also play 3-2 (as in the mode choice); before they were silent/10-2.

## 16. Secret codes and pad-tap sound (PC)
- [C] `0x40BDA0` (called by `0x40F4C0` in phases 2, 6 and 10 of scene 0xB = mode choice and music wheel): every pad tap (bits 1F of P1/P2) plays **3-2** (obj 0xB66340) and runs the code checker P1 `0x40B920` / P2 `0x40BB20`. The history (5 taps at 0xB62260/70; 3 taps at 0xB63A28/38) is NOT cleared on a match.
- [C] 5-tap table (0x42DFF0): DL DR DL DR DL = cancels everything (mfx &= 0xFFF); UL UL UR UR C = VANISH; UL DL UR DR C = MIRROR; UL DL DR UR C = RANDOM; UL UR C UL UR = NONSTOP; UL UR UL UR C = SYNCHRO (0x1000); UR UL UR UL C = COUPLE (0x2000); DL UL UR DR C = UNION (0x4000); DL C DR C DL = v_rand (0x100). The Arcade's REMIXDB/REMIXCZ and 8-tap 2X/3X/4X codes do not exist.
- [C] 3-tap table (0x42E020): **Q Q Q** (UL UL UL) cycles the speed none>2X>4X>8X>raccel(0x200)>none on each tap (the history stays QQQ, so the 4th Q advances again); **E E E** (UR UR UR) cycles backwards (none>raccel>8X... with the original's chaining: raccel>4X).
- [C] Something changed -> effect index 2 (obj 0xB6633C = "2-1"). The Arcade used idx 1.
- [C] Center arrows: Q/E (UL/UR) held draw layers 19/20 of 099.BGA (frame 150+cnt%120); the music wheel has the same guide (0x40E050) as the mode screen, with the cover at (320,325).

## 17. Timer
- [C] `0x40D4D0(584, 0x1B5, n)`: the PC timer sits at y=437 (y up), 8 px above the Arcade (429). Same 02.TGA glyphs (48x40 cells, 5 per row, v0=162; step -40, 2 digits).

## 18. PC .PNZ files (T%03d, LOAD1..4, CREDITS screens)
- [C] Same format and cipher as the Arcade (u32 H | data; p[i] = bitrev8(c[i]) ^ KEY[(H+i)&0x3FF]; H = adler32), but the **1024-byte key is different**: pumpy.exe @0x42D27C (decrypted at 0x4022F0, read at 0x402340). 110/110 files in `Title/` validate (adler + PNG). Before, `piu_pc` used the Arcade key -> invalid image -> black LOAD screen when entering a song and after the DANCE GRADE. `src/piu_key_pc.h`, `piu_use_pc_key`.


## 19. Options screen (083.DAT)

PC title: UR selects OPTIONS and UR again opens scene 0x1C (`piu_pc`: state `S_OPTION`). **User request:** identical to the Prex3 menu (`game_option.c`, `086.DAT`); the PC uses `083.DAT` + `083.MP3` looping (same 50 layers). 5 items: level (3), STAGE BREAK, SHOW HELP, DEFAULT SETUP, EXIT; DL/DR move, centre applies, leaving plays 2-1. Values are the service menu's (`g_cfg.level/stage/help`, `PIU_EEPROM.BIN`) [inferred]. Checked by screenshot; not played.

## 20. Codes: one buffer per screen

**Empirical (user):** the tap history is separate per screen (mode / music), cleared on entering each, and only counts while the screen is active (phases 2 and 10, never during the entry animation). Before, UL UL on the title + UL in mode completed QQQ.


**Transition (user request):** the title fades out (30 frames) when opening options and the options fade in; EXIT just returns to the title, no effect.


## 21. Credits screen (scene 0x26)

**Confirmed** in `pumpy.exe`: on the title, DL selects CREDITS and DL again fades (30 frames) and sets scene `0x26` (`0x403EA4`). Init `0x404710`: loads `TITLE\CREDITS.PNZ` (`0x4046B0`) and plays `AUDIO\84.MP3` (`0x405CE0`); loop `0x404790`: draws the 640x480 image full screen (`glDrawPixels`) for up to `0x76C` (1900) frames, or until centre (bits `0x404`); on exit it stops the music (`0x405D90`) and returns to the title (`0x403DA0`). Music repeat [inferred]. `piu_pc`: state `S_CREDITS`. Checked by screenshot; not played.
