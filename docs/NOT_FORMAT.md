# Formato STEP/*.NOT (PIU.EXE DOS, build Dec 18 2000)

Validado contra os **472** arquivos de `E:\06_pc\game\STEP` (tools/not_parse.py): tamanho == 0x88 + 4*count em 100%, linhas estritamente crescentes, maior linha 1560 (< 2048).
Fonte no binário: loader `0x21361` (abre `STEP\%s`, nome + ".NOT"@0x1200A4), decifra `0x29949`, expande `0x21251`.

## Layout (little-endian)
| Off | Tam | Campo |
|---|---|---|
| 0x00 | u32 | `count` — nº de registros |
| 0x04 | u32 | 0 (reservado) |
| 0x08 | f32[4] | **BPM por slot** (0x08..0x17; `DAT_001558A4[slot]`). 101_DB: [146.05,120,120,120,…]; steps com mudança de BPM têm vários valores (501_h2: 106.0, 131.9, 120…). `slot = DAT_003D0BF4` (C) |
| 0x18 | s32[4] | **offset de sincronia por slot, em centésimos de s** (`DAT_001558B4[slot]`) (C: usado em `pos − off*240/100`). 101_*=161, 102_*=530.  |
| 0x1C..0x37 | | zeros |
| 0x38 | u32 | `DAT_001558D4` = **N = linhas por batida** (C, derivado da fórmula de Y). 2 ou 4 nos 472 steps |
| 0x3C | u32 | sempre 4 (visto) — `DAT_001558D8` |
| 0x40 | u32 | varia por step (1,2,5,8,10,12,23…) — semântica desconhecida |
| 0x44 | char[0x44] | título (NUL-term; **cp949** nos steps coreanos) |
| 0x88 | `count` × 4 B | registros **cifrados** |

## Cifra (C — pseudocódigo de 0x29949)
```
key = 0xD2; for i in range(n): buf[i] ^= key; key = (key + 0x95) & 0xFF
```
Aplicada ao corpo inteiro (a partir de 0x88). O cabeçalho não é cifrado.

## Registro (C — 0x21251)
`struct { u16 row; u16 mask; }`
- `row`: índice de linha/tick (0..2047), crescente.
- `mask`: 13 bits lidos de **bit 15 → bit 3** (col = 15 − bit). Bit ligado ⇒ `table[row*14 + col] = '1'`.
- Colunas **0–9**: 10 pads (P1 0–4, P2 5–9; steps singles só usam 0–4). Colunas **10–12**: controle/grade (quase sempre ligadas: 194/194 em 103_E1) — significado não confirmado (I: linhas de batida/compasso).
- Bits 2..0 nunca usados (0 em todos os arquivos testados).
- Após expandir, o loader troca `'1'`→`0x39` ('9') nas colunas 0–9 de todas as 0x800 linhas (`0x15594C`, stride 14). **Atenção:** a ordem relevante é expand (grava '1') e depois varre trocando '1'→'9'; ver 0x21361 antes de reproduzir.

## Nomes de arquivo (C — strings @0x120691..)
`%d_H1 %d_E1 %d_DB %d_H4 %d_H2 %d_E2 %d_H3` (+ ".NOT"); em disco ocorrem também em minúsculas (`501_db.NOT`, `h1..h4`, `e1,e2`, `db`).
Existe um caminho de debug `.STF` (`run %d_e2.stf -demo %d`, `run step_name -d(ouble)`) — não há `.STF` no disco.

## Pendências
- Semântica de 0x0C/0x10/0x14, 0x18 e 0x38–0x40; tempo por linha (ticks → ms) — achar na função que consome `0x15594C`.
- Comparar com o formato do PREX 3 (E:\14_px3) para reaproveitar leitor.

## Mapeamento header → memória (C — loader 0x21361, lido do assembly)
- `fread(0x155874, 1, 8)`  ⇒ file[0x00..0x08) → `count`=0x155874, 0x155878
- `fread(0x1558A4, 1, 0x80)` ⇒ file[0x08..0x88) → 0x1558A4+(off−8). Ex.: 0x38→0x1558D4; título (0x44) → 0x1558E0
- `fread(0x1537EC, 1, count*4)` ⇒ registros (cifrados) → decifra in-place (0x29949) → expande (0x21251)
- Nome do arquivo: `strcpy(0x3D585A, nome)`; `0x2FC2D(nome, ".NOT")` (append); `sprintf(0x1557F4, "STEP\%s", nome)`; `fopen(path, "rb"@0x1200B1)`.
- Convenção Watcom: `fread(buf=EAX, size=EDX, n=EBX, fp=ECX)`.

## Estado da tabela de notas (C — 0x22840)
`0x15594C + row*14 + col`: byte. Desenho só para valor **> 0x32** (o loader converte '1'→'9', 0x39). Valores ≤ 0x32 = nota já consumida/inexistente (inferido: o julgamento reescreve a célula).

> **Nota de layout:** BPM e offsets não podem ocupar os mesmos bytes. Pelos endereços (`0x1558A4 + slot*4` para BPM e `0x1558B4 + slot*4` para offset) o f32[]/s32[] são arrays de **4 entradas cada** (0x08..0x17 = 4 BPM; 0x18..0x27 = 4 offsets; 0x28..0x37 = terceiro array de 4, ver abaixo). `slot` ∈ 0..3.

## Escala de tempo e scroll (C — assembly de 0x25064–0x25121 e 0x22840)
- Tick do jogo a **240 Hz** (`0x20514`: `inc [0x153738]`; `music_pos` = ticks desde o início do stage).
- `c14 = round( (pos − off[slot]*240/100) * bpm[slot] )` (FPU: `fild; fmul [0x1558A4+slot*4]; frndint; fistp [0x3D0C14]`).
  ⇒ **14400 unidades de c14 por batida** (240 ticks/s × 60 s / bpm × bpm). `c18 = c14/240` = 60 × batidas.
- Linha visível: `row = c18*N/60 = batida × N`. Janela: de `row0 = c18*N/60 − 2N` até `row0 + 10N` (de 2 batidas atrás a 10 à frente).
- Y de uma seta: `Y = c14*mult/240 − (60*row/N)*mult` = `60*mult` px por batida. `mult` = 1/2/4/8 (opção de velocidade).
- Compasso: 4·N linhas (marcadores de colunas 10–12 a cada 4N linhas em 101_DB com N=2 ⇒ 8).
