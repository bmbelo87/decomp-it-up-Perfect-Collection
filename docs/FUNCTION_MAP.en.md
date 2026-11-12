**Languages:** [Português (BR)](FUNCTION_MAP.md) · **English** · [Español](FUNCTION_MAP.es.md)

# PIU.EXE (DOS, build "Dec 18 2000") — symbol map

Addresses = VAs of the original LE (also those of `piu_le.exe` opened in REA).
Symbols cannot be renamed in REA (Ghidra/Windows without mutation) — **this file is the source of the names**.
Confidence: C = confirmed in the pseudocode, I = inferred (needs corroboration).

## Layout (LE, 4 objects)
| Obj | Base VA | Size | Contents |
|---|---|---|---|
| 1 | 0x10000 | 0x10 | stub (discarded in the synthetic PE) |
| 2 | 0x20000 | 0xE45F0 | code (game 0x20000–0xFB…, Watcom/Glide/zlib runtime up to ~0x104600) |
| 3 | 0x110000 | 0x48 | data |
| 4 | 0x120000 | 0x9060D0 | data/bss (52 pages with content) |

## Boot (C)
- `0xFC004` `_cstart` (Watcom/DOS4GW; INT21h, DPMI) → `0xFF54C`, `0xFF4FD`, `0xFF597`
- `0xFF4FD` `__CMain` → calls **`game_main` 0x21235**
- `0xFF597` finalizer runner (table 0x15350C..0x153518, 6-byte entries)
- `0x21235` **game_main**: `game_init 0x20CC0` → `game_loop 0x20F37` → `game_shutdown 0x20D7C`

## game_init 0x20CC0 (C, call order)
`0xFBEB2, 0xFBE3F, 0x20C5B, [DAT_0012B010=1 running], 0xFBF3A, 0xFBF93, 0x28297, 0x29218, 0x299D6, 0x20727, 0x2E61C, 0x21FE1, 0x264B1, 0x29E3B, 0x2D78A (if !=0 → state=0x1C service), 0x2A19B, 0x2FB34, 0x2F9C7, 0x2F95F(0,0x406E0000), 0x2FA4C, 0x266C0(&0x15374C), 0x20B84`
Names: not assigned yet (0xFB… = probable runtime/Glide init — I).

## game_shutdown 0x20D7C (C)
`0x2056F, 0xFBE91, 0xFBFA3, 0x2FB9A, 0x294B9, 0x282EA, 0x26516, 0x2E65B`
(mirrors the init in reverse order — I)

## game_loop 0x20F37 (C)
Globals: `running`=0x12B010, `game_state`=0x153710, `key`=0x153714 (scancode<<8), `input_flags`=0x153718, `debug_overlay`=0x153734, `dbg2`=0x15372C.
Per frame: `0xC76BC` (begin frame) → `0x20DCD` (input reading, returns scancode<<8 | flags) → keys:
- 0x2D00 + flag 0x80000 → leaves the loop (X; I)
- 0x2900 → `0x26742`; 0x4200 → toggles `debug_overlay` (F8; I); 0x0100 → `game_state=0` (ESC; I); 0x4300 → `0x2087F` (F9; I)
- `0x26759()` decides `0x20E0E` vs `0x26948`; `0x27D08` (if state!=2); `0xC4FE0`
- **dispatch by `game_state`** (table below) → `0x205C4` (overlay) → `0xC4F2C`, `0x20C1E`, `0xDB26E` (end frame/flip)

### States (game_state → handler) (C)
| state | handler | note |
|---|---|---|
| 0,3,6-9,12,15,0x12,0x15,0x17-0x1B,0x1E | fallback 0x26611 / 0x20E7A | no handler of its own |
| 1 | 0x247E2 | |
| 2 | 0x255D9 | (0x27D08 does not run in state 2) |
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
| 28 (0x1C) | 0x2E46A | service/test screen (entry) |
| 29 (0x1D) | **0x2E4D4** | service screen: switch on `DAT_0012CB80` (9 sub-states: 0x2D847, 0x2D954, 0x2DB18, 0x2DC59, 0x2DCDC, 0x2DFC0, 0x2E167, 0x2E2BC, 0x2E450); draws "PUMP IT UP (R%d/v%d.%02d)" and "Dec 18 2000" via 0x26658(x,y,str) |
| 31 (0x1F) | 0x25E90 | |
| 32 (0x20) | 0x25F27 | |

Utility functions already seen: `0x26658(x,y,fmt,...)` = draw text (I); `0xFB6E2(n)` = Watcom stack check (prologue; ignore).

## Songs (STAGE.CFG) (C, source: file)
ID ranges: 1xx = 1st, 2xx = 2nd, 3xx = 3rd, 4xx = 4th, 5xx = this version (reported by the user).

## Next targets
1. `stage.cfg` parser (string @0x120077) and the `.NOT` format.
2. BGA/audio/TITLE loader; Glide layer (gr* → OpenGL).
3. Gameplay state handlers (identify which state = selection, play, result).

---
## Update 2 — STAGE.CFG and calling convention

### Calling convention (C — observed at 0x20D37: `mov eax,0x120077; call 0x2A19B`)
Watcom register convention (`-3r`): args in **EAX, EDX, EBX, ECX**, the rest on the stack. Ghidra's decompiler **loses these args**
(shows `FUN_xxx(void)`). Always confirm with `read_bytes`/assembly before assuming the signature. `0xFB6E2(n)` = `__CHK` (stack check), ignore.

### STAGE.CFG loading
- `0x2A19B` **stage_cfg_load(const char *path)** (C): `fopen`(0xFBA01) → `while fgets`(0xFCACF) → tokenizes (0xFC6A5 → `DAT_006E1780`) → `strcmp`(0xFC570) against 3 keywords → handlers
  `0x2A039`, `0x2A0CA`, `0x2A16A` (I: `T`, `MODE`, `S`, exact order not yet verified) → `fclose`(0xFBC75) → final loop: `DAT_006E163A[i*2] = DAT_006E163C[i*2] - DAT_006E1638[i*2]` (duration = end − start, int16 stride 4; I)
- Global counters: `0x12C380`, `0x12C384`, `0x12C388` zeroed at the start (I: no. of T / modes / entries).
- Called in `game_init` with `"stage.cfg"` (@0x120077) in EAX.

### STAGE.CFG format (C — read from the file E:\06_pc\game\STAGE.CFG)
```
T <id>:<bpm>:<title>         105 songs (id 1xx=19, 2xx=24, 3xx=30, 4xx=15, 5xx=17)
MODE<TAB><name>              EASY HARD DOUBLE REMIX REMIXDB CRAZY REMIXCZ MENU END
<TAB>S<TAB><level><TAB><id>  section entries (EASY=61 HARD=69 DOUBLE=70 REMIX=18 REMIXDB=15 CRAZY=37 REMIXCZ=4 MENU=5)
```
IDs: 1xx=1st, 2xx=2nd, 3xx=3rd, 4xx=4th, 5xx=this version. The 5xx ones show up in EASY (509–517) and HARD/DOUBLE, etc.
Each id links to `STEP/<id>_<DB|E1|E2|H1|H2…>.NOT` (pattern seen: 101_DB.NOT, 101_H1.NOT, 103_E1.NOT…).

---
## Update 3 — STAGE.CFG parser closed + step loader

### 0x2A19B stage_cfg_load (C) — handlers
The MODE name is **not read**: modes are **positional** (order in the file: 0 EASY,1 HARD,2 DOUBLE,3 REMIX,4 REMIXDB,5 CRAZY,6 REMIXCZ,7 MENU,8 END; max 9).
- `0x2A0CA` **T** line (I: keyword not read): `song[DAT_0012C380]`: stride 40 B @0x6DFC28: `+0 int id` (atoi 0xFC4F5), `+4 float bpm` (strtod 0xFC8DC), title via 0xFBE3F; `DAT_0012C380++`.
- `0x2A039` **S** line: entry `DAT_0012C384` stride 4 B @0x6DF6B0: `[+1]=level`; if level==0 → `[+0]=0,[+2]=0`; otherwise `[+0]= (level>=12)`, and if >=12 `level-=10`; `[+2]=u16 id`. `DAT_0012C384++`.
- `0x2A16A` **MODE** line: `mode_start[DAT_0012C388]` (u16, stride 4 @0x6E1638) = `DAT_0012C384`; `DAT_0012C388++` (cap 9). End: `mode_len[i]=start[i+1]-start[i]` (@0x6E163A).
(Which of the 3 strings 0xFC570 each handler compares has not been read from the bytes yet; the T/S/MODE assignment is based on the shape of the handlers.)

### Step loader `0x21361` (C)
`sprintf(path,"STEP\%s", DAT_003D585A /*name*/)` → fopen → 3 freads (header; see NOT_FORMAT.md) → fclose → `0x21251` (decrypt+expand) → sweeps 0x800 rows×10 cols swapping '1'→'9'. Failure: `0x266C0("fail %s %s")`.
Resource strings: `AUDIO\%03d.AUD`, `AUDIO\D%03d.AUD`, `BGA\%03d.DAT`, `T%03d`(+.PNZ in TITLE), `.\cap\p5_%03d.TGA` (screenshots), `font.tga`, `event.tga`, `BAR01.TGA`, `CD01.TGA`.
On-disk data formats: `.NOT` (steps), `.AUD`, `.DAT` (BGA, + BGA_extracted/ already extracted), `.PNZ` (TITLE; probably compressed PNG — the build embeds libpng/zlib).

---
## Update 4 — arrow rendering (0x22840)
`0x22840` **draw_arrows(player)** (C; player in EAX: base column 0 = P1, 5 = P2). Player option flags: `DAT_003D0C1C` (P1) / `DAT_003D5800` (P2).
- Speed multiplier `mult` (C): `flags&8`→8; else `flags&4`→4; else `flags&2`→2; else 1.
- Other flags (I): `0x10`/`0x100` choose a sprite variant (`0x22750` / `0x2279F`); `0x200` and `DAT_0012B028&0x40` merge flags with those of `DAT_0015C94C/950` (effect: mirror/side swap — not confirmed).
- Window: `row0 = DAT_003D0C18*DAT_001558D4/60 - 2*DAT_001558D4` (min 0); sweeps `[row0, row0 + 10*DAT_001558D4)`. `DAT_003D0C18` = music time (C: used as a 1/60 counter; unit to be confirmed).
- Arrow Y = `DAT_003D0C14*mult/240 - (row*60/DAT_001558D4)*mult`.
- 5 passes per column, with X translate: col0→0, col1→48, col2→96, col3→144, col4→192 (C, floats 0/48/96/144/192). Sweep order in the code: 0,2,4,1,3.
- Helpers (I): `0xC6EF8` translate, `0xC4FE0` push matrix, `0xC4F2C` pop, `0xC7944` set color, `0x2EE67` draws the arrow sprite (column args in a register, lost in the decompiler). Ends with a tail call `0x224FB`.
- Consumers of `0x15594C`: 0x214A1 (57 B), 0x21762 (291 B), several chunks in 0x21479–0x216EF (includes the expand/loader), 0x22840.

