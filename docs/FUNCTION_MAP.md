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
## Atualização 4 — renderização das setas (0x22840)
`0x22840` **draw_arrows(player)** (C; player em EAX: coluna-base 0 = P1, 5 = P2). Flags de opção do jogador: `DAT_003D0C1C` (P1) / `DAT_003D5800` (P2).
- Multiplicador de velocidade `mult` (C): `flags&8`→8; senão `flags&4`→4; senão `flags&2`→2; senão 1.
- Outras flags (I): `0x10`/`0x100` escolhem variante de sprite (`0x22750` / `0x2279F`); `0x200` e `DAT_0012B028&0x40` mesclam flags com as de `DAT_0015C94C/950` (efeito: espelho/troca de lado — não confirmado).
- Janela: `row0 = DAT_003D0C18*DAT_001558D4/60 - 2*DAT_001558D4` (mín 0); varre `[row0, row0 + 10*DAT_001558D4)`. `DAT_003D0C18` = tempo da música (C: usado como contador de 1/60; unidade a confirmar).
- Y da seta = `DAT_003D0C14*mult/240 - (row*60/DAT_001558D4)*mult`.
- 5 passes por coluna, com translate X: col0→0, col1→48, col2→96, col3→144, col4→192 (C, floats 0/48/96/144/192). Ordem de varredura no código: 0,2,4,1,3.
- Helpers (I): `0xC6EF8` translate, `0xC4FE0` push matrix, `0xC4F2C` pop, `0xC7944` set color, `0x2EE67` desenha sprite da seta (args de coluna em registrador, perdidos no decompilador). Termina com chamada em cauda `0x224FB`.
- Consumidores de `0x15594C`: 0x214A1 (57 B), 0x21762 (291 B), vários trechos em 0x21479–0x216EF (inclui o expand/loader), 0x22840.

---
## Atualização 5 — frame de gameplay (0x24DE6) e tempo
`0x24DE6` **gameplay_frame** (C; handler de estado de jogo; termina com cauda `0x224FB`). Não é chamado direto do switch: um dos handlers de `game_state` (candidatos 0x247E2/0x255D9/0x25639) o chama — a confirmar.
- `0x205B4` **get_music_pos** = `return DAT_00153738` (C).
- Flags de modo `DAT_0012B028` (ushort): bit0 = P1 ativo, bit1 = P2 ativo (C); `0x40` = demo/attract (I); `0x80|0x200` = modo duplo/co-op (I, `&0x280` copia dados P1→P2); byte2 `&0x10` = modo especial (I).
- Cálculo de tempo por frame (C):
  `pos = max(get_music_pos(), DAT_0012B030)`; `DAT_0012B02C = max(pos,0)`;
  `DAT_003D0C14 (scroll) = round( DAT_0012B02C − offset[DAT_003D0BF4]*0xF0/100 )`; `offset[]` = `DAT_001558B4` = **int[8] em file 0x18..0x38 do .NOT** (101_*: offset[0]=161) ⇒ **unidade de 0xF0 (=240) por unidade-base; offset em centésimos**;
  `DAT_003D0C18 = scroll / 0xF0`.
  Debug (`DAT_0015372C`): imprime `"%02d:%02d:%02d"` com `pos/0x3840`, `(pos/0xF0)%60`, `(pos%0xF0)*100/0xF0` ⇒ **pos está em 1/240 s** (0x3840 = 14400 = 60 s × 240) — **C**. Logo `DAT_003D0C18` = segundos inteiros e `DAT_003D0C14` = 1/240 s.
  (Conflito aberto: com 1/240 s, `row0 = c18*DAT_001558D4/60 − 2*N` dá poucas linhas/s; a fórmula de Y em 0x22840 pode usar `DAT_001558D4` como outra grandeza (ex.: pixels/beat). Reconciliar lendo 0x22840 em assembly antes de portar.)
