# PIU.EXE (DOS, build "Dec 18 2000") — mapa de símbolos

Endereços = VAs do LE original (também os do `piu_le.exe` aberto no REA).
Não dá para renomear no REA (Ghidra/Windows sem mutação) — **este arquivo é a fonte dos nomes**.
Confiança: C = confirmado no pseudocódigo, I = inferido (precisa de corroboração).

## Layout (LE, 4 objetos)
| Obj | VA base | Tamanho | Conteúdo |
|---|---|---|---|
| 1 | 0x10000 | 0x10 | stub (descartado no PE sintético) |
| 2 | 0x20000 | 0xE45F0 | código (jogo 0x20000–0xFB…, runtime Watcom/Glide/zlib até ~0x104600) |
| 3 | 0x110000 | 0x48 | dados |
| 4 | 0x120000 | 0x9060D0 | dados/bss (52 páginas com conteúdo) |

## Boot (C)
- `0xFC004` `_cstart` (Watcom/DOS4GW; INT21h, DPMI) → `0xFF54C`, `0xFF4FD`, `0xFF597`
- `0xFF4FD` `__CMain` → chama **`game_main` 0x21235**
- `0xFF597` runner de finalizadores (tabela 0x15350C..0x153518, entradas de 6 bytes)
- `0x21235` **game_main**: `game_init 0x20CC0` → `game_loop 0x20F37` → `game_shutdown 0x20D7C`

## game_init 0x20CC0 (C, ordem de chamada)
`0xFBEB2, 0xFBE3F, 0x20C5B, [DAT_0012B010=1 running], 0xFBF3A, 0xFBF93, 0x28297, 0x29218, 0x299D6, 0x20727, 0x2E61C, 0x21FE1, 0x264B1, 0x29E3B, 0x2D78A (se !=0 → state=0x1C service), 0x2A19B, 0x2FB34, 0x2F9C7, 0x2F95F(0,0x406E0000), 0x2FA4C, 0x266C0(&0x15374C), 0x20B84`
Nomes: ainda não atribuídos (0xFB… = runtime/Glide init provável — I).

## game_shutdown 0x20D7C (C)
`0x2056F, 0xFBE91, 0xFBFA3, 0x2FB9A, 0x294B9, 0x282EA, 0x26516, 0x2E65B`
(espelha a init na ordem inversa — I)

## game_loop 0x20F37 (C)
Globais: `running`=0x12B010, `game_state`=0x153710, `key`=0x153714 (scancode<<8), `input_flags`=0x153718, `debug_overlay`=0x153734, `dbg2`=0x15372C.
Por frame: `0xC76BC` (begin frame) → `0x20DCD` (leitura de input, retorna scancode<<8 | flags) → teclas:
- 0x2D00 + flag 0x80000 → sai do loop (X; I)
- 0x2900 → `0x26742`; 0x4200 → alterna `debug_overlay` (F8; I); 0x0100 → `game_state=0` (ESC; I); 0x4300 → `0x2087F` (F9; I)
- `0x26759()` decide `0x20E0E` vs `0x26948`; `0x27D08` (se state!=2); `0xC4FE0`
- **dispatch por `game_state`** (tabela abaixo) → `0x205C4` (overlay) → `0xC4F2C`, `0x20C1E`, `0xDB26E` (end frame/flip)

### Estados (game_state → handler) (C)
| state | handler | obs |
|---|---|---|
| 0,3,6-9,12,15,0x12,0x15,0x17-0x1B,0x1E | fallback 0x26611 / 0x20E7A | sem handler próprio |
| 1 | 0x247E2 | |
| 2 | 0x255D9 | (0x27D08 não roda no state 2) |
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
| 28 (0x1C) | 0x2E46A | tela de serviço/teste (entrada) |
| 29 (0x1D) | **0x2E4D4** | tela de serviço: switch em `DAT_0012CB80` (9 sub-estados: 0x2D847, 0x2D954, 0x2DB18, 0x2DC59, 0x2DCDC, 0x2DFC0, 0x2E167, 0x2E2BC, 0x2E450); desenha "PUMP IT UP (R%d/v%d.%02d)" e "Dec 18 2000" via 0x26658(x,y,str) |
| 31 (0x1F) | 0x25E90 | |
| 32 (0x20) | 0x25F27 | |

Funções utilitárias já vistas: `0x26658(x,y,fmt,...)` = draw text (I); `0xFB6E2(n)` = stack check do Watcom (prólogo; ignorar).

## Músicas (STAGE.CFG) (C, fonte: arquivo)
Faixas de ID: 1xx = 1st, 2xx = 2nd, 3xx = 3rd, 4xx = 4th, 5xx = desta versão (informado pelo usuário).

## Próximos alvos
1. Parser de `stage.cfg` (string @0x120077) e formato do `.NOT`.
2. Loader de BGA/áudio/TITLE; camada Glide (gr* → OpenGL).
3. Handlers de estado de gameplay (identificar qual state = seleção, jogo, resultado).