---
## Update 5 — gameplay frame (0x24DE6) and timing
`0x24DE6` **gameplay_frame** (C; play state handler; ends with tail call `0x224FB`). It is not called directly from the switch: one of the `game_state` handlers (candidates 0x247E2/0x255D9/0x25639) calls it — to be confirmed.
- `0x205B4` **get_music_pos** = `return DAT_00153738` (C).
- Mode flags `DAT_0012B028` (ushort): bit0 = P1 active, bit1 = P2 active (C); `0x40` = demo/attract (I); `0x80|0x200` = double/co-op mode (I, `&0x280` copies P1→P2 data); byte2 `&0x10` = special mode (I).
- Per-frame time computation (C):
  `pos = max(get_music_pos(), DAT_0012B030)`; `DAT_0012B02C = max(pos,0)`;
  `DAT_003D0C14 (scroll) = round( DAT_0012B02C − offset[DAT_003D0BF4]*0xF0/100 )`; `offset[]` = `DAT_001558B4` = **int[8] at file 0x18..0x38 of the .NOT** (101_*: offset[0]=161) ⇒ **unit of 0xF0 (=240) per base unit; offset in hundredths**;
  `DAT_003D0C18 = scroll / 0xF0`.
  Debug (`DAT_0015372C`): prints `"%02d:%02d:%02d"` with `pos/0x3840`, `(pos/0xF0)%60`, `(pos%0xF0)*100/0xF0` ⇒ **pos is in 1/240 s** (0x3840 = 14400 = 60 s × 240) — **C**. So `DAT_003D0C18` = whole seconds and `DAT_003D0C14` = 1/240 s.
  (Open conflict: with 1/240 s, `row0 = c18*DAT_001558D4/60 − 2*N` gives few rows/s; the Y formula at 0x22840 may use `DAT_001558D4` as another quantity (e.g. pixels/beat). Reconcile by reading 0x22840 in assembly before porting.)
- End of song: if `pos` reaches the end → `DAT_00153710 = 10` (up to 2 times via `DAT_0012B024`) or `= 4` (I: result screen / next stage).
- `DAT_003D0BEC`++ per frame (animation counter); `DAT_003D0BD0`++ (game frames); `DAT_0012B034` = "first frame" flag (initializes audio/video: 0x294B9, 0x2949F, 0x205A4, 0x2058F).
- Life/gauge (I): P1 at `DAT_003D5808..`, P2 `DAT_003D5831..` (test values 500/400/300/200/100/900 set when `DAT_00153714==0x8600`, F12-like cheat/test; float `DAT_003D580C`=1.0 = full bar; game over if gauge < 1 and `!(flags&0x40)`).
- Draw order per frame (C): bg `0x2643C` → (P1) `0x22674`, (P2) `0x226AD` (or `0x226F9` in double) → arrows `0x22DCD`/`0x22DF1`/`0x22E20` (per player, if option `&0x80` == 0) → `0x22FD8` → bars (translate X=176.0 / 572.0, Y=16.0; `0x21E47`, `0x21885`) → `0x23A4B` (per player; I: judgment/combos) → `0x21915`, `0x20134`.

## Option transformations on the note table (C)
- `0x214A1` **copy_p1_to_p2**: for the 0x800 rows copies cols 0–4 → cols 5–9 (`0x155951 + r*14 + c = 0x15594C + r*14 + c`, c<5).
- `0x21762` **mirror_chart**: for each row, in place and **in sequence** (order matters!): if src col=='9' then dst='9' with (src→dst): 0→8, 1→9, 2→7, 3→5, 4→6, 5→3, 6→4, 7→2, 8→0, 9→1. Only writes '9', never erases. Reproduce the exact order.
- Other transformations between 0x214D9–0x216EF (refs to 0x15594C) not read yet.
- Judgment does NOT reference `0x15594C` directly (uses pointer/offset) — find it in 0x23A4B/0x22DCD.

## Update 6 — scale conflict resolved (C)
See NOT_FORMAT.en.md §"Time scale and scroll". Tick per interrupt: `0x20514` (increments `0x15373C`, `0x153740`, `0x153738`; calls `0x26CE5`, `0x29335`, and every 2 ticks `0x27C23`/`0x20424`/`0x27D08`). `0x20514` is the 240 Hz timer ISR/callback.
`0x24C61–0x24D63` **stage_start** (C): reads the id from the name at `0x3D6380`, loads `BGA\%03d.DAT` (fallback `BGAN.DAT` for ids 0x74–0x77 and 0xDB–0xE0), waits for `0x2057F() >= 0x4B0` (1200 ticks = 5 s), `0x27B2A(0)`, zeroes the tick (`0x2058F`) and `set_music_pos(0)` (`0x205A4`), `game_state = 2`.

---
## Update 7 — input (pads), life and states
### 0x27D08 poll_pads (C) — called by the ISR 0x20514 (state==2, every 2 ticks) and by game_loop (state != 2)
`0xFC73A()` reads the I/O board (inp() on a port; recomp: replace with keyboard/gamepad). Bytes read: `3D64FC..3D6502`; `AH` = service byte (active-low).
Current button mask `DAT_003D6514` (`+0x153720`), pressed this tick `DAT_003D6510` (OR into `DAT_00153718`), released `DAT_003D650C` (`0x15371C`), previous `DAT_003D6518`.
| Bit | Origin (C) |
|---|---|
| P1 0x08 / 0x10 | `[64FC] & 0x0F` / `& 0xF0` |
| P1 0x04 / 0x01 | `[64FD] & 0x0F` / `& 0xF0` |
| P1 0x02 | `[64FE] & 0x0F` |
| P2 0x800 / 0x1000 | `[6500] & 0x0F` / `& 0xF0` |
| P2 0x400 / 0x100 | `[6501] & 0x0F` / `& 0xF0` |
| P2 0x200 | `[6502] & 0x0F` |
| 0x100000 / 0x10000 / 0x20000 / 0x40000 | `AH&4==0` / `AH&2==0` / `AH&0x40==0` / `AH&0x80==0` (service/coin/test — I) |
In `game_state==2` (and `DAT_0012B028 byte2 & 0x10 == 0`) or `==0x1D`: for each pressed/released P1 bit (0x01..0x10) it calls `0x27BE9`; for P2 (0x100..0x1000) it calls `0x27C1C` (**candidates for pad judgment/effect**; pad index goes in a register — lost in the decompiler; read assembly).
Likely column→bit mapping: col0=0x01, col1=0x02, col2=0x04, col3=0x08, col4=0x10 (I, by bit order; confirm in 0x27BE9).

### 0x23A4B draw_life_gauge(player) (C; player in EDX)
P1 at X=32.0/Y=444.0, P2 at X=352.0. Life: `DAT_003D5810` (P1) / `DAT_003D5839` (P2); blinks when `< 0xB4 (180)`. "In song" state = `game_state == 2` (C: `stage_start` sets 2; the ISR only runs poll/game logic in that state).

### Summary map of confirmed states
- `2` = in song (stage_start 0x24C61.., 240 Hz ISR, poll_pads, gameplay_frame) — C/I
- `0x1D` = service/test screen (0x2E4D4) — C
- `10`, `4` = destinations at the end of the song (I: result / next stage)


## Update 8 — Judgment (confirmed in assembly, capstone)

Correction: 0x27BE9/0x27C1C are NOT judgment; they only set coin/service bits on port 0x2A4 (via 0xFC743). Ghidra did not define functions at 0x22E5F–0x24D64 ("not_in_procedure"); read through capstone.

- apply_judgment 0x23229 (EAX=player 0/1, DL=note '5'..'9'): switch by jump table 0x23215 (DL-0x35):
  '5' Perfect(0x234D5): counter +0x12 (0x3D5822), combo +0x0C++, life +=(+0x04*12/1000), +0x14 combo-bonus; combo>3 => +0x3E8 at 0x3D5808
  '6' Great(0x2354A): counter +0x14 (0x3D5824), combo++, bonus +0x1F4
  '7' Good(0x235B8): counter +0x16 (0x3D5826), combo=0
  '8' Bad(0x235E6): counter +0x18 (0x3D5828), combo=0, life -= 0x32
  '9' Miss(0x2362A): counter +0x1A (0x3D582A is the row pointer, see below; miss counter at another offset), combo=0, life -= f(life)
  Player struct: base 0x3D5810, stride 0x29. +0x00 life(0x3D5810), +0x04 (0x3D5814), +0x0C combo(0x3D581C), +0x10 (0x3D5820), +0x1A current row (16.16, 0x3D582A), +0x43 (P2 row 0x3D5853).
- Cells of table 0x15594C (stride 14 = 1 row; 10 columns; P1 0..4, P2 5..9): '0' empty, '1' note, '9' pending note, '5'..'8' = Perfect..Bad already judged. Row note = max(char) of the columns (0x22F21 P1 col 0-4, 0x22F5D P2 col 5-9, 0x22F9C all).
- Press: in the frame, for each bit of 0x153718 (P1) / 0x153719 (P2) (pressed this frame), if the cell of the current row 0x3D582A>>16 == '9' => it is replaced by the computed note (BL) = classification by distance:
  d = |(row*60/N) - 0x3D0C18| (0xFC4D5=abs); d<=T0 => '5'; d<=T1 => '6'; d<=T2 => '7'; otherwise '8'.
  Thresholds (0x24BA6.., by slot BPM b=[0x1558A4]): Tk = round((b*Mk/120 + Ck) * 0.5) with (M,C)=(8,4)->0x3D0BE4 Perfect, (16,5)->0x3D0BD8 Great, (24,6)->0x3D0BF0 Good, (32,7)->0x3D0BDC Bad. Unit: the same as the scroll position in 0x22840 (60 px/beat).
- Column<->bit mapping P1: 0x01->[+0], 0x02->[+4], 0x04->[+2], 0x08->[+1], 0x10->[+3]; P2: 0x01->[+5(0x155951)]... (see code 0x242E7).
- After the row passes (0x3D582A>>16 > 0x1557EC = last judged row): note=max of the row (0x22F9C); if '5'..'8' => apply_judgment; cells still '9' become Miss ('9'). Demo (flag 0x40): random note '5'..'7' via 0x23E2A (rand%10: <7 Perfect, <9 Great, otherwise Good).
- Current row: 0x23ED4 (per frame) computes 0x3D0BD4 = round((12B02C - time)*bpm/240 ...)*N/60+1 (clamp 0..2047, monotonic) and stores it in 0x3D582C; helpers 0x23E69 (recomputes BPM slot 0x3D0BF4 by comparing 0x1558C4/0x1558C8 = BPM change points of the 3rd array).
- Resets 0x23155/0x23194/0x231D6: cells != '0' go back to '1' (columns 0-4, 5-9, 0-9) — used when the player fails (stage break/continue).