- Fim de música: se `pos` chega ao fim → `DAT_00153710 = 10` (até 2 vezes via `DAT_0012B024`) ou `= 4` (I: tela de resultado / próximo stage).
- `DAT_003D0BEC`++ por frame (contador de animação); `DAT_003D0BD0`++ (frames de jogo); `DAT_0012B034` = flag de "primeiro frame" (inicializa áudio/vídeo: 0x294B9, 0x2949F, 0x205A4, 0x2058F).
- Vida/gauge (I): P1 em `DAT_003D5808..`, P2 `DAT_003D5831..` (valores de teste 500/400/300/200/100/900 setados quando `DAT_00153714==0x8600`, F12-like cheat/teste; float `DAT_003D580C`=1.0 = barra cheia; fim de jogo se gauge < 1 e `!(flags&0x40)`).
- Ordem de desenho por frame (C): bg `0x2643C` → (P1) `0x22674`, (P2) `0x226AD` (ou `0x226F9` em duplo) → setas `0x22DCD`/`0x22DF1`/`0x22E20` (por jogador, se opção `&0x80` == 0) → `0x22FD8` → barras (translate X=176.0 / 572.0, Y=16.0; `0x21E47`, `0x21885`) → `0x23A4B` (por jogador; I: julgamento/combos) → `0x21915`, `0x20134`.

## Transformações de opções sobre a tabela de notas (C)
- `0x214A1` **copy_p1_to_p2**: para as 0x800 linhas copia cols 0–4 → cols 5–9 (`0x155951 + r*14 + c = 0x15594C + r*14 + c`, c<5).
- `0x21762` **mirror_chart**: para cada linha, in-place e **em sequência** (ordem importa!): se col src=='9' então dst='9' com (src→dst): 0→8, 1→9, 2→7, 3→5, 4→6, 5→3, 6→4, 7→2, 8→0, 9→1. Só escreve '9', nunca apaga. Reproduzir a ordem exata.
- Outras transformações entre 0x214D9–0x216EF (refs a 0x15594C) ainda não lidas.
- O julgamento NÃO referencia `0x15594C` diretamente (usa ponteiro/offset) — achar em 0x23A4B/0x22DCD.

## Atualização 6 — conflito de escala resolvido (C)
Ver NOT_FORMAT.md §"Escala de tempo e scroll". Tick por interrupção: `0x20514` (incrementa `0x15373C`, `0x153740`, `0x153738`; chama `0x26CE5`, `0x29335`, e a cada 2 ticks `0x27C23`/`0x20424`/`0x27D08`). `0x20514` é a ISR/callback de timer a 240 Hz.
`0x24C61–0x24D63` **stage_start** (C): lê id do nome em `0x3D6380`, carrega `BGA\%03d.DAT` (fallback `BGAN.DAT` p/ ids 0x74–0x77 e 0xDB–0xE0), espera `0x2057F() >= 0x4B0` (1200 ticks = 5 s), `0x27B2A(0)`, zera tick (`0x2058F`) e `set_music_pos(0)` (`0x205A4`), `game_state = 2`.

---
## Atualização 7 — entrada (pads), vida e estados
### 0x27D08 poll_pads (C) — chamado pela ISR 0x20514 (state==2, a cada 2 ticks) e pelo game_loop (state != 2)
`0xFC73A()` lê a placa de I/O (inp() em porta; recomp: trocar por teclado/gamepad). Bytes lidos: `3D64FC..3D6502`; `AH` = byte de serviço (ativo-baixo).
Máscara de botões atual `DAT_003D6514` (`+0x153720`), pressionados neste tick `DAT_003D6510` (OR em `DAT_00153718`), soltos `DAT_003D650C` (`0x15371C`), anterior `DAT_003D6518`.
| Bit | Origem (C) |
|---|---|
| P1 0x08 / 0x10 | `[64FC] & 0x0F` / `& 0xF0` |
| P1 0x04 / 0x01 | `[64FD] & 0x0F` / `& 0xF0` |
| P1 0x02 | `[64FE] & 0x0F` |
| P2 0x800 / 0x1000 | `[6500] & 0x0F` / `& 0xF0` |
| P2 0x400 / 0x100 | `[6501] & 0x0F` / `& 0xF0` |
| P2 0x200 | `[6502] & 0x0F` |
| 0x100000 / 0x10000 / 0x20000 / 0x40000 | `AH&4==0` / `AH&2==0` / `AH&0x40==0` / `AH&0x80==0` (serviço/moeda/teste — I) |
Em `game_state==2` (e `DAT_0012B028 byte2 & 0x10 == 0`) ou `==0x1D`: para cada bit P1 (0x01..0x10) pressionado/solto chama `0x27BE9`; para P2 (0x100..0x1000) chama `0x27C1C` (**candidatos a julgamento/efeito de pad**; índice do pad passa em registrador — perdido no decompilador; ler assembly).
Mapeamento coluna→bit provável: col0=0x01, col1=0x02, col2=0x04, col3=0x08, col4=0x10 (I, por ordem de bits; confirmar em 0x27BE9).