---
## Atualização 2 — STAGE.CFG e convenção de chamada

### Convenção de chamada (C — observado em 0x20D37: `mov eax,0x120077; call 0x2A19B`)
Watcom register convention (`-3r`): args em **EAX, EDX, EBX, ECX**, resto na pilha. O decompilador do Ghidra **perde esses args**
(mostra `FUN_xxx(void)`). Sempre confirmar com `read_bytes`/assembly antes de assumir a assinatura. `0xFB6E2(n)` = `__CHK` (stack check), ignorar.

### Carga do STAGE.CFG
- `0x2A19B` **stage_cfg_load(const char *path)** (C): `fopen`(0xFBA01) → `while fgets`(0xFCACF) → tokeniza (0xFC6A5 → `DAT_006E1780`) → `strcmp`(0xFC570) com 3 palavras-chave → handlers
  `0x2A039`, `0x2A0CA`, `0x2A16A` (I: `T`, `MODE`, `S`, a ordem exata ainda não verificada) → `fclose`(0xFBC75) → laço final: `DAT_006E163A[i*2] = DAT_006E163C[i*2] - DAT_006E1638[i*2]` (duração = fim − início, int16 stride 4; I)
- Contadores globais: `0x12C380`, `0x12C384`, `0x12C388` zerados no início (I: nº de T / modos / entradas).
- Chamada em `game_init` com `"stage.cfg"` (@0x120077) em EAX.

### Formato STAGE.CFG (C — lido do arquivo E:\06_pc\game\STAGE.CFG)
```
T <id>:<bpm>:<título>        105 músicas (id 1xx=19, 2xx=24, 3xx=30, 4xx=15, 5xx=17)
MODE<TAB><nome>              EASY HARD DOUBLE REMIX REMIXDB CRAZY REMIXCZ MENU END
<TAB>S<TAB><nível><TAB><id>  entradas da seção (EASY=61 HARD=69 DOUBLE=70 REMIX=18 REMIXDB=15 CRAZY=37 REMIXCZ=4 MENU=5)
```
IDs: 1xx=1st, 2xx=2nd, 3xx=3rd, 4xx=4th, 5xx=esta versão. Os 5xx aparecem em EASY (509–517) e HARD/DOUBLE, etc.
Cada id liga a `STEP/<id>_<DB|E1|E2|H1|H2…>.NOT` (padrão visto: 101_DB.NOT, 101_H1.NOT, 103_E1.NOT…).

---
## Atualização 3 — parser do STAGE.CFG fechado + loader de steps

### 0x2A19B stage_cfg_load (C) — handlers
O nome do MODE **não é lido**: modos são **posicionais** (ordem no arquivo: 0 EASY,1 HARD,2 DOUBLE,3 REMIX,4 REMIXDB,5 CRAZY,6 REMIXCZ,7 MENU,8 END; máx 9).
- `0x2A0CA` linha **T** (I: keyword não lida): `song[DAT_0012C380]`: stride 40 B @0x6DFC28: `+0 int id` (atoi 0xFC4F5), `+4 float bpm` (strtod 0xFC8DC), título via 0xFBE3F; `DAT_0012C380++`.
- `0x2A039` linha **S**: entrada `DAT_0012C384` stride 4 B @0x6DF6B0: `[+1]=level`; se level==0 → `[+0]=0,[+2]=0`; senão `[+0]= (level>=12)`, e se >=12 `level-=10`; `[+2]=u16 id`. `DAT_0012C384++`.
- `0x2A16A` linha **MODE**: `mode_start[DAT_0012C388]` (u16, stride 4 @0x6E1638) = `DAT_0012C384`; `DAT_0012C388++` (cap 9). Fim: `mode_len[i]=start[i+1]-start[i]` (@0x6E163A).
(Quais das 3 strings 0xFC570 compara cada handler ainda não foi lido dos bytes; atribuição T/S/MODE é pela forma dos handlers.)

### Loader de step `0x21361` (C)
`sprintf(path,"STEP\%s", DAT_003D585A /*nome*/)` → fopen → 3 freads (header; ver NOT_FORMAT.md) → fclose → `0x21251` (decifra+expande) → varre 0x800 linhas×10 cols trocando '1'→'9'. Falha: `0x266C0("fail %s %s")`.
Strings de recursos: `AUDIO\%03d.AUD`, `AUDIO\D%03d.AUD`, `BGA\%03d.DAT`, `T%03d`(+.PNZ em TITLE), `.\cap\p5_%03d.TGA` (screenshots), `font.tga`, `event.tga`, `BAR01.TGA`, `CD01.TGA`.
Formatos de dados em disco: `.NOT` (steps), `.AUD`, `.DAT` (BGA, + BGA_extracted/ já extraído), `.PNZ` (TITLE; provável PNG comprimido — build embute libpng/zlib).

---