## Update 9 — File formats (confirmed, 100% of the files validate by Adler-32)

Checksum everywhere = Adler-32 (0x298D1/0x2992E, mod 0xFFF1).
- Generic loader 0x289B0 (path EAX, buffer EDX): fread u32 H, then up to 0x300000 bytes; decrypts 0x2896A; checks adler.
  AUDIO\%03d.AUD (0x29263), AUDIO\D%03d.AUD (0x29295), TITLE\%s.PNZ (0x2A533) use this loader.
  Cipher: p[i] = bitrev8(c[i]) XOR KEY[(H+i)&0x3FF]; KEY = 1024 bytes at 0x12B13C (initial data, obj4); H == adler32(p).
  Result: .PNZ = PNG; .AUD = MP3 (ID3v2). tools/piu_unpack.py (322/322 OK).
- Container BGA\*.DAT "RES " (res_load 0x29A45; res_fopen 0x26290 reads the directory of the 50 entries of 0x1000-ish ...): header 0x18 {magic,ver=2,count,datasize,?,adler(encrypted data)};
  directory count*28 encrypted with XOR(0xD2,+0x95) (0x29949) {name[16],size,offset,adler(plain)}; data with the same XOR. Contents: .PNG(3380), .SPR(3583), .BGA(114 = BGA script), .SP2(5). tools/piu_res.py (117/117 OK).
- Still open: .BGA, .SPR, .SP2 formats; TGA (cap\p5_%03d.TGA, font.tga) on disk? (none); Glide layer.


## Update 10 — .BGA / .SPR / .SP2 (assembly + tools/piu_bga.py: 114/114 .BGA and all .SPR/.SP2 parse)

BGA (loader 0x2628F -> res_load 0x29A45, entry "<name>.bga"): 16B header {'BGA\0', u32 @obj+4 (120/146/180...: looks like BPM, not confirmed), u32 @obj+8, u32 @obj+0}.
  Then UP TO 50 layers (the loader reads 50 but the file may have fewer: 102.BGA has 40; end = EOF): name[64] (.spr/.tga of the layer), u32 N, N keyframes of 44 B. Size = 16 + 68*L + 44*sum(N), L<=50.
  Keyframe (render 0x25FA5; 0x2643C(obj,frame) walks the 50 layers; 0x2647A(obj,layer,frame) just one):
    s16 time, s16 active (0 => does not draw/ends), f32 x, y, pivx, pivy, scale, rot, r, g, b, a  (colors x 0x12B0B0/0x12B0B4 = 1.0)
    Before the 1st key nothing is drawn; between keys i-1 and i it interpolates linearly (0x25F88: a+(b-a)*t, t=(f-t0)/(t1-t0)) all fields; after the last key => returns 1 (end).
    Matrix: translate(0,480) -> translate(x,-y) -> pivot/rotate(rot, Z) -> scale(s,s,1) (inverted Y, 640x480 screen); glColor(r,g,b,a); sprite drawn by 0x2EFAD(sprite=layer+0x44, t).
SPR/SP2 (text, CRLF): "TYPE <TILE|ANI|PATTERN a w h|IMAGE>" / "NUM n" (2 files omit the word NUM) / n lines "T file.tga x y w h u0 v0 u1 v1" (pixels, 256x256 texture; ".tga" maps to the .PNG of the same name in the RES).
  0x2EFAD (draw by type, u16 field at +0x20): type 0/1 (TILE/IMAGE) draws all entries; 2 (ANI) picks frame floor(t*n); 3 (PATTERN a w h) w*h tiles repeating/scrolling the frames with parameter a (inferred).
  0x2EEFF draws 1 quad: screen vertices (words at +0x24,+0x26,+0x28,+0x2A; Y=480-y), float UV at +0x2C..+0x38.
  SP2 (arrow541..545): lines of 11 tokens "T tag file x y w h u v w h" (extra tag; source probably u,v,w,h) -- not confirmed.
Open: u32 @obj+8/@obj+0 of the BGA header, BGA<->music time link, PATTERN parameter a, font.tga/event.tga.


## Update 11 — C skeleton (src/)
- `src/piu.h` API; `piu_data.c` (Adler-32, ciphers, RES, .NOT, .BGA, interpolation 0x25FA5); `piu_play.c` (240 Hz clock, current row, windows by BPM, classify, apply_judgment); `piu_key.h` (1024 B KEY extracted from the exe); `main_test.c` (`build/piu_test.exe ../game`).
- Validation: AUD 213/213, PNZ 109/109, DAT 117/117 (all entries and .BGA), NOT 472/472. Autoplay 101_DB (bpm 146.1, N=2): P/G/Go/B windows = 7/12/18/23 px; 92 notes; error 0 and 6 => 92 Perfect; error 20 => mix (P37 G18 Go29 B8).
- Pending for fidelity: initial life/limits (0x23A4B), `[0x3D0BE0]` (life_rate modifier; -700 set at 0x24B2D..), final score calculation, hold/long notes (mask columns 10-12?), result screen, Glide->OpenGL, SDL2 + window, MP3 audio, input.


## Update 12 — Playable game (src/game.c)
SDL2+OpenGL 1.x. `PiuReconstructed.exe`/`jogar.bat`. Selection (STAGE.CFG + TITLE/Txxx.PNZ), game (BGA from the .DAT with keyframes/SPR, arrows from ARROW01/03.PNG, judgment 0x23229/windows 0x24BA6, lost row = Miss, initial life 500, fail at 0), result. BGA frame = pos/4 (60 fps; 0x250C5). Selftest with autoplay: 172/172 Perfect in 101_H1; without input: Misses and fail. Pending: gamepad, music preview, original sprites/effects (SP2 ARROW541-545), hold/long notes, original screens.


## Update 13 - attract/boot flow (confirmed in PIU.EXE)
- Main loop FUN_00020F37; state in DAT_00153710: 0 idle, 0x0A/0x0B selection (0x2B763/0x2D0BF), 0x0D/0x0E logo, 0x10/0x11 intro, 0x13/0x14 title, 0x16/0x17 03.DAT screen (0x28843/0x288A4, 1920 frames), 0x1C setup (0x2E46A), 0x1D (0x2E4D4), 0x01 game (0x247E2).
- Name table at 0x12B0C9 {name ptr, fn}: exit, idle, run, setup, sel, intro, logo, title.
- idle 0x20E7A: credit (FUN_204BA; 99 if free play) -> state 0x13; otherwise n=DAT_001537D8++%4: 0 logo(81.DAT), 1 intro(01.DAT), 3 FUN_2D14D (not reconstructed).
- Frame counter DAT_003D651C; FUN_2643C(EAX=bga, EDX=frame) draws the 50 layers. Logo/intro: frame = counter BEFORE the ++; title: frame = counter % 180 (IDIV 0xB4 at 0x2874D).
- logo ends at 500 (0x1F4); intro at 0x3B1; title returns to idle with counter >= 0x708 without credit/start. START requires credit (FUN_2047F = DAT_00153730 / DAT_006E191A); 60 frames (0x3C) after START (or P1+P2) -> 0x2B763 (selection).
- DAT_006E1919 != 0 and DAT_006E1914 == 0 -> goes through the 03.DAT screen (0x28843) before selection (values come from runtime config; assumed 0).
- Title text: printf("%d%02d",3,0x34) at (601,458) black and (600,459) white = "352" (version). Overlay FUN_20134: CREDIT(S)/INSERT COIN(S)/PRESS CENTER STEP from FONT.PNG (00.DAT), blinking alpha |(t%200-100)*0.01|+0.2 (floats at 0x12000C/0x120010). On-screen positions: INFERRED.
- Coin: FUN_20424 (bit 0x20000 of the input, state != 0x1D): DAT_00153730++. ESC (0x100) -> state 0. Alt+X quits.
- Pending: FUN_2D14D (demo/ranking), real selection 0x2D0BF, 03.DAT screen, 83.DAT/84.DAT, setup 0x2E46A, real result.

## Update 14 - real selection (state 0xB, FUN_0002D0BF) - PARTIAL ANALYSIS
- State machine DAT_006E1798 (0..7): 0 init FUN_2BB5E (loads BGA\099.DAT, plays AUDIO\%03d.AUD via FUN_29263), 1 intro FUN_2BC14, 2 selection FUN_2BD93, 3 FUN_2C4AB, 4 FUN_2C63F (BGA\098.DAT), 5 FUN_2C81C, 6 FUN_2C840 (big), 7 FUN_2CDFA. Phases 2 and 6 call FUN_2CFC2/FUN_2ABDA.
- BGA/099.DAT = "Music Select" screen (title, TIME, rails, SHIFT buttons, "1P/2P PLAYER PRESS CENTER STEP TO JOIN"); 99.DAT/98.DAT = menu_bg + mode buttons (czp, btlp, dbp, ezp, hdp...); 091..096.DAT = per-level buttons (NB/HB/...); 090/091(2 digits).DAT = title bars (BARnn.PNG = 256x256 sheets with 8 bars, CDnn.PNG = round covers).
- Order of the bars: by song id, only for songs that have a bar (BAR01 = 109..115,201; BAR02 = 202..210...). Exact id->(sheet,row) mapping NOT confirmed.
- STAGE.CFG has "MODE EASY ..." sections with "S level id" lines separated by a blank line (pages) - the original uses this for the lists.
- Horizontal list with step 0x42 (66 px) in FUN_2BD93; selection timer 0x1E - (FUN_2057F/0xF0) (30 s); per-player keys: bits 8/0x10/1/2 = left/right/..., mode in DAT_006E1778 (1..6 -> flags 0x10,0x20,0x100,0x80,0x40,0x400 in DAT_0012B028).
- 83.DAT = BGA of the game screen (ctx2, bar, t_perf/t_great/t_good/t_bad/t_miss/t_combo, ngb, lin; 0..2790 frames); 84.DAT = ending; 03.DAT = help screen (1935 frames).
- Tool: piu.exe -view <name> -frame N -selftest 1 -shot x.ppm renders any BGA.

## Update 15 - real selection: phase 2 = MODE choice (FUN_0002bd93) [observed]
- FUN_0002bc14 (phase 1->2): draws 099 (FUN_0002643c), zeroes DAT_006e17a4 (animation counter), DAT_006e1798=2, DAT_006e1784=7.
- FUN_0002bd93 (phase 2): vertical list of modes, item = 0x42 px, scroll animates 0x16 frames (3 px/frame).
  - DAT_006e1788 = direction (0/1 stopped, 2 up, 3 down); DAT_006e179c = current item; DAT_006e178c = total items.
  - Input: DAT_00153718 (P1: bits 1,2,8,0x10) and high byte (P2); DAT_00153720/21 bit1/bit2 = repeat.
  - Item (DAT_006e1788+DAT_006e179c) -> DAT_006e1778: 0->4 (or 5 if 2 players), 1->1, 2->2, 3->3, 4->6.
  - Confirm: bits in DAT_0012b028 |= 0x10(mode1) 0x20(2) 0x100(3) 0x80(4) 0x40(5) 0x400(6); DAT_006e1798=3 (next phase).
  - Timer: 30 s (0x1e - time/0xF0) -> auto-confirms the current item.
