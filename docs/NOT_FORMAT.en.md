**Languages:** [Português (BR)](NOT_FORMAT.md) · **English** · [Español](NOT_FORMAT.es.md)

# STEP/*.NOT format (PIU.EXE DOS, Dec 18 2000 build)

Validated against the **472** files in `E:\06_pc\game\STEP` (tools/not_parse.py): size == 0x88 + 4*count in 100%, strictly increasing rows, largest row 1560 (< 2048).
Source in the binary: loader `0x21361` (opens `STEP\%s`, name + ".NOT"@0x1200A4), decrypt `0x29949`, expand `0x21251`.

## Layout (little-endian)
| Off | Size | Field |
|---|---|---|
| 0x00 | u32 | `count` — number of records |
| 0x04 | u32 | 0 (reserved) |
| 0x08 | f32[4] | **BPM per slot** (0x08..0x17; `DAT_001558A4[slot]`). 101_DB: [146.05,120,120,120,…]; steps with BPM changes have several values (501_h2: 106.0, 131.9, 120…). `slot = DAT_003D0BF4` (C) |
| 0x18 | s32[4] | **sync offset per slot, in hundredths of a second** (`DAT_001558B4[slot]`) (C: used in `pos − off*240/100`). 101_*=161, 102_*=530.  |
| 0x1C..0x37 | | zeros |
| 0x38 | u32 | `DAT_001558D4` = **N = rows per beat** (C, derived from the Y formula). 2 or 4 in the 472 steps |
| 0x3C | u32 | always 4 (seen) — `DAT_001558D8` |
| 0x40 | u32 | varies per step (1,2,5,8,10,12,23…) — unknown semantics |
| 0x44 | char[0x44] | title (NUL-terminated; **cp949** in the Korean steps) |
| 0x88 | `count` × 4 B | **encrypted** records |

## Cipher (C — pseudocode of 0x29949)
```
key = 0xD2; for i in range(n): buf[i] ^= key; key = (key + 0x95) & 0xFF
```
Applied to the whole body (from 0x88). The header is not encrypted.

## Record (C — 0x21251)
`struct { u16 row; u16 mask; }`
- `row`: row/tick index (0..2047), increasing.
- `mask`: 13 bits read from **bit 15 → bit 3** (col = 15 − bit). A set bit ⇒ `table[row*14 + col] = '1'`.
- Columns **0–9**: 10 pads (P1 0–4, P2 5–9; single steps only use 0–4). Columns **10–12**: control/grid (almost always on: 194/194 in 103_E1) — meaning not confirmed (I: beat/measure lines).
- Bits 2..0 never used (0 in every tested file).
- After expanding, the loader swaps `'1'`→`0x39` ('9') in columns 0–9 of all 0x800 rows (`0x15594C`, stride 14). **Careful:** the relevant order is expand (writes '1') and then sweep swapping '1'→'9'; see 0x21361 before reproducing.

## File names (C — strings @0x120691..)
`%d_H1 %d_E1 %d_DB %d_H4 %d_H2 %d_E2 %d_H3` (+ ".NOT"); on disk they also occur in lowercase (`501_db.NOT`, `h1..h4`, `e1,e2`, `db`).
There is a `.STF` debug path (`run %d_e2.stf -demo %d`, `run step_name -d(ouble)`) — there is no `.STF` on disk.

## Open items
- Semantics of 0x0C/0x10/0x14, 0x18 and 0x38–0x40; time per row (ticks → ms) — find it in the function that consumes `0x15594C`.
- Compare with the PREX 3 format (E:\14_px3) to reuse the reader.

## Header → memory mapping (C — loader 0x21361, read from the assembly)
- `fread(0x155874, 1, 8)`  ⇒ file[0x00..0x08) → `count`=0x155874, 0x155878
- `fread(0x1558A4, 1, 0x80)` ⇒ file[0x08..0x88) → 0x1558A4+(off−8). E.g. 0x38→0x1558D4; title (0x44) → 0x1558E0
- `fread(0x1537EC, 1, count*4)` ⇒ records (encrypted) → decrypt in place (0x29949) → expand (0x21251)
- File name: `strcpy(0x3D585A, name)`; `0x2FC2D(name, ".NOT")` (append); `sprintf(0x1557F4, "STEP\%s", name)`; `fopen(path, "rb"@0x1200B1)`.
- Watcom convention: `fread(buf=EAX, size=EDX, n=EBX, fp=ECX)`.

## Note table state (C — 0x22840)
`0x15594C + row*14 + col`: byte. Drawn only for value **> 0x32** (the loader converts '1'→'9', 0x39). Values ≤ 0x32 = note already consumed/nonexistent (inferred: judgment rewrites the cell).

> **Layout note:** BPM and offsets cannot occupy the same bytes. From the addresses (`0x1558A4 + slot*4` for BPM and `0x1558B4 + slot*4` for offset) the f32[]/s32[] are arrays of **4 entries each** (0x08..0x17 = 4 BPM; 0x18..0x27 = 4 offsets; 0x28..0x37 = third array of 4, see below). `slot` ∈ 0..3.

## Time scale and scroll (C — assembly of 0x25064–0x25121 and 0x22840)
- Game tick at **240 Hz** (`0x20514`: `inc [0x153738]`; `music_pos` = ticks since the start of the stage).
- `c14 = round( (pos − off[slot]*240/100) * bpm[slot] )` (FPU: `fild; fmul [0x1558A4+slot*4]; frndint; fistp [0x3D0C14]`).
  ⇒ **14400 c14 units per beat** (240 ticks/s × 60 s / bpm × bpm). `c18 = c14/240` = 60 × beats.
- Visible row: `row = c18*N/60 = beat × N`. Window: from `row0 = c18*N/60 − 2N` to `row0 + 10N` (from 2 beats behind to 10 ahead).
- Y of an arrow: `Y = c14*mult/240 − (60*row/N)*mult` = `60*mult` px per beat. `mult` = 1/2/4/8 (speed option).
- Measure: 4·N rows (column 10–12 markers every 4N rows in 101_DB with N=2 ⇒ 8).