### 0x23A4B draw_life_gauge(player) (C; player em EDX)
P1 em X=32.0/Y=444.0, P2 em X=352.0. Vida: `DAT_003D5810` (P1) / `DAT_003D5839` (P2); pisca quando `< 0xB4 (180)`. Estado "em música" = `game_state == 2` (C: `stage_start` define 2; ISR só roda poll/lógica de jogo nesse estado).

### Mapa resumido de estados confirmados
- `2` = em música (stage_start 0x24C61.., ISR 240 Hz, poll_pads, gameplay_frame) — C/I
- `0x1D` = tela de serviço/teste (0x2E4D4) — C
- `10`, `4` = destinos ao fim da música (I: resultado / próximo stage)


## Atualização 8 — Julgamento (confirmado em assembly, capstone)

Correção: 0x27BE9/0x27C1C NÃO são julgamento; só setam bits de moeda/serviço na porta 0x2A4 (via 0xFC743). Ghidra não definiu funções em 0x22E5F–0x24D64 ("not_in_procedure"); ler por capstone.

- apply_judgment 0x23229 (EAX=player 0/1, DL=nota '5'..'9'): switch por jump table 0x23215 (DL-0x35):
  '5' Perfect(0x234D5): contador +0x12 (0x3D5822), combo +0x0C++, vida +=(+0x04*12/1000), +0x14 combo-bonus; combo>3 => +0x3E8 em 0x3D5808
  '6' Great(0x2354A): contador +0x14 (0x3D5824), combo++, bonus +0x1F4
  '7' Good(0x235B8): contador +0x16 (0x3D5826), combo=0
  '8' Bad(0x235E6): contador +0x18 (0x3D5828), combo=0, vida -= 0x32
  '9' Miss(0x2362A): contador +0x1A (0x3D582A é o ponteiro de linha, ver abaixo; contador de miss em outro offset), combo=0, vida -= f(vida)
  Struct do jogador: base 0x3D5810, stride 0x29. +0x00 vida(0x3D5810), +0x04 (0x3D5814), +0x0C combo(0x3D581C), +0x10 (0x3D5820), +0x1A linha atual (16.16, 0x3D582A), +0x43 (P2 linha 0x3D5853).
- Células da tabela 0x15594C (stride 14 = 1 linha; 10 colunas; P1 0..4, P2 5..9): '0' vazio, '1' nota, '9' nota pendente, '5'..'8' = Perfect..Bad já julgada. Nota da linha = max(char) das colunas (0x22F21 P1 col 0-4, 0x22F5D P2 col 5-9, 0x22F9C todas).
- Pressão: no frame, para cada bit de 0x153718 (P1) / 0x153719 (P2) (pressionado neste frame), se a célula da linha atual 0x3D582A>>16 == '9' => troca pela nota calculada (BL) = classificação por distância:
  d = |(linha*60/N) - 0x3D0C18| (0xFC4D5=abs); d<=T0 => '5'; d<=T1 => '6'; d<=T2 => '7'; senão '8'.
  Limiares (0x24BA6.., por BPM do slot b=[0x1558A4]): Tk = round((b*Mk/120 + Ck) * 0.5) com (M,C)=(8,4)->0x3D0BE4 Perfect, (16,5)->0x3D0BD8 Great, (24,6)->0x3D0BF0 Good, (32,7)->0x3D0BDC Bad. Unidade: a mesma da posição de scroll em 0x22840 (60 px/batida).
- Mapeamento coluna<->bit P1: 0x01->[+0], 0x02->[+4], 0x04->[+2], 0x08->[+1], 0x10->[+3]; P2: 0x01->[+5(0x155951)]... (ver código 0x242E7).
- Depois que a linha passa (0x3D582A>>16 > 0x1557EC = última linha julgada): nota=max da linha (0x22F9C); se '5'..'8' => apply_judgment; células ainda '9' viram Miss ('9'). Demo (flag 0x40): nota aleatória '5'..'7' via 0x23E2A (rand%10: <7 Perfect, <9 Great, senão Good).
- Linha atual: 0x23ED4 (por frame) calcula 0x3D0BD4 = round((12B02C - tempo)*bpm/240 ...)*N/60+1 (clamp 0..2047, monotônica) e guarda em 0x3D582C; helpers 0x23E69 (recalcula slot de BPM 0x3D0BF4 comparando 0x1558C4/0x1558C8 = pontos de troca de BPM do 3º array) .
- Resets 0x23155/0x23194/0x231D6: células != '0' voltam a '1' (colunas 0-4, 5-9, 0-9) — usadas quando o jogador erra (stage break/continue).