- Conclusion: the real order is 099 -> MODE choice -> (phase 3) music. The music list only comes in phase 3.

## Update 16 - SFX search in PIU.EXE (partially negative result)
- PIU.EXE: 0 occurrences of RIFF/WAVE/fmt/.WAV/VOC/ID3/OggS; no PCM block (code entropy ~6 up to 0xF0000, then low data).
- Only file formats loaded: AUDIO\%03d.AUD (0x120336) and AUDIO\D%03d.AUD (0x120345) via FUN_00029263 (only builds the name; id in EAX), BGA\*.DAT (SPR/PNG/BGA/SP2).
- BGA\*.DAT (234 DATs/entries): only .SPR .PNG .BGA .SP2 -> no audio inside.
- AUDIO\001/002/003.AUD = 128 kbps MP3 of 18.2 s / 29.2 s / 36.3 s (screen music, not SFX). FUN_28843 (help, state 0x17) plays 003 (MOV EAX,3 before 0x29263) -> 003 = help music. 001 probably intro (FUN_28442 passes the id in EAX; not confirmed).
- The EXE's service menu (strings @0x1056065): "SOUND TEST", "EFFECT SOUND", "DEMO SOUND", "AUDIO" -> a sound effect option exists, but the sound source is neither in a file nor embedded in a detectable way.
- Conclusion: no SFX in a file or embedded was found. Hypothesis (not confirmed): effects came from the arcade cabinet's sound hardware.

## Update 17 - SFX: they are HARDWARE (sound board), not files [observed + inferred]
- FUN_00030fac(EAX=on/off, EDX=idx, EBX=voice) = "play effect": writes a 16-byte descriptor (table @0x7238A0 + idx*16) into the registers of a sound chip through port [0x724AB8] (FUN_00030085 = port write; address/data pairs at base+0..3, +0x20..23, +0x40..43, +0x60..63) and ends with key-on (reg1 | 0x80).
- Confirmed callers: coin (FUN_00020424: FUN_30fac), phase 2 selection (FUN_2bd93: move item and confirm), among others.
- Init: FUN_00028297 = chip init (27cd4/27b2a, waits 500 ms, detects with FUN_3037c) and then FUN_3155e reads PIU.BIN in 16-byte records (size>>4 = 35) into the voice table, then FUN_30fa5.
- PIU.BIN (560 bytes) = 35 voice descriptors (chained addresses: the end of one = the start of the next) -> the PCM itself is NOT in a file nor in the EXE: it sits in the arcade sound board's ROM.
- Conclusion: there are no WAVs of this version to extract; the 35 effects can only be replaced by external samples.

## Update 18 - SFX: EFFECTS folder + Linux ELF (piu, R5/v3.52, not stripped) [observed]
- game/EFFECTS has 35 WAV = the 35 voices of PIU.BIN. Order/index = table `effects_file_names` (@0x08113500 of the ELF): 0 01-1,1 01-11,2 2-1,3 3-2,4 4-2,5 5-1,6 6-2,7 7-1,8 8-1,9 9-5,10 9-A,11 9-B,12 9-C,13 9-D,14 9-F,15 TIME_LIMIT,16 10-2,17 13-1,18 12-1,19 14,20 START1,21 START2,22 START3,23..27 RANK_A,B,C,D,F,28..31 NOTGOOD1-4,32..34 GOODJOB1-3.
- The ELF is a translation of the DOS code: symbols jump_XXXXX / func_XXXXX = VA addresses of PIU.EXE (e.g. func_294b9, jump_2c14d). ac_sound_play_effect(EAX=0, EDX=index, EBX=channel) = FUN_00030fac.
- Calls (DOS VA: index/channel): 0x20470 coin 21/-; 0x2c14d,2c181,2c249,2c27a (move L/R in mode) and 0x2cb76,2cbaa,2cc72,2cca3 (move L/R in music) 16/4 (10-2); 0x2c486 and 0x2cdc3 confirm 4/3 (4-2); 0x2882e,288f2,2acda,2acf9,2d048,2d0xx pad presses 3/2 (3-2); 0x2a91b 2/1 (2-1, 5 s); 0x25e44 rank 27/6 (RANK_F, other ranks enter through the same point); 0x24f50,24f9a 7/7 (7-1); 0x234b5 9/2 (9-5); 0x2e151 stop effect (eax=1).
- game.c: sfx_load/sfx_play (8 channels, a new one on the same channel cuts the previous). Hooked up: coin, move, enter the song, join on the title, result rank. NOT hooked up (call site not traced yet): "1 full credit", NOTGOOD/GOODJOB, TIME_LIMIT, 2-1, 7-1, 9-x.

## Update 19 - SERVICE menu rebuilt [observed, except where marked]
- Dispatcher FUN_0002E4D4: DAT_0012CB80 = screen (0 menu FUN_2D847, 1 I/O FUN_2D954, 2 EEPROM FUN_2DB18, 3 SCREEN FUN_2DC59, 4 GAME OPTION FUN_2DCDC, 5 COIN OPTION FUN_2DFC0, 6 SOUND TEST FUN_2E167, 7 DATE FUN_2E2BC, 8 EXIT FUN_2E450). Header "PUMP IT UP (R%d/v%d.%02d)" (4,4) with 5,3,0x34 (= R5/v3.52, same as the ELF's `version` file) and "Dec 18 2000" (0x224,4); footer (0xEC,0x24) "MOVE - TEST BUTTON" + (0xEC,0x10) "SELECT - SERVICE BUTTON", or just "EXIT - SERVICE BUTTON" on screens 1-3. Y-up coordinates, 8 px advance font (FUN_00026658 = text).
- Buttons: DAT_00153718 == 0x10000 TEST (move, cursor DAT_0012CB84 in the menu / DAT_0012CB88 in the rows), == 0x20000 SERVICE (select).
- Menu: title "GAME SETTING" (0x114,0x1C2); items x=0x114, y=0x160-0x14*i; "Lock OK:%d Err:%d" (0x114,0x98) (counters of FUN_29818).
- Colors (FUN_000C7E84): selected row red (0x12CB98), normal white (0x12CBE0), default value green (0x12CBB0).
- GAME OPTION (values x=0x194): GAME MODE 6e1914 NORMAL/EVENT; LEVEL 6e1915 0..2 (1.EASY 2.NORMAL 3.HARD, default 1); STAGE BREAK 6e1916 0..4 (OFF / "%d STAGE", default 2); LANGUAGE 6e1917 0..2 (ENGLISH, ENGLISH, CHINESE); DEMO SOUND 6e1918 OFF/ON; SHOW HELP 6e1919 OFF/ON; DEFAULT SETTING FUN_2D6B5 (14=0,15=1,16=2,17=0,18=0,19=0,1b=1); SAVE AND EXIT FUN_2D750; EXIT.
- COIN OPTION: COIN SETTING 6e191a 0..10 (0 = "FREE", otherwise "1 CREDIT / %2d COINS" at x=0x174), DEFAULT SETTING FUN_2D70F, SAVE AND EXIT, EXIT.
- SOUND TEST: AUDIO (#%02d, 6e1920) and EFFECT SOUND (#%02d, 6e191c), EXIT; TEST stops the sound (FUN_294B9) and moves; SERVICE plays (FUN_2E0ED audio / FUN_2E12A effect).
- DATE: YEAR(%4d, 6e192c=2000), MONTH(%2d, 6e1928 only 11/12), DAY(%2d, 6e1924; 1..30 if month 11, 1..31 if 12), EXIT saves (FUN_2D3AB).
- I/O TEST: "1.TEST  BUTTON" (0x104,400) "2.SERVICE BUTTON" (0x17C) "3.CLEAR BUTTON" (0x168) "4.COIN" (0x154), state by FUN_2D640 (not read; here: pressed button turns red - inferred). EEPROM TEST: WRITE/READ/RESULT with "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345" and SUCCESS/FAIL.
- INFERRED: SCREEN TEST screen (FUN_2D194/2D259 not read: color bars), AUDIO #id value, persistence file PIU_EEPROM.BIN (the original writes to EEPROM), default coin 1.
- game.c: svc_open/svc_press/svc_draw; F1 opens/SERVICE, F2 TEST, F3 coin clear, F4 coin; `-service N` opens screen N directly. COIN SETTING now controls g_cpc/g_freeplay.

## Update 20 - screen music [observed]
- FUN_00028442 (start of the 01.DAT intro): FUN_294B9 (stops the audio); if there is credit -> title; otherwise, if DAT_006e1918 (DEMO SOUND) != 0: FUN_00029263(EAX=1) + FUN_0002949F = AUDIO\001.AUD.
- FUN_0002BB5E (entry of selection / mode choice, after PRESS CENTER STEP): FUN_294B9 and AUDIO\002.AUD unconditional (EAX=2), loads BGA 099 (strings 0x1205F9..), zeroes DAT_006E17A4/1778, 1788=-1, 1798=179C=1.
- FUN_00028843 (help 03.DAT, state 0x17): AUDIO\003.AUD (EAX=3).
- game.c: intro plays 001 if DEMO SOUND (initial default on: inferred, the code's literal DEFAULT SETTING sets 0); selection plays 002; the other states call music_stop.

## Update 21 - MODE choice screen (state 0xB phase 2) rebuilt [observed]
- Init FUN_0002BB5E: FUN_294B9, BGA 099, FUN_2B609(1) loads the 14 sheets BAR01..14 (90.DAT) into table 0x12C3A1 (14 rows x 40 bytes: [0]=texture, [1..8]=ids of the 8 bars, [9]=index), AUDIO\002.AUD. List = STAGE.CFG "MODE MENU": ids 2,3,4,5,6 (DOUBLE, NORMAL, HARD, NONSTOP REMIX, CRAZY); with 2 players item 0 becomes id 1 (BATTLE) (FUN_2BC14: DAT_006E0EE8 hi16 = 1). BAR14 bars: 516,517,1..6 = Slam, Space Fantasy, BATTLE, DOUBLE, NORMAL, HARD, NONSTOP REMIX, CRAZY.
- Loop FUN_2BD93 per frame (60 Hz): time = 30 - FUN_2057F/0xF0; (1) mode highlight layer (DAT_6E1778 0..6 -> layers 0,3,1,2,4,6,5) at frame 600+f%90 (FUN_2647A = 1 layer; FUN_2643C = 50); (2) whole BGA at frame 30; (3) time number at (584,429) yellow (FUN_2A672); (4) arrows: layers 0x1B/0x1C and 0x1E/0x1F at frame 90+anim; (5) translate(local) with local = anim*3-66 (dir 2) or -anim*3+66 (dir 3) for 22 frames; FUN_2B192 draws the bars at (213,218)+66*i; FUN_2B459 pulses the highlight at (292|358,319); BGA at frame 150 shifted by -(cur-1)*66; (6) layer 0x2E/0x2F at frame f%30.
- Bars (FUN_2AE66/2AF37): glRotatef(45), 194x32 quad (x 30..224) with u 0.113..0.875, v = bar*0.125; highlight centered (-97,-16). These routines use Y UP (480-y) - the BGA does not.
- Pulse: p=(anim/2)%50; copies scale p/150+1 alpha 1-p/30 (p<60), p/100+1 and 1-p/20 (p<40), p/50+1 and 1-p/10 (p<20).
- Input (DAT_00153718): bits 1/2 = scroll left/right (dir 2/3; cur-- / cur++, cur 0..3, only with anim>3; limit -> anim=22), bits 8/0x10 = choose item cur+0 / cur+1 (dir 0/1). Confirms when the SAME button is pressed 2 times (prev==dir) or time < 1 s; item (dir+cur) -> mode 4|5 (Double/Battle), 1, 2, 3, 6 -> DAT_0012B028 |= 0x80|0x40, 0x10, 0x20, 0x100, 0x400; next phase 3 (music list).
- Not read: FUN_2A672 (time number), FUN_2B958, FUN_21915, FUN_20134 (player/credit HUD). Key->bit mapping is inferred (Q/Left, E/Right, Z/Up, C/Down, Enter = confirm).
- game.c: S_MODE (mode_enter/mode_step/draw_mode, bga_draw_n); `-mode 1|2` opens the screen with 1 or 2 players.

## Update 22 - keys and pad bits (Linux ELF `piu`) [observed]
- ELF: player1keys @0x081134E0 = Q,E,S,Z,C (GLFW 0x51,0x45,0x53,0x5A,0x43) -> pad bits 0..4 (io_input_kbd_p1: `esi ^= 1<<i`); player2keys @0x081134CC = KP7,KP9,KP5,KP1,KP3 (0x147,0x149,0x145,0x141,0x143). serviceKeys G,H,J (+K, L on P2) set service/coin bits (0x200, 0x4000, 0x8000, 0x400).
- CORRECTION: the ELF key table is the PORT's, not the DOS bits (it cannot be used to infer the bit order). From the user's observation on the mode screen: bits 1/2 = down-left/down-right (Z/C) scroll the list (dir 2/3); bits 8/0x10 = up-left/up-right (Q/E) select cur+0/cur+1 (dir 0/1) and repeating the same confirms. DOS pad bit order: 1=DL 2=DR 4=center 8=UL 0x10=UR (inferred). The center (S) is not tested on this screen.

## Update 23 - BGA layer interpolation: the pivot is NOT interpolated [observed]
- FUN_00025FA5 (0x260D9..0x2624F): between keys i-1 and i it only interpolates x, y, scale, rotation and color r,g,b,a (FUN_25F88 = lerp); the pivot (px,py, +0xC/+0x10) comes from the previous key, not interpolated. Matrix: translate(0,480), translate(x,-y), translate(px,-py), rotate, scale, translate(-px,py), translate(0,-480) = Y up, equivalent to the Y down used in game.c.
- Effect: the zoom/fade ring of the SHIFT bubbles (099.BGA layers 0x1C and 0x1F, m09_1/m09_2.SPR, 1.0 -> 1.2 with alpha 1 -> 0, pivot (135,392)) was out of place because the pivot of the last key (0,0, inactive) was being interpolated.
- piu_data.c: piu_bga_eval now uses the previous key's pivot.

## Update 24 - phase 3: chosen mode animation (097.DAT) and phase 4/5 [observed]
- FUN_0002BB5E loads BGA 097 into object 0x2FF58C. FUN_0002C4AB (phase 3, 30 frames): FUN_2643C(0x2FF58C, frame = DAT_006E17A4 + table[DAT_006E1778]) with table @0x12CB25 = {0,0,120,360,240,480,600} (mode 1 NORMAL 0, 2 HARD 120, 4 DOUBLE 240, 3 REMIX 360, 5 BATTLE 480, 6 CRAZY 600; 097.BGA has 6 segments of 120 frames: nzb/hdb/dbb/nrb/btb/czb). On reaching 30: FUN_294B9 (stops music 002) and DAT_006E1798 = 4.
- FUN_0002C63F (phase 4): loads BGA 098 (object 0x15C95C) and the mode's colored background in object 0x2FF58C: DAT_006E1778 -> 0:091, 1:091 (NORMAL pink), 2:092 (HARD orange), 3:094 (REMIX blue), 4:093 (DOUBLE green), 5:095 (BATTLE red), 6:096 (CRAZY purple); FUN_2B609(0), FUN_2B70B (list by group DAT_006E1784 according to the mode), DAT_006E1798 = 5.
- game.c: S_MCONF (mconf_step/draw_mconf) and, afterwards, S_SELECT with 098 + 09x background under the old list; `-conf N` opens the animation of mode N.

## Update 25 - phase 5: music wheel, FUN_0002C840 [observed; parts marked as inferred]
- Same machine as the mode choice (DAT_006E1788 = dir: 0/1 point at the left/right poster, 2/3 scroll; DAT_006E17A4 = anim; DAT_006E1794 = previous dir; DAT_006E179C = position; DAT_006E178C = list size). Input accepted with anim > 3 (bits 8/0x10 -> dir 0/1, 1/2 -> dir 2/3). Scroll: anim==0 -> pos -= 2 / += 2 (circular, FUN_2B09D). Confirm: same pointer twice (prev == dir < 2) or 30 s (FUN_2057F/0xF0): DAT_006E17A0 = side, FUN_30FAC(0, 4, 3), DAT_006E1798 = 7.
- Drawing: 09x background (object 0x2FF58C) rotating cnt*0.06 degrees around (320,240), frame (cnt*2)%90; 098 (0x15C95C) at frame 0; SHIFT arrows layers 0x1B/0x1C (dir 2) and 0x1E/0x1F (dir 3) at frame 90 + anim (anim < 90); wheel FUN_2B0DD: translate (213,218), 8 items +66 and 8 items -66, local offset anim*6-0x84 (dir 2) / -anim*6+0x84 (dir 3) for 22 frames; each item is a BARxx bar of 90.DAT (FUN_2AFFF, same table 0x12C3A1 as the mode choice, song ids 101..517).
- With dir 0/1: layers 0x23/0x2B/0x2C of 098 at frame 0x96, two posters (FUN_2B266(0,0), (1,0), (dir,anim)) and BPM "%5.1f" at (0x128,0x63) (FUN_2C5CA, table 0x6DFC28: id -> float).
- Poster (FUN_2AE17/FUN_2AD1B): table 0x12C5D1, 30 rows of 40 bytes (handle + 4 ids): row r = sheet CD(r+1).PNG of 90.DAT (CD01..CD26; 256x256, 2x2 crops of 128x96: u = (j%2)*0.5, v = (j/2)*0.375). Drawn 200x142 centered at (161,320) / (481,320) with Y up; pulse as in the bubbles: p = (anim/2)%50, scale p/S0+1, alpha 1-p/S1 while p < S2, S = {150,30,60},{100,20,40},{50,10,20} (0x1205B3..).
- List (FUN_2B70B): 4-byte entries at 0x6DF6B0 (byte0 = valid, byte1 = level, hi16 = id), filled by FUN_2A039 from a text file (not located) and, without restriction (DAT_0012B024==0), FUN_2B695 marks all as valid. Per mode (DAT_006E1784) only the bars [DAT_006E1636, +DAT_006E1638) of the table enter.
- Not reconstructed: the LEVEL value (byte1 of each entry, "%d" at (0x130,0x46) via FUN_26658), the per-group/mode list (here all STAGE.CFG songs enter), music pre-listening (FUN_29295/FUN_2949F after 30 frames pointing), FUN_2A672 (time number), FUN_2B958, FUN_21915, FUN_20134 and phase 7 (difficulty choice).
- game.c: S_WHEEL (wheel_enter/wheel_step/draw_wheel, CD_TBL[26][4]); `-wheel MODE [-wdir 0|1|2|3]` opens directly; after confirming it falls into the old list (difficulty) with the chosen song.

## Update 26 - per-mode lists (STAGE.CFG) and PRESS CENTER STEP position [observed / inferred where indicated]
- FUN_0002C63F: DAT_006E1784 (STAGE.CFG section, positional) = 0 EASY if flag 0x10 (NORMAL), 1 HARD if 0x20, 2 DOUBLE if 0x80, 3 REMIX if 0x100, 4 REMIXDB if 0x200, 6 REMIXCZ if 0x800, 5 CRAZY if 0x400; BATTLE (0x40) depends on DAT_0012B024 (0 EASY, 1 HARD, 2 REMIX). FUN_2B70B copies the section entries (range [DAT_006E1636 + g*4, count DAT_006E1638 + g*4]) into the wheel list.
- "S level id" line = 4-byte entry: byte1 = level (printed "%d" at (0x130,0x46) on the wheel), hi16 = id. Blank lines separate pages per series (grouping only).
- Inferred: I did not find who sets the flags 0x200 (REMIXDB) and 0x800 (REMIXCZ). By the data REMIXDB = REMIX played in double (only 119 and the others up to 415) and REMIXCZ = REMIX played in crazy (225, 327, 330, 415). In the reconstruction, REMIX always uses the REMIX section; DB/CZ are not selected yet.
- FUN_00020134 (state 0x14): FUN_21885(x, y=0x16 up, sprite): PRESS CENTER STEP = sprite 0x25 at x=0xE (P1) / 0x198 (P2); INSERT COIN = sprite 0x24 at x=0x30 / 0x1A8. game.c adjusted.

## Update 27 - REMIXDB / REMIXCZ: secret codes (found in the Linux ELF) and real phases [observed]
- Real numbering of the phases of state 0xB (dispatcher `jump_2d0dd` of the ELF, table 0x8059B3B; DAT_006E1798): 0 FUN_2BB5E, 1 FUN_2BC14, 2 FUN_2BD93 (mode), 3 FUN_2C4AB (097), 4 FUN_2C63F (loads 098/background/list), 5 FUN_2C81C (only sets phase=6 and zeroes the time), 6 FUN_2C840 (**music wheel**), 7 next. (Update 25 called the wheel "phase 5".)
- FUN_2B958 (chunk 0x2ACDA in the ELF) on every pad tap calls FUN_2A72B (`func_2a72b`): memcmp of the last 8 keys (history DAT at 0x86D6CC2) against 15 sequences of 8 bytes (table 0x81219AA; bits 1 DL, 2 DR, 4 center, 8 UL, 0x10 UR), dispatch by table 0x8057331. Ignored if the BATTLE flag (0x40) is on.
- With the REMIX flag (0x100): `DL UL DL C UR DR UR C` (func_2a88a) and (flag&3)!=3 (1 player only): flag &= 0xF00F, |= 0x200 (REMIXDB), group (DAT_006E1784) = 4, rebuilds the list (FUN_2B70B). `C DL C DR C DL C DR` (func_2a8c4) and phase == 6: flag |= 0x800 (REMIXCZ), group = 6, rebuilds the list. In keys (Z=DL, C=DR, Q=UL, E=UR, S=center): REMIXDB = Z Q Z S E C E S; REMIXCZ = S Z S C S Z S C.
- Other codes of the same table (not implemented): `C x8` clears flags (&0xFFF); several toggle game option bits (DAT 0x83CAEAA: 0x10, 0x20, 0x8, 0x4, 0x2, 0x40, 0x80, 0x100/0x200) and bits 0x1000/0x2000/0x4000 of the mode flag.
- game.c: `wheel_code` (8-tap history, only with REMIX mode) switches the section and rebuilds the list.

## Update 28 - music preview on the wheel (empirical)
- When pointing at a poster on the music wheel (UL/UR = Q/E, numpad 7/9) the preview `AUDIO\Dxxx.AUD` of the pointed song plays (xxx = STAGE.CFG id); repeating the same side (confirm) does not restart it; leaving the wheel for the selection stops the preview. **Inferred/empirical**: the original routine that triggers the preview has not been located in PIU.EXE yet (not confirmed whether scrolling also changes the preview).
- game.c: `music_load_pfx("D", id)` in `S_WHEEL`.

## Update 29 - phase 7: from the wheel straight to gameplay
- Confirmed (ELF func_2cdfa = FUN_2CDFA, phase 7 of 0xB): takes the song (list DAT_006E1782 + index, FUN_2B09D), stops the music (FUN_294B9), builds the step name from the mode flags (0x81206d2 / DOS DAT_0012B028) with `%d_E1/H1/H3/E2/H2/H4/DB.STF` and calls FUN_2058F ("run %s %s %d"). There is no difficulty selection screen: the **level** is the mode chosen before (EASY/HARD/REMIX/DOUBLE/CRAZY), the wheel lists only the songs of the section, and confirming enters gameplay.
- Rule: DOUBLE/REMIXDB -> DB; 1P: HARD/REMIX -> H1, CRAZY/REMIXCZ -> H3, otherwise E1; 2P: HARD/REMIX -> H2, CRAZY/REMIXCZ -> H4, otherwise E2. Flag 0x4000 branch (battle, %d_H1/E1 + "-S") not handled.
- game.c: `wheel_step` calls `start_song(di)`; the old S_SELECT screen only serves as a fallback if the file is missing.

- Loading screen (FUN_2A533, ELF func_2a533): before starting, sprintf "T%03d" of the song, loads TITLE\T%03d.PNZ and draws it fullscreen (ac_game_draw_title); then FUN_2058F starts the game. game.c: state S_LOAD (`load_step`/`draw_load`); duration ~1.5 s and **empirical** (in the original it is the load time).

## Update 30 - gameplay frame (ELF func_24de6 = FUN_24DE6): draw map (partial)
Confirmed in the ELF (not yet checked in PIU.EXE; `func_XXXXX` addresses = DOS VA). Draw order of a frame:
1. Background: `func_2643c(0x8152006, frame)` (frame = time>>2; 50 calls of `func_25fa5`, BGA layers) - unless the 0x40 flag (battle) is in the "blink" branch (full red/blue background alternating every 3 frames).
2. Field(s) per player - `func_22674` (P1) / `func_226ad` (P2), or `func_226f9` (double, two halves): `glTranslatef(x, 378, 0)` with **x = 28 (P1), 348 (P2); double: 66 and 311** (Y bottom to top: receptor line at 378 = 102 from the top). Each field calls `func_221a4` (5 receptors at x = 35,83,131,179,227 and y = 35 relative, 48 px between columns, pulse scale from table 0x81206EA indexed by the state 0x814AFCE.., sprites at 0x83C7226/0x83C907E/0x83C62FA/0x83C8152/0x83C9FAA with 60-byte stride = sprite objects drawn by `func_2EE67`), `func_22386` and `func_22502` (other field layers). `func_22dcd/22df1/22e20` -> `func_22840` (arrows/notes, 498 lines) draw the notes when the player is not in "erased" mode (bit 0x80).
3. Judgment: `func_22fd8(player)` - per-player counter (0x83CAED8 + 0x29*p) decrements, `glTranslatef(0, 240, ...)`, 'appear' scale and the judgment sprite (0x37 '7' and above).
4. Life/score bars: `func_21e47(life, 7)` at (176|573, 16) and `func_21885` at x=14 (P1) / x=560 (P2), y=52, size 40 (only if not battle and without flag 0x1000).
5. `func_23a4b(player)` (combo?), `func_21915`, `func_20134` (HUD/counter) and, if DAT 0x8148DD6, a time debug line (`func_26658`, "%d:%02d:%d").
State of the prototype (draw_play): uses **xs=200** for 1 player (wrong: P1 is at x=28, P2 at 348), receptors/arrows in its own coordinates and a text HUD; still missing: reading `func_2EE67` (sprite format), the gameplay sprite sheets (00.DAT/01.DAT/03.DAT?) and the drawings of `func_22386/22502/22840/21885/21e47/23a4b/21915/20134`. **Next step**: read the sprite format and the gameplay sheet loader.

## Update 31 - gameplay field with SP2 sprites and end of the old selection screen
- Confirmed (ELF func_21fe1/func_221a4/func_22386/func_22502/func_22840): the field sprites are `BGA/00.DAT` ARROW541..545.SP2 (tiles b1/b2 receptor, n1..n6 note, f1..f8 hit, p1..p8 touch), one per direction: DL=542, UL=541, center=545, UR=543, DR=544. Receptor b1 for 3/4 of the beat and b2 in the last quarter of each beat; pulse scale when touched (table 0x81206EA: 1,.97,.94,.91,.88,.85,.72,.69); p1..p8 in 16 frames, f1..f8 in 24; note n1..n6 = 6 frames per beat. Field at x0=28 (P1) / 348 (P2), double 66 and 311; receptor at (x0+35+48k, 67 from the top); notes rise from below.
- Inferred: which judgments fire the f tiles (here PERFECT/GREAT/GOOD); Y of the receptor line (67) deduced from translate 378+35 in Y-up.
- The old S_SELECT screen (list + level) was disabled: there is no fallback any more nor return from the result to it (goes back to attract); it is only reachable via the debug `-skip`.

## Update 32 - gameplay HUD: life, score, combo, judgment (ELF func_23a4b, func_21e47/21885, func_22fd8, func_21d11)
Confirmed in the ELF and the same in DOS (FUN_22FD8 has the same branches). Coordinates in Y-up; player P1 `0x83CAEB2`, P2 +0x29 per player (struct: +4 float life, +8 integer life 0..1000, +0x14 combo, +0x18 MISS streak, +0x26 last judgment, +0x24 timer).
- **Life** (`func_23a4b`): frame `gauge_b` (ARROW542, dec00.tga) at (32|352, 445); v=life/1000, n=(int)(v*256), q=n/6; the fill `gauge__` (same texture, v 32..64) is the rectangle x in [253-6q, 256] (P1) / [0, 6q+2] (P2) of the frame. Danger: life<180 lights `gauge_D` (event.tga) yellow every 2 frames (in the original it depends on option 0x86D6FC0; here always). The original blinks the fill on alternate frames (not reproduced: **inferred** that it does not matter).
- **Score** (`func_21e47(value,7)` at (176|573, 16)): 7 digits of font.tga (32x39 cells, v0=0.53125+(d/8)*0.15234375, u=(d%8)*0.125), back to front, step -24; label `score_` (ARROW541) at (14|560, 52).
- **Judgment** (`func_22fd8`): 25 frames; center (160|480|320 in double, 240); scale = table 0x812071E[25-t] for t>10 (1.52..1.0), otherwise 1; tile = perfec/great_/good__/bad___/miss__ (ARROW542) by the last judgment '5'..'9' ('A' = attack_, battle).
- **Combo** (`func_21d11`): inside the same scale, 70 px below; 3 digits of event.tga (32x48, v0=.375+(d/8)*.1875) with leading zeros, step -26 from +10; only with combo>=4 (white). Then the MISS streak (>=4) in red (1,0.2,0.2) (fixed: I had read the order of the glColor3f arguments backwards), continuing to the left (the translation is not undone). The `combo_` tile exists in the SP2 but is **not used** in the ELF nor in DOS during play (only the digits).
- NUM of ARROW542.SP2 says 43 but there are 45 tiles; `spr_parse` now counts the T lines. `sp2_find` compares only 6 letters (gauge_b/__/D collide): use `sp2_find_x`.
- game.c: HUD block of `draw_play` (Y-up drawing with `tile_up`). Not handled: option icons (func_21915), time/credit (func_20134), battle mode, 2 simultaneous players.

## Update 33 - BGA PATTERN sprites (ELF func_2efad type 3, func_2f4ef)
- Confirmed: `TYPE pattern a b c` (FUN_2F4EF): a -> [obj+0x24], b -> hi16 of [obj+0x28], c -> low byte. When drawing (type 3) the grid has **c columns x b rows**, step = (x+w, y+h) of tile 0 (translate x*column, row*height downwards); tile index = `(a != 0 ? row*c + column : b*column + row) + (int)(n*(1-frac))` modulo n. Before, only one tile was drawn (the Mobius Strip, 213, shows 2x2 identical 320x240 tiles).
- Type 1 (TILE): the tiles are drawn from last to first (inverted order, confirmed in the loop that decrements).
- game.c: `Spr.pb/pc/a`, `bga_draw_n`.

- Receptor drawing order (func_221a4, confirmed): DL(x35), center(x131), DR(x227), UR(x179), UL(x83), in three passes (b1/b2, p, f). Since they are separate sprites, Z/S/C stay behind and Q/E in front. game.c: `ORD` in `draw_play` (notes use the same order; inferred).

## Update 34 - couple (2 players) and audio sync
- Data (STEP/*.NOT, mask bit15->col0 ... bit6->col9): `X1` (E1/H1/H3) only has columns 0-4; `X2` (E2/H2/H4) has **both halves**: with 2 players the ELF loads **a single file** (`%d_H2.STF`, func_2cdfa) and P1 plays columns 0-4 and P2 columns 5-9 (in 101 the left half of H2 = H1). They are not two files. 1 player (P1 or P2) -> X1; 2 players -> X2; CRAZY: H3 (1P) / H4 (2P); DOUBLE (0x280): DB.
- game.c: `g_two` (couple) separate from `g_dbl` (DB). Couple uses two fields (x0 = 28 and 348), two `PiuPlayer` (`g_ps[2]`), life/score/judgment per player; P1 = Z Q S E C, P2 = numpad; ends when both reach zero. Before, H2 was treated as double (field at 66/311, a single player).
- Sync: `cb_pos` is the position already delivered to the device; the heard position is lower. `music_pos` now subtracts 2 audio blocks (`g_alat_ms`, ~46 ms) and the manual adjustment `-aoff MS` (positive delays the arrows).

## Update 35 - ULTRAMANIA (514) and P2 life
- Data: `514_H1`, `403_H1` and `505_H3` (1-player files) carry notes in columns 5-9 (in 514, H1 = H2, both halves). The original decides single/double/couple **only by the mode flags**; with 1 player only columns 0-4 are played. The old heuristic (any note in 5-9 turns on double) was removed and columns 5-9 only enter in DB (double) and couple.
- P2 life: mirror of P1 (range [0, 6q+3] with the texture inverted in x; red on the inner edge, near the center).
- `start_song` now writes a log line (stderr): song, step, sflag, mode, section.

- Blinking life (func_23a4b, confirmed): the `gauge__` fill is only drawn on frames where `0x83C627A % 2 != 0` (60 Hz frame counter), the `gauge_D` alert on the even ones (with life<180). game.c: `fr = now*60/1000` in the HUD block. The 60 Hz is inferred from the original's per-frame counter.

## Update 36 - title screen: credits / FREE PLAY and PRESS CENTER STEP (ELF func_20134)
- Confirmed: the title draws SP2 tiles (not a FONT crop, which included the "(" next to it): FREE PLAY = tile `freeply` (ARROW542) at (320, 13); with credits: `credit` (ARROW541) at (263,22) + number at x=360 (y=25); with a partial coin (coins % coins_per_credit != 0): `credit` at 239, credits at 338, `left__` "(" at (354,21), remainder at 358, `slash_` at (372,21), coins per credit at 388 (>=10) or 378, `right_` ")" at 400/390. Numbers = `func_200e3`: FONT digits 16x16 (u=(d%16)/16, v=96+(d/16)*16), step -12, right to left. PRESS CENTER STEP = `press_` at x=14 / 408, INSERT COIN = `insert` at 48 / 424 (y=22), blinking alpha.
- game.c: `draw_title_overlay` / `title_digits`.

## Update 37 - player texts on the MODE SELECT (ELF func_2b958, called by func_2bd93)
- Confirmed: they are layers of 099.BGA (mt01..mt08, MT10/MT11) drawn by `func_2647a(obj, layer, frame)` with frame = base + (anim % 30). Tables 0x81221EB.. (layers 13,14,12,11,19,17,18,16,15,20) and 0x8122213.. (bases 360,420,300,240,510). P1: already joined -> layer 13 base 360 (pointing: 14 base 420); not joined with credit -> 12 base 300 ("PRESS CENTER STEP TO JOIN"); without credit -> 11 base 240 ("NOT READY! PLEASE INSERT COIN(S)"). P2: 17 (18 pointing) / 16 / 15. Double branch (0x280): layers 19/20 base 510 ("PLEASE WAIT") - not handled. The TIME number (func_2a672, x=584 y=429) is still provisional text (the yellow digits are in 03.PNG of 099.DAT).
- game.c: new block in `draw_mode`.

## Update 38 - BPM/LEVEL and TIME with the original font (func_26658/26521, func_2a672/2a590)
- Text (func_26658(x, y, fmt, ...)): glyphs of FONT.PNG with **8x15** cells (32 per row; u=(c-0x20)%32*8, v=(c-0x20)/32*15), 8x15 quad with **Y up** (the base sits at y, the top at y+15), step 8, space draws nothing. BPM `%5.1f` at (0x128, 0x63), level `%d` at (0x130, 0x46) (inferred). Before, we used the 8x8 font with the base at 480-y, which sat too low.
- TIME (func_2a672(584, 429, n)): 2 digits of **02.TGA from the BGA itself (099/098)**, 48x40 cells, 5 per row, v0 = 162 + (d/5)*40, 48x40 quad at (584 - 40*i, 429) Y up, leading zeros.
- game.c: `orig_text`, `orig_time`.

## Update 39 - end of song: STAGE CLEARED / FAILED / BREAK, DANCE GRADE and GAME OVER (83.DAT) - ELF func_24de6, func_255d9, func_25430..25570, func_257cf, func_2568a
Confirmed in the ELF (same branches in DOS). The gameplay frame `func_24de6` sets the **phase** `0x83C62A6` and `func_255d9` (table 0x8052624) runs each one by playing frames of `83.BGA` with the frame counter `0x83C627A` (60 Hz):
- 0 playing; **1 FAILED**: frame c (up to 180) -> phase 3; **2 CLEARED**: frame 360+c (up to 240) -> state 5 (result); **3 GAME OVER**: frame 2280+c (up to 120) -> attract (zeroes flags); **4 STAGE BREAK**: frame 1800+c (up to 240) -> phase 3.
- STAGE BREAK (24F50/24F9A): sfx idx 7 channel 7 when **all** active players have life < 1 (with STAGE BREAK on, ~7 MISS in a row) or MISS streak > 50 (51 MISS, also when off). End of song (2505A): STAGE BREAK on -> CLEARED if any player's life >= 0.6, otherwise FAILED; off -> CLEARED (**inferred**). Option = `0x81206B6` (= STAGE BREAK of GAME OPTION, 0 = OFF).
- Result (`func_257cf`, frame 600 + c%480): they only appear with c > 10 score (7 digits, x=160|588, y=16), c>30/35/40/45/50/55 PERFECT/GREAT/GOOD/BAD/MISS/MAX COMBO (3 digits at x=176|488, y=343,293,243,193,143,78; event.tga digits 24x40, v0=192, step -22), c>70 the grade (tile grade_s if life>=1.0 and 0 MISS, otherwise A >=0.9, B >=0.8, C >=0.7, D >=0.6, F; at (118|522, 228), alpha = fade*0.5; fade = 1 until c=380 and then (400-c)/20). **SFX**: idx 8 channel 1 at c=30,35,...,55 (one per number); idx 9 channel 2 at c=70 (grade); at c=80 the grade voice idx 0x17..0x1B (RANK_A..F, channel 6) by life (>=0.9/0.8/0.7/0.6). Exits with c>400 or (c>250 and button) -> `func_2568a`.
- Next stage (`func_2568a`): stage++ ; EASY/HARD/CRAZY: continues if <3, or ==3 with the 3 lives >= 0.9 (extra stage); DOUBLE: ==3 requires the 1st life >= 1.0; BATTLE <3; REMIX/REMIXDB/REMIXCZ <2. Continues -> music wheel (state 0xA); otherwise GAME OVER.
- Not handled: lamps (func_27bb6), result music (func_27b2a, not identified), battle mode (ATTACK), credit/time on the result. game.c: `end_start/end_step/draw_end`, state `S_END`; test: `-end PHASE -endc FRAME`.

## Update 40 — Codes (modifiers) on the mode and music screens

**Confirmed (ELF `piu`, `func_2a72b` P1 / `func_2a973` P2, table `0x81219AA`, 16 × 8 keys; DL=1 DR=2 C=4 UL=8 UR=0x10):** each player's last 8 keys are compared against the table; once a sequence is found, the history is zeroed (`func_2a6c9`) and, if any flag changed, a beep plays (`ac_sound_play_effect(0,2,1)`). The table matches the user's code list (numpad: 7=UL 9=UR 1=DL 3=DR 5=center):

| # | Code | Effect on the flags |
|---|---|---|
| 0 | 55555555 | zeroes the high mode flags and modifiers (cancel) |
| 1 | 55975315 | VANISH (0x10, clears 0x100) |
| 2 | 55159357 | MIRROR (0x20) |
| 3 / 4 / 5 | 55153555 / 55355755 / 55755595 | 8X / 4X / 2X (0x08 / 0x04 / 0x02, exclusive) |
| 6 | 51535957 | RANDOM VELOCITY (0x40) |
| 7 | 79579579 | NONSTEP (0x80) |
| 8 / 9 | 55797955 / 55979755 | mode flags 0x1000 / 0x2000 (exclusive) |
| 10 | 13573159 | mode flag 0x4000 |
| 11 / 12 | 79513579 / 79351531 | RANDOM VANISH (0x100) / RANDOM STEP (0x200) |
| 13 | 17159395 | REMIXDB (REMIX only, 1 player, on the wheel) |
| 14 | 51535153 | REMIXCZ (REMIX only, phase 6) |
| 15 | 95197537 | exists in the table but is not dispatched in that function |

Entered on the MODE SELECT screens and on the music wheel; letters = P1, numeric keypad = P2 (only whoever joined). Modifiers persist during the credit and are zeroed at the end of the game / when returning to the title.

**Applied in game:** speed 2X/4X/8X (adjusts `g_speed`) and MIRROR (mirrors columns per half; double mirrors all 10). **Not applied yet** (only stored): VANISH, RANDOM VELOCITY/VANISH/STEP, NONSTEP, SYNCHRO/COUPLE/UNION. **Inferred:** beep index/channel (sfx 1, channel 2) and the bit names.

## Update 41 — Modifier icons

**Confirmed (ELF `func_21915`, called by `func_2be94`, `func_2c840` and `func_24de6`, i.e. MODE SELECT, music wheel and gameplay; `func_21885` = translate(x,y) + sprite + translate back):** the icons come from `ARROW541.SP2` (the original's index is the tile − 1), x = 4 (P1) / 600 (P2), Y up: speed 2X/4X/8X `accel_` (y 240) or random step `raccel` (240); vanish (200) or random vanish `v_rand` (200); mirror (160); random velocity `random` (120); nonstep `nonstp` (80). Mode flags: `sync__` (0x1000) or `couple` (0x2000) at (4, 280), without battle. In double only the player who joined shows the icons.
Test: `-mod 0x2f4` turns on modifiers on P1/P2.

## Update 42 — LOAD screens after the DANCE GRADE and modifier effects

**Confirmed (ELF `func_2c63f`, 0x2C689..0x2C6A2):** when returning to the music wheel, the game shows `TITLE\LOAD%d.PNZ` (`%d` = stages played, `DAT 0x81206CE`) through the same routine `func_2a533` as the T%03d screens; with 0 (coming from the mode select) there is no LOAD screen. `func_2d3df` compares the DOS date with 2000‑12‑27: after it, stage 1 uses LOAD4 instead of LOAD1 (LOAD1 is the "2000.12.27 Wed" announcement). LOAD2 = Banya, LOAD3 = "TRY BONUS STAGE", LOAD4 = "TRY NEXT STAGE". Duration (~1.5 s) inferred.

**Modifiers in game:** default speed is now 1X (2X/4X/8X = 2/4/8; the 1–8 keys still work), MIRROR, RANDOM STEP (column permutation), NONSTEP (hidden arrows), VANISH (arrows disappear in the top half of the screen), RANDOM VANISH, RANDOM VELOCITY (speed oscillates). The last five are **inferred** (only the bits and the icons are confirmed). SYNCHRO/COUPLE/UNION remain only stored.

**Cleanup (post‑Update 42):** removed the early test leftovers — the basic music selection (`S_SELECT`/`draw_select`/`diffs_avail`, `-skip` argument) and the basic result screen (`S_RESULT`/`draw_result`). The LOAD screen only appears after the DANCE GRADE (never from the MODE SELECT to the wheel).

**Music at the end of the phase (Update 43, confirmed in the ELF):** `func_24de6` only calls `func_294b9` (stop music) in the two STAGE BREAK branches (phase 4). In FAILED/CLEARED/DANCE GRADE/GAME OVER the song's own music **keeps playing** until it ends; only `func_2c63f` (wheel entry) and the attract stop it. There is no music file of its own for the result (only 001/002/003.AUD, used in title/mode/demo). Before, the code stopped the music when the song ended.

## Update 44 — Music covers (CD) outside the 26 sheets: 112, 226, 329 and 330

**Confirmed (ELF `func_2b609` loads, `func_2ae17` draws; DOS the same, table of 30 rows × 40 bytes at ELF `0x8121C77`/DOS `piu_le.exe` 0xF11D1):** each row = pointer to the sheet name, texture handle, 4 ids. The loader walks **30 rows** (`cmp edx,0x1e`), not 26. Rows 26–29 have ids only in specific positions and **reuse sheets**: 26 → `CD26.TGA` slot 1 = **226** (Another Heart 2nd), 27 → `CD26` slot 1 = **112** (Another Heart), 28 → **`CD06`** slot 3 = **329** (Extravaganza 3rd step, same image as 205), 29 → **`CD11`** slot 1 = **330** (Mix 2-7 3rd step). Rows 0–25 use `CD(n+1)`. CD27–CD30 do not exist in 90.DAT.

Cause of the bug (reported in play: "Extravaganza 3rd" without a DISC): `CD_TBL` in `game.c` had only 26 rows and the name was always `CD%02d` of the row. Fix: `CD_TBL[30]` + `CD_SHEET[30]`. Note: 112/226 fall into slot 1 of CD26, which (by inference) may be an empty/other image; the original game draws whatever the sheet has.

**"Hidden" songs (analysis summary):** there is no hide field; `func_2b70b` only lists section entries with byte0 ≠ 0 (`func_2b695` sets everything to 1 on the 1st stage; `func_2b6bc(section,id)` zeroes the played song until the next credit). No section = hidden (408 "Kiss"); the `Mix` ones only exist in REMIX/REMIXDB/REMIXCZ. **Implemented (game.c):** `SEnt.vis`; `wheel_list` only lists `vis`; `wheel_begin` with `g_stage<1` shows everything again (func_2b695); when choosing the song (func_2cdfa) it hides the id in the current section (func_2b6bc). Inferred: the per-credit reset uses `g_stage=0` (GAME OVER/ESC/attract); the exact point where the ELF zeroes `0x81206CE` was not located. In-game test pending.

## Update 45 — F1 (SERVICE) interrupts everything and leaving service resets the game

**Empirical (user request; not read in the code):** F1 outside service works like closing the active window: `svc_halt` stops the music, unloads the BGAs (`g_eb/g_ab/g_ab2`), zeroes players (`g_sflag`), stage, modifiers and `g_att`, and goes to state 0 (attract); while service is open the game does not advance (no `attract_step`/`play_update`). On leaving (ESC or the exit item, `svc_leave`) it returns to state 0 with `g_att=0`: without credit it starts with **81.DAT**; with credit/FREE PLAY the attract goes straight to the title (`cred_avail`). Before, the service only drew on top and the game/music kept going underneath.

## Update 46 — Line under PRESS CENTER STEP (and other sprite tiles)

**Cause (verified by magnified capture):** `tile_up` sampled the tile's exact crop; with `GL_LINEAR` and the window scaled (e.g. 800x600) the bottom edge pulled the texel of the neighboring sprite on the sheet, forming a gray dashed line under the text. **Fix:** the crop uses half a texel inwards (`+0.5/w`, `-0.5/h`) in `tile_up`; it applies to all tiles of `ARROW541/542`, without changing the quad position.

## Update 47 — Player frame (MODE SELECT) without seams; player texts also on the music wheel

**Seams (verified by capture + layer isolation):** the frame is layer 9 (`m05.spr`, fill, alpha 0.5) + layer 10 (`m06.spr`, border), each in 3 pieces (left cap + middle stretched 10→172 px + mirrored right cap). In the data the joints meet **exactly** (the solid part of the cap ends at u=170 → x=67, where the middle starts; the right one likewise at x=239). At 640x480 there is no seam; with the window scaled `GL_LINEAR` softens the cap's hard edge and 1 pixel is covered by both pieces (dark line). **Fix:** layer `m05.spr` is drawn with `GL_NEAREST` (restored to `GL_LINEAR` afterwards) and without the half-texel inset. `draw_tile` gained the half-texel inset (same as Update 46) on the other layers. What remains, in the border (`m06`), is a slightly lighter 1-pixel column at the seam: the pieces overlap by 16/10 px per original drawing (doubled antialiased edges), so it is faithful to the data.

**Music wheel:** `func_2c840` calls `func_2b958` (player texts) right after the TIME, then `func_21915` (icons) and `func_20134` (credits/FREE PLAY) — the same sequence as the MODE SELECT (`func_2bd93`); 098.BGA has the same layers 11–20. Before, the wheel left the frames empty. Now `draw_player_texts(dbl)` (shared) draws them; ELF tables 0x81221EB (layers 13,14,12,11,19,17,18,16,15,20) and 0x8122213 (bases 360,420,300,240,510). **Double branch (flags 0x280 = DOUBLE 0x80 / REMIXDB 0x200):** a player who has not joined uses layer 19 (P1) / 20 (P2) base 510 ("PLEASE WAIT!" text). **Not done yet:** the `func_20134` overlay (FREE PLAY/credits and PRESS CENTER STEP below) is only drawn on the title; in the ELF the MODE SELECT and the wheel also call it.


## Update 48 — PC build (`pumpy.exe`, 04/16/2001): packer, unpacking and dance pad check

**Confirmed (read in the file):** `pumpy.exe` (PE32, 152 KB) is packed: all sections named `BitArts`, entry `0x7A9000` in a multi-stage stub, only 17 KERNEL32 imports (the rest is resolved at runtime). The real code/data only exist in memory.

**Unpacking (empirical, via dump):** the game was opened as administrator, stayed on the pad error screen and a `.DMP` was generated by the Task Manager. `tools/dmp_extract.py file.DMP pumpy.exe output.exe` rebuilds the image (base `0x400000`, `0x7B2000` bytes) -> `E:\06_pc\pumpy_unpacked.exe` (704 procedures in Ghidra; **not executable**: IAT with the dump system's addresses, some entries point to AcLayers shims).

**Pad check ("PUMPY PAD"):** strings `Initialize PUMPY PAD...` (`0x42D144`, used at `0x401679`) and `Checking PUMPY PAD...` (`0x42D1C4`, `0x401831`). `0x4016F0` creates a window of class `KEY`, runs a message loop (up to 11 attempts, `Sleep(100)`) and `0x401990` (WM_KEYDOWN/0x111) stores up to 21 keys at `0x43275C`. `0x4015D0(player-1)` compares those keys (memcmp `0x429270`) with an expected sequence from an offset of the table `0x42D068` (strings `0x42D12C` P1 / `0x42D114` P2); returns 1 if it matches. Failure -> "Cannot detect your controller. try again" (`0x42D15C`); success -> "Successfully Detected Thank you for Play" (`0x42D188`). **Inferred:** the USB pad announces itself as a keyboard and types that sequence (challenge-response).

**Driver:** `gwiopm.sys` (GiveIO-like; `0x42D6B4`, uses at `0x40274F/0x4027E0/0x40298A`) installed via SCM (advapi32) -> requires administrator (error 740 when opened without elevation).

**Architecture differences vs Arcade:** OpenGL rendering (opengl32; no Glide), DirectInput/DirectSound/winmm, MP3 via DirectShow (`RenderFile`, `AUDIO\%03d.MP3`, `AUDIO\D%03d.MP3`), `piukey.cfg`, folders `bga\`, `eff\`, `title\`. RES v3 (header with `datasize` before `count`).


## Update 49 — PC x Arcade difference map and PC build data loading

Full table in `docs/PC_DIFF.en.md` (generated with `tools/map_pc_dos.py` and `tools/cmp_bga.py`). **Confirmed:** `pumpy.exe` is the same arcade code ported to Win32/OpenGL; PC and DOS functions match by order and by strings (PC `0x403BEF/0x403CB0/0x403DA0` = DOS `0x28469/0x285D4/0x284E0`; PC service `0x40F7E0…` = DOS `0x2E4FF…`). **New on PC:** pad check, `gwiopm` driver, MP3 via DirectShow, `eff\*.wav` via DirectSound, `piukey.cfg`, `c:\pump.ini`, `083.dat`/`083.mp3`. Implemented: `piu_res_load` reads RES v3; `piu_pc` uses `Eff/` and resolves paths case-insensitively on Linux/macOS.


## Update 50 — Sound on confirming a secret code (Arcade)

**Empirical (user):** confirming a code on the Arcade plays `2-1.WAV` (index 2), not `01-11` (index 1, previously inferred from `0x2A91B`). `code_press` now uses `sfx_play(2, 5)`, same as the PC.


## Update 51 — 5-1.WAV together with the grade voice (Dance Grade)

**Empirical (user):** when the grade is shown on the Dance Grade, `5-1.WAV` (idx 5, channel 3) plays together with the grade WAV (RANK_x / A..F). Applies to Arcade and PC (shared code in `PH_RESULT`, `c == 80`).


## Update 52 — Universal Remix BGA (PC: `BGAN.DAT`)

**Confirmed** in `pumpy.exe` `0x40965C..0x4096E6`: songs 116–119 and 219–224 (Remix without their own `.dat`: 117, 118, 119, 219, 220, 221, 223, 224) always use `BGAN.DAT`; the others try `BGA\%03d.DAT` and fall back to `116.DAT` if the file fails. `116.DAT` holds `116.BGA` (50 layers, 120 BPM, keys with r,g,b,a colour that alternate the background) + ANI sprites (`at01..15`, `s01..15`, `t01..15`). `bga_load` applies the rule on Arcade and PC (empirical: user). Checked by screenshot (blue tunnel → red); not played.