## Atualização 9 — Formatos de arquivo (confirmado, 100% dos arquivos validam por Adler-32)

Checksum em tudo = Adler-32 (0x298D1/0x2992E, mod 0xFFF1).
- Loader genérico 0x289B0 (path EAX, buffer EDX): fread u32 H, depois ate 0x300000 bytes; descifra 0x2896A; confere adler.
  AUDIO\%03d.AUD (0x29263), AUDIO\D%03d.AUD (0x29295), TITLE\%s.PNZ (0x2A533) usam esse loader.
  Cifra: p[i] = bitrev8(c[i]) XOR KEY[(H+i)&0x3FF]; KEY = 1024 bytes em 0x12B13C (dados iniciais, obj4); H == adler32(p).
  Resultado: .PNZ = PNG; .AUD = MP3 (ID3v2). tools/piu_unpack.py (322/322 OK).
- Container BGA\*.DAT "RES " (res_load 0x29A45; res_fopen 0x26290 le o diretorio das 50 entradas de 0x1000-ish ...): header 0x18 {magic,ver=2,count,datasize,?,adler(data cifrado)};
  diretorio count*28 cifrado com XOR(0xD2,+0x95) (0x29949) {name[16],size,offset,adler(plain)}; dados com a mesma XOR. Conteudo: .PNG(3380), .SPR(3583), .BGA(114 = script do BGA), .SP2(5). tools/piu_res.py (117/117 OK).
- Ainda abertos: formatos .BGA, .SPR, .SP2; TGA (cap\p5_%03d.TGA, font.tga) em disco? (nao ha); camada Glide.


## Atualização 10 — .BGA / .SPR / .SP2 (assembly + tools/piu_bga.py: 114/114 .BGA e todos .SPR/.SP2 parseiam)

BGA (loader 0x2628F -> res_load 0x29A45, entrada "<nome>.bga"): header 16B {'BGA\0', u32 @obj+4 (120/146/180...: parece BPM, nao confirmado), u32 @obj+8, u32 @obj+0}.
  Depois ATE 50 layers (o loader le 50 mas o arquivo pode ter menos: 102.BGA tem 40; fim = EOF): name[64] (.spr/.tga do layer), u32 N, N keyframes de 44 B. Tamanho = 16 + 68*L + 44*sum(N), L<=50.
  Keyframe (render 0x25FA5; 0x2643C(obj,frame) percorre os 50 layers; 0x2647A(obj,layer,frame) um so):
    s16 time, s16 active (0 => nao desenha/termina), f32 x, y, pivx, pivy, scale, rot, r, g, b, a  (cores x 0x12B0B0/0x12B0B4 = 1.0)
    Antes da 1a key nada e desenhado; entre keys i-1 e i interpola linear (0x25F88: a+(b-a)*t, t=(f-t0)/(t1-t0)) todos os campos; apos a ultima key => retorna 1 (fim).
    Matriz: translate(0,480) -> translate(x,-y) -> pivot/rotate(rot, Z) -> scale(s,s,1) (Y invertido, tela 640x480); glColor(r,g,b,a); sprite desenhado por 0x2EFAD(sprite=layer+0x44, t).
SPR/SP2 (texto, CRLF): "TYPE <TILE|ANI|PATTERN a w h|IMAGE>" / "NUM n" (2 arquivos omitem a palavra NUM) / n linhas "T arquivo.tga x y w h u0 v0 u1 v1" (pixels, textura 256x256; ".tga" mapeia para o .PNG de mesmo nome no RES).
  0x2EFAD (desenho por tipo, campo u16 em +0x20): tipo 0/1 (TILE/IMAGE) desenha todas as entradas; 2 (ANI) escolhe frame floor(t*n); 3 (PATTERN a w h) w*h tiles repetindo/rolando os frames com parametro a (inferido).
  0x2EEFF desenha 1 quad: vertices de tela (words em +0x24,+0x26,+0x28,+0x2A; Y=480-y), UV float em +0x2C..+0x38.
  SP2 (arrow541..545): linhas de 11 tokens "T tag arquivo x y w h u v w h" (tag extra; fonte provavelmente u,v,w,h) -- nao confirmado.
Em aberto: u32 @obj+8/@obj+0 do header BGA, ligacao BGA<->tempo da musica, parametro a do PATTERN, font.tga/event.tga.


