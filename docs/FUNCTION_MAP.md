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


## Atualização 11 — Esqueleto C (src/)
- `src/piu.h` API; `piu_data.c` (Adler-32, cifras, RES, .NOT, .BGA, interpolação 0x25FA5); `piu_play.c` (relógio 240 Hz, linha atual, janelas por BPM, classify, apply_judgment); `piu_key.h` (KEY 1024 B extraída do exe); `main_test.c` (`build/piu_test.exe ../game`).
- Validação: AUD 213/213, PNZ 109/109, DAT 117/117 (todas as entradas e .BGA), NOT 472/472. Autoplay 101_DB (bpm 146.1, N=2): janelas P/G/Go/B = 7/12/18/23 px; 92 notas; erro 0 e 6 => 92 Perfect; erro 20 => mistura (P37 G18 Go29 B8).
- Pendente p/ fidelidade: vida inicial/limites (0x23A4B), `[0x3D0BE0]` (modifier de life_rate; -700 setado em 0x24B2D..), calculo de pontuacao final, hold/long notes (mask colunas 10-12?), tela de resultado, Glide->OpenGL, SDL2 + janela, audio MP3, input.


## Atualização 12 — Jogo jogável (src/game.c)
SDL2+OpenGL 1.x. `PiuReconstructed.exe`/`jogar.bat`. Seleção (STAGE.CFG + TITLE/Txxx.PNZ), jogo (BGA do .DAT com keyframes/SPR, setas de ARROW01/03.PNG, julgamento 0x23229/janelas 0x24BA6, perda de linha=Miss, vida 500 inicial, fail em 0), resultado. Frame do BGA = pos/4 (60 fps; 0x250C5). Selftest com autoplay: 172/172 Perfect em 101_H1; sem entrada: Misses e fail. Pendente: gamepad, preview de música, sprites/efeitos originais (SP2 ARROW541-545), hold/long notes, telas originais.


## Atualizacao 13 - fluxo attract/boot (confirmado no PIU.EXE)
- Laco principal FUN_00020F37; estado em DAT_00153710: 0 idle, 0x0A/0x0B selecao (0x2B763/0x2D0BF), 0x0D/0x0E logo, 0x10/0x11 intro, 0x13/0x14 titulo, 0x16/0x17 tela 03.DAT (0x28843/0x288A4, 1920 quadros), 0x1C setup (0x2E46A), 0x1D (0x2E4D4), 0x01 jogo (0x247E2).
- Tabela de nomes em 0x12B0C9 {ptr nome, fn}: exit, idle, run, setup, sel, intro, logo, title.
- idle 0x20E7A: credito (FUN_204BA; 99 se free play) -> estado 0x13; senao n=DAT_001537D8++%4: 0 logo(81.DAT), 1 intro(01.DAT), 3 FUN_2D14D (nao reconstruido).
- Contador de quadros DAT_003D651C; FUN_2643C(EAX=bga, EDX=quadro) desenha as 50 camadas. Logo/intro: quadro = contador ANTES do ++; titulo: quadro = contador % 180 (IDIV 0xB4 em 0x2874D).
- logo termina em 500 (0x1F4); intro em 0x3B1; titulo volta ao idle com contador >= 0x708 sem credito/start. START exige credito (FUN_2047F = DAT_00153730 / DAT_006E191A); 60 quadros (0x3C) depois do START (ou P1+P2) -> 0x2B763 (selecao).
- DAT_006E1919 != 0 e DAT_006E1914 == 0 -> passa pela tela 03.DAT (0x28843) antes da selecao (valores vem de config em runtime; assumido 0).
- Texto do titulo: printf("%d%02d",3,0x34) em (601,458) preto e (600,459) branco = "352" (versao). Overlay FUN_20134: CREDIT(S)/INSERT COIN(S)/PRESS CENTER STEP da FONT.PNG (00.DAT), alpha pisca |(t%200-100)*0.01|+0.2 (floats em 0x12000C/0x120010). Posicoes na tela: INFERIDAS.
- Moeda: FUN_20424 (bit 0x20000 do input, estado != 0x1D): DAT_00153730++. ESC (0x100) -> estado 0. Alt+X sai.
- Pendente: FUN_2D14D (demo/ranking), selecao real 0x2D0BF, tela 03.DAT, 83.DAT/84.DAT, setup 0x2E46A, resultado real.

## Atualizacao 14 - selecao real (estado 0xB, FUN_0002D0BF) - ANALISE PARCIAL
- Sub-maquina DAT_006E1798 (0..7): 0 init FUN_2BB5E (carrega BGA\099.DAT, toca AUDIO\%03d.AUD via FUN_29263), 1 intro FUN_2BC14, 2 selecao FUN_2BD93, 3 FUN_2C4AB, 4 FUN_2C63F (BGA\098.DAT), 5 FUN_2C81C, 6 FUN_2C840 (grande), 7 FUN_2CDFA. Fases 2 e 6 chamam FUN_2CFC2/FUN_2ABDA.
- BGA/099.DAT = tela "Music Select" (titulo, TIME, trilhos, botoes SHIFT, "1P/2P PLAYER PRESS CENTER STEP TO JOIN"); 99.DAT/98.DAT = menu_bg + botoes de modo (czp, btlp, dbp, ezp, hdp...); 091..096.DAT = botoes por nivel (NB/HB/...); 090/091(2 digitos).DAT = barras de titulo (BARnn.PNG = folhas 256x256 com 8 barras, CDnn.PNG = capas redondas).
- Ordem das barras: por id de musica, so para musicas que tem barra (BAR01 = 109..115,201; BAR02 = 202..210...). Mapeamento exato id->(folha,linha) NAO confirmado.
- STAGE.CFG tem secoes "MODE EASY ..." com linhas "S nivel id" separadas por linha em branco (paginas) - o original usa isso para as listas.
- Lista horizontal com passo 0x42 (66 px) em FUN_2BD93; timer de selecao 0x1E - (FUN_2057F/0xF0) (30 s); teclas por jogador: bits 8/0x10/1/2 = esq/dir/..., modo em DAT_006E1778 (1..6 -> flags 0x10,0x20,0x100,0x80,0x40,0x400 em DAT_0012B028).
- 83.DAT = BGA da tela de jogo (ctx2, bar, t_perf/t_great/t_good/t_bad/t_miss/t_combo, ngb, lin; 0..2790 quadros); 84.DAT = ending; 03.DAT = tela de ajuda (1935 quadros).
- Ferramenta: piu.exe -view <nome> -frame N -selftest 1 -shot x.ppm renderiza qualquer BGA.

## Atualizacao 15 - selecao real: fase 2 = escolha de MODO (FUN_0002bd93) [observado]
- FUN_0002bc14 (fase 1->2): desenha 099 (FUN_0002643c), zera DAT_006e17a4 (contador de animacao), DAT_006e1798=2, DAT_006e1784=7.
- FUN_0002bd93 (fase 2): lista vertical de modos, item = 0x42 px, scroll anima 0x16 frames (3 px/frame).
  - DAT_006e1788 = direcao (0/1 parado, 2 sobe, 3 desce); DAT_006e179c = item atual; DAT_006e178c = total de itens.
  - Entrada: DAT_00153718 (P1: bits 1,2,8,0x10) e byte alto (P2); DAT_00153720/21 bit1/bit2 = repeticao.
  - Item (DAT_006e1788+DAT_006e179c) -> DAT_006e1778: 0->4 (ou 5 se 2 jogadores), 1->1, 2->2, 3->3, 4->6.
  - Confirma: bits em DAT_0012b028 |= 0x10(modo1) 0x20(2) 0x100(3) 0x80(4) 0x40(5) 0x400(6); DAT_006e1798=3 (proxima fase).
  - Timer: 30 s (0x1e - tempo/0xF0) -> auto-confirma o item atual.
- Conclusao: a ordem real e 099 -> escolha de MODO -> (fase 3) musica. A lista de musicas so vem na fase 3.

## Atualizacao 16 - busca de SFX no PIU.EXE (resultado negativo parcial)
- PIU.EXE: 0 ocorrencias de RIFF/WAVE/fmt/.WAV/VOC/ID3/OggS; sem bloco PCM (entropia de codigo ~6 ate 0xF0000, depois dados baixos).
- Unicos formatos de arquivo carregados: AUDIO\%03d.AUD (0x120336) e AUDIO\D%03d.AUD (0x120345) via FUN_00029263 (so monta o nome; id em EAX), BGA\*.DAT (SPR/PNG/BGA/SP2).
- BGA\*.DAT (234 DATs/entradas): so .SPR .PNG .BGA .SP2 -> nenhum audio dentro.
- AUDIO\001/002/003.AUD = MP3 128 kbps de 18,2 s / 29,2 s / 36,3 s (musicas de tela, nao SFX). FUN_28843 (ajuda, estado 0x17) toca 003 (MOV EAX,3 antes de 0x29263) -> 003 = musica da ajuda. 001 provavelmente intro (FUN_28442 passa o id em EAX; nao confirmado).
- Menu de service do EXE (strings @0x1056065): "SOUND TEST", "EFFECT SOUND", "DEMO SOUND", "AUDIO" -> existe opcao de efeito sonoro, mas a fonte do som nao esta em arquivo nem embutida de forma detectavel.
- Conclusao: nenhum SFX em arquivo ou embutido foi encontrado. Hipotese (nao confirmada): efeitos vinham de hardware de som do gabinete arcade.

## Atualizacao 17 - SFX: sao de HARDWARE (placa de som), nao de arquivo [observado + inferido]
- FUN_00030fac(EAX=on/off, EDX=idx, EBX=voz) = "toca efeito": grava um descritor de 16 bytes (tabela @0x7238A0 + idx*16) em registradores de um chip de som via porta [0x724AB8] (FUN_00030085 = escrita em porta; pares endereco/dado em base+0..3, +0x20..23, +0x40..43, +0x60..63) e termina com key-on (reg1 | 0x80).
- Chamadores confirmados: moeda (FUN_00020424: FUN_30fac), selecao fase 2 (FUN_2bd93: mover item e confirmar), entre outros.
- Init: FUN_00028297 = init do chip (27cd4/27b2a, espera 500 ms, detecta com FUN_3037c) e entao FUN_3155e le PIU.BIN em registros de 16 bytes (tamanho>>4 = 35) para a tabela de vozes, depois FUN_30fa5.
- PIU.BIN (560 bytes) = 35 descritores de voz (enderecos encadeados: o fim de um = inicio do proximo) -> o PCM em si NAO esta em arquivo nem no EXE: fica na ROM da placa de som arcade.
- Conclusao: nao existem WAVs desta versao para extrair; os 35 efeitos so podem ser substituidos por amostras externas.

## Atualizacao 18 - SFX: pasta EFFECTS + ELF Linux (piu, R5/v3.52, nao stripado) [observado]
- game/EFFECTS tem 35 WAV = as 35 vozes de PIU.BIN. Ordem/indice = tabela `effects_file_names` (@0x08113500 do ELF): 0 01-1,1 01-11,2 2-1,3 3-2,4 4-2,5 5-1,6 6-2,7 7-1,8 8-1,9 9-5,10 9-A,11 9-B,12 9-C,13 9-D,14 9-F,15 TIME_LIMIT,16 10-2,17 13-1,18 12-1,19 14,20 START1,21 START2,22 START3,23..27 RANK_A,B,C,D,F,28..31 NOTGOOD1-4,32..34 GOODJOB1-3.
- O ELF e uma traducao do codigo DOS: simbolos jump_XXXXX / func_XXXXX = enderecos VA do PIU.EXE (ex.: func_294b9, jump_2c14d). ac_sound_play_effect(EAX=0, EDX=indice, EBX=canal) = FUN_00030fac.
- Chamadas (VA DOS: indice/canal): 0x20470 moeda 21/-; 0x2c14d,2c181,2c249,2c27a (mover L/R no modo) e 0x2cb76,2cbaa,2cc72,2cca3 (mover L/R na musica) 16/4 (10-2); 0x2c486 e 0x2cdc3 confirmar 4/3 (4-2); 0x2882e,288f2,2acda,2acf9,2d048,2d0xx apertos de pad 3/2 (3-2); 0x2a91b 2/1 (2-1, 5 s); 0x25e44 rank 27/6 (RANK_F, demais ranks entram pelo mesmo ponto); 0x24f50,24f9a 7/7 (7-1); 0x234b5 9/2 (9-5); 0x2e151 parar efeito (eax=1).
- game.c: sfx_load/sfx_play (8 canais, novo no mesmo canal corta o anterior). Ligados: moeda, mover, entrar na musica, join no titulo, rank do resultado. NAO ligados (ponto de chamada ainda nao rastreado): "1 credito completo", NOTGOOD/GOODJOB, TIME_LIMIT, 2-1, 7-1, 9-x.

## Atualizacao 19 - menu de SERVICE reconstruido [observado, exceto onde marcado]
- Despachante FUN_0002E4D4: DAT_0012CB80 = tela (0 menu FUN_2D847, 1 I/O FUN_2D954, 2 EEPROM FUN_2DB18, 3 SCREEN FUN_2DC59, 4 GAME OPTION FUN_2DCDC, 5 COIN OPTION FUN_2DFC0, 6 SOUND TEST FUN_2E167, 7 DATE FUN_2E2BC, 8 EXIT FUN_2E450). Cabecalho "PUMP IT UP (R%d/v%d.%02d)" (4,4) com 5,3,0x34 (= R5/v3.52, igual ao arquivo `version` do ELF) e "Dec 18 2000" (0x224,4); rodape (0xEC,0x24) "MOVE - TEST BUTTON" + (0xEC,0x10) "SELECT - SERVICE BUTTON", ou so "EXIT - SERVICE BUTTON" nas telas 1-3. Coordenadas y-para-cima, fonte 8 px de avanco (FUN_00026658 = texto).
- Botoes: DAT_00153718 == 0x10000 TEST (mover, cursor DAT_0012CB84 no menu / DAT_0012CB88 nas linhas), == 0x20000 SERVICE (selecionar).
- Menu: titulo "GAME SETTING" (0x114,0x1C2); itens x=0x114, y=0x160-0x14*i; "Lock OK:%d Err:%d" (0x114,0x98) (contadores de FUN_29818).
- Cores (FUN_000C7E84): linha selecionada vermelho (0x12CB98), normal branco (0x12CBE0), valor padrao verde (0x12CBB0).
- GAME OPTION (valores x=0x194): GAME MODE 6e1914 NORMAL/EVENT; LEVEL 6e1915 0..2 (1.EASY 2.NORMAL 3.HARD, padrao 1); STAGE BREAK 6e1916 0..4 (OFF / "%d STAGE", padrao 2); LANGUAGE 6e1917 0..2 (ENGLISH, ENGLISH, CHINESE); DEMO SOUND 6e1918 OFF/ON; SHOW HELP 6e1919 OFF/ON; DEFAULT SETTING FUN_2D6B5 (14=0,15=1,16=2,17=0,18=0,19=0,1b=1); SAVE AND EXIT FUN_2D750; EXIT.
- COIN OPTION: COIN SETTING 6e191a 0..10 (0 = "FREE", senao "1 CREDIT / %2d COINS" em x=0x174), DEFAULT SETTING FUN_2D70F, SAVE AND EXIT, EXIT.
- SOUND TEST: AUDIO (#%02d, 6e1920) e EFFECT SOUND (#%02d, 6e191c), EXIT; TEST para o som (FUN_294B9) e move; SERVICE toca (FUN_2E0ED audio / FUN_2E12A efeito).
- DATE: YEAR(%4d, 6e192c=2000), MONTH(%2d, 6e1928 so 11/12), DAY(%2d, 6e1924; 1..30 se mes 11, 1..31 se 12), EXIT salva (FUN_2D3AB).
- I/O TEST: "1.TEST  BUTTON" (0x104,400) "2.SERVICE BUTTON" (0x17C) "3.CLEAR BUTTON" (0x168) "4.COIN" (0x154), estado por FUN_2D640 (nao lido; aqui: botao apertado fica vermelho - inferido). EEPROM TEST: WRITE/READ/RESULT com "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345" e SUCCESS/FAIL.
- INFERIDO: tela SCREEN TEST (FUN_2D194/2D259 nao lidas: barras de cor), valor de AUDIO #id, arquivo de persistencia PIU_EEPROM.BIN (o original grava em EEPROM), coin padrao 1.
- game.c: svc_open/svc_press/svc_draw; F1 abre/SERVICE, F2 TEST, F3 coin clear, F4 moeda; `-service N` abre direto a tela N. COIN SETTING agora controla g_cpc/g_freeplay.

## Atualizacao 20 - musicas de tela [observado]
- FUN_00028442 (inicio da intro 01.DAT): FUN_294B9 (para o audio); se ha credito -> titulo; senao, se DAT_006e1918 (DEMO SOUND) != 0: FUN_00029263(EAX=1) + FUN_0002949F = AUDIO\001.AUD.
- FUN_0002BB5E (entrada da selecao / escolha de modo, apos PRESS CENTER STEP): FUN_294B9 e AUDIO\002.AUD incondicional (EAX=2), carrega BGA 099 (strings 0x1205F9..), zera DAT_006E17A4/1778, 1788=-1, 1798=179C=1.
- FUN_00028843 (ajuda 03.DAT, estado 0x17): AUDIO\003.AUD (EAX=3).
- game.c: intro toca 001 se DEMO SOUND (padrao inicial ligado: inferido, o DEFAULT SETTING literal do codigo poe 0); selecao toca 002; os demais estados chamam music_stop.

## Atualizacao 21 - tela de escolha de MODO (estado 0xB fase 2) reconstruida [observado]
- Init FUN_0002BB5E: FUN_294B9, BGA 099, FUN_2B609(1) carrega as 14 folhas BAR01..14 (90.DAT) na tabela 0x12C3A1 (14 linhas x 40 bytes: [0]=textura, [1..8]=ids das 8 faixas, [9]=indice), AUDIO\002.AUD. Lista = STAGE.CFG "MODE MENU": ids 2,3,4,5,6 (DOUBLE, NORMAL, HARD, NONSTOP REMIX, CRAZY); com 2 jogadores o item 0 vira id 1 (BATTLE) (FUN_2BC14: DAT_006E0EE8 hi16 = 1). Faixas de BAR14: 516,517,1..6 = Slam, Space Fantasy, BATTLE, DOUBLE, NORMAL, HARD, NONSTOP REMIX, CRAZY.
- Laco FUN_2BD93 por quadro (60 Hz): tempo = 30 - FUN_2057F/0xF0; (1) camada de destaque do modo (DAT_6E1778 0..6 -> camadas 0,3,1,2,4,6,5) no quadro 600+f%90 (FUN_2647A = 1 camada; FUN_2643C = 50); (2) BGA inteiro no quadro 30; (3) numero do tempo em (584,429) amarelo (FUN_2A672); (4) setas: camadas 0x1B/0x1C e 0x1E/0x1F no quadro 90+anim; (5) translate(local) com local = anim*3-66 (dir 2) ou -anim*3+66 (dir 3) por 22 quadros; FUN_2B192 desenha as barras em (213,218)+66*i; FUN_2B459 pulsa o destaque em (292|358,319); BGA no quadro 150 deslocado em -(cur-1)*66; (6) camada 0x2E/0x2F no quadro f%30.
- Barras (FUN_2AE66/2AF37): glRotatef(45), quad 194x32 (x 30..224) com u 0.113..0.875, v = faixa*0.125; destaque centrado (-97,-16). Estas rotinas usam y PARA CIMA (480-y) - o BGA nao.
- Pulso: p=(anim/2)%50; copias escala p/150+1 alfa 1-p/30 (p<60), p/100+1 e 1-p/20 (p<40), p/50+1 e 1-p/10 (p<20).
- Entrada (DAT_00153718): bits 1/2 = rola esquerda/direita (dir 2/3; cur-- / cur++, cur 0..3, so com anim>3; limite -> anim=22), bits 8/0x10 = escolhe item cur+0 / cur+1 (dir 0/1). Confirma quando apertar o MESMO botao 2 vezes (prev==dir) ou o tempo < 1 s; item (dir+cur) -> modo 4|5 (Double/Battle), 1, 2, 3, 6 -> DAT_0012B028 |= 0x80|0x40, 0x10, 0x20, 0x100, 0x400; proxima fase 3 (lista de musicas).
- Nao lidos: FUN_2A672 (numero do tempo), FUN_2B958, FUN_21915, FUN_20134 (HUD de jogadores/creditos). Mapeamento teclas->bits e inferido (Q/Left, E/Right, Z/Up, C/Down, Enter = confirma).
- game.c: S_MODE (mode_enter/mode_step/draw_mode, bga_draw_n); `-mode 1|2` abre a tela com 1 ou 2 jogadores.

## Atualizacao 22 - teclas e bits do pad (ELF Linux `piu`) [observado]
- ELF: player1keys @0x081134E0 = Q,E,S,Z,C (GLFW 0x51,0x45,0x53,0x5A,0x43) -> bits 0..4 do pad (io_input_kbd_p1: `esi ^= 1<<i`); player2keys @0x081134CC = KP7,KP9,KP5,KP1,KP3 (0x147,0x149,0x145,0x141,0x143). serviceKeys G,H,J (+K, L no P2) ligam bits de service/coin (0x200, 0x4000, 0x8000, 0x400).
- CORRECAO: a tabela de teclas do ELF e do PORTE, nao dos bits do DOS (nao serve para inferir a ordem dos bits). Pela observacao do usuario na tela de modo: bits 1/2 = baixo-esq/baixo-dir (Z/C) rolam a lista (dir 2/3); bits 8/0x10 = cima-esq/cima-dir (Q/E) selecionam cur+0/cur+1 (dir 0/1) e repetir o mesmo confirma. Ordem dos bits do pad no DOS: 1=DL 2=DR 4=centro 8=UL 0x10=UR (inferido). O centro (S) nao e testado nessa tela.

## Atualizacao 23 - interpolacao de camadas BGA: o pivo NAO e interpolado [observado]
- FUN_00025FA5 (0x260D9..0x2624F): entre as chaves i-1 e i interpola so x, y, escala, rotacao e cor r,g,b,a (FUN_25F88 = lerp); o pivo (px,py, +0xC/+0x10) vem da chave anterior, sem interpolar. Matriz: translate(0,480), translate(x,-y), translate(px,-py), rotate, scale, translate(-px,py), translate(0,-480) = y para cima, equivalente ao y para baixo usado em game.c.
- Efeito: o anel de zoom/fade das bolinhas SHIFT (099.BGA camadas 0x1C e 0x1F, m09_1/m09_2.SPR, 1,0 -> 1,2 com alfa 1 -> 0, pivo (135,392)) saia do lugar porque o pivo da ultima chave (0,0, inativa) era interpolado.
- piu_data.c: piu_bga_eval passa a usar o pivo da chave anterior.

## Atualizacao 24 - fase 3: animacao do modo escolhido (097.DAT) e fase 4/5 [observado]
- FUN_0002BB5E carrega BGA 097 no objeto 0x2FF58C. FUN_0002C4AB (fase 3, 30 quadros): FUN_2643C(0x2FF58C, quadro = DAT_006E17A4 + tabela[DAT_006E1778]) com tabela @0x12CB25 = {0,0,120,360,240,480,600} (modo 1 NORMAL 0, 2 HARD 120, 4 DOUBLE 240, 3 REMIX 360, 5 BATTLE 480, 6 CRAZY 600; a 097.BGA tem 6 trechos de 120 quadros: nzb/hdb/dbb/nrb/btb/czb). Ao chegar a 30: FUN_294B9 (para a musica 002) e DAT_006E1798 = 4.
- FUN_0002C63F (fase 4): carrega BGA 098 (objeto 0x15C95C) e o fundo colorido do modo no objeto 0x2FF58C: DAT_006E1778 -> 0:091, 1:091 (NORMAL rosa), 2:092 (HARD laranja), 3:094 (REMIX azul), 4:093 (DOUBLE verde), 5:095 (BATTLE vermelho), 6:096 (CRAZY roxo); FUN_2B609(0), FUN_2B70B (lista pelo grupo DAT_006E1784 conforme o modo), DAT_006E1798 = 5.
- game.c: S_MCONF (mconf_step/draw_mconf) e, depois, S_SELECT com 098 + fundo 09x sob a lista antiga; `-conf N` abre a animacao do modo N.

## Atualizacao 25 - fase 5: roda de musicas, FUN_0002C840 [observado; trechos marcados como inferidos]
- Mesma maquina da escolha de modo (DAT_006E1788 = dir: 0/1 apontam cartaz esquerdo/direito, 2/3 rolam; DAT_006E17A4 = anim; DAT_006E1794 = dir anterior; DAT_006E179C = posicao; DAT_006E178C = tamanho da lista). Entrada aceita com anim > 3 (bits 8/0x10 -> dir 0/1, 1/2 -> dir 2/3). Rolar: anim==0 -> pos -= 2 / += 2 (circular, FUN_2B09D). Confirma: mesmo ponteiro duas vezes (prev == dir < 2) ou 30 s (FUN_2057F/0xF0): DAT_006E17A0 = lado, FUN_30FAC(0, 4, 3), DAT_006E1798 = 7.
- Desenho: fundo 09x (objeto 0x2FF58C) girando cnt*0.06 graus em torno de (320,240), quadro (cnt*2)%90; 098 (0x15C95C) no quadro 0; setas SHIFT camadas 0x1B/0x1C (dir 2) e 0x1E/0x1F (dir 3) no quadro 90 + anim (anim < 90); roda FUN_2B0DD: translate (213,218), 8 itens +66 e 8 itens -66, deslocamento local anim*6-0x84 (dir 2) / -anim*6+0x84 (dir 3) por 22 quadros; cada item e uma faixa BARxx da 90.DAT (FUN_2AFFF, mesma tabela 0x12C3A1 da escolha de modo, ids de musica 101..517).
- Com dir 0/1: camadas 0x23/0x2B/0x2C da 098 no quadro 0x96, dois cartazes (FUN_2B266(0,0), (1,0), (dir,anim)) e BPM "%5.1f" em (0x128,0x63) (FUN_2C5CA, tabela 0x6DFC28: id -> float).
- Cartaz (FUN_2AE17/FUN_2AD1B): tabela 0x12C5D1, 30 linhas de 40 bytes (handle + 4 ids): linha r = folha CD(r+1).PNG da 90.DAT (CD01..CD26; 256x256, 2x2 recortes de 128x96: u = (j%2)*0.5, v = (j/2)*0.375). Desenhado 200x142 centrado em (161,320) / (481,320) com y para cima; pulso como nas bolinhas: p = (anim/2)%50, escala p/S0+1, alfa 1-p/S1 enquanto p < S2, S = {150,30,60},{100,20,40},{50,10,20} (0x1205B3..).
- Lista (FUN_2B70B): entradas de 4 bytes em 0x6DF6B0 (byte0 = valido, byte1 = nivel, hi16 = id), preenchidas por FUN_2A039 a partir de um arquivo de texto (nao localizado) e, sem limitacao (DAT_0012B024==0), FUN_2B695 marca todas como validas. Por modo (DAT_006E1784) so entram as faixas [DAT_006E1636, +DAT_006E1638) da tabela.
- Nao reconstruido: o valor de LEVEL (byte1 de cada entrada, "%d" em (0x130,0x46) via FUN_26658), a lista por grupo/modo (aqui entram todas as musicas do STAGE.CFG), pre-escuta da musica (FUN_29295/FUN_2949F ao fim de 30 quadros apontando), FUN_2A672 (numero do tempo), FUN_2B958, FUN_21915, FUN_20134 e a fase 7 (escolha de dificuldade).
- game.c: S_WHEEL (wheel_enter/wheel_step/draw_wheel, CD_TBL[26][4]); `-wheel MODO [-wdir 0|1|2|3]` abre direto; depois de confirmar cai na lista antiga (dificuldade) com a musica escolhida.

## Atualizacao 26 - listas por modo (STAGE.CFG) e posicao do PRESS CENTER STEP [observado / inferido onde indicado]
- FUN_0002C63F: DAT_006E1784 (secao do STAGE.CFG, posicional) = 0 EASY se flag 0x10 (NORMAL), 1 HARD se 0x20, 2 DOUBLE se 0x80, 3 REMIX se 0x100, 4 REMIXDB se 0x200, 6 REMIXCZ se 0x800, 5 CRAZY se 0x400; BATTLE (0x40) depende de DAT_0012B024 (0 EASY, 1 HARD, 2 REMIX). FUN_2B70B copia as entradas da secao (faixa [DAT_006E1636 + g*4, contagem DAT_006E1638 + g*4]) para a lista da roda.
- Linha "S nivel id" = entrada de 4 bytes: byte1 = nivel (impresso "%d" em (0x130,0x46) na roda), hi16 = id. Linhas em branco separam paginas por serie (so agrupamento).
- Inferido: nao achei quem liga as flags 0x200 (REMIXDB) e 0x800 (REMIXCZ). Pelos dados REMIXDB = REMIX jogado em double (so 119 e as demais ate 415) e REMIXCZ = REMIX jogado em crazy (225, 327, 330, 415). Na reconstrucao, REMIX usa sempre a secao REMIX; DB/CZ ainda nao sao selecionados.
- FUN_00020134 (estado 0x14): FUN_21885(x, y=0x16 para cima, sprite): PRESS CENTER STEP = sprite 0x25 em x=0xE (P1) / 0x198 (P2); INSERT COIN = sprite 0x24 em x=0x30 / 0x1A8. game.c ajustado.

## Atualizacao 27 - REMIXDB / REMIXCZ: codigos secretos (achado no ELF Linux) e fases reais [observado]
- Numeracao real das fases do estado 0xB (despachante `jump_2d0dd` do ELF, tabela 0x8059B3B; DAT_006E1798): 0 FUN_2BB5E, 1 FUN_2BC14, 2 FUN_2BD93 (modo), 3 FUN_2C4AB (097), 4 FUN_2C63F (carrega 098/fundo/lista), 5 FUN_2C81C (so poe fase=6 e zera o tempo), 6 FUN_2C840 (**roda de musicas**), 7 seguinte. (A Atualizacao 25 chamava a roda de "fase 5".)
- FUN_2B958 (trecho 0x2ACDA no ELF) a cada toque do pad chama FUN_2A72B (`func_2a72b`): memcmp das 8 ultimas teclas (historico DAT em 0x86D6CC2) com 15 sequencias de 8 bytes (tabela 0x81219AA; bits 1 DL, 2 DR, 4 centro, 8 UL, 0x10 UR), despacho por tabela 0x8057331. Ignorado se a flag BATTLE (0x40) estiver ligada.
- Com a flag REMIX (0x100): `DL UL DL C UR DR UR C` (func_2a88a) e (flag&3)!=3 (so 1 jogador): flag &= 0xF00F, |= 0x200 (REMIXDB), grupo (DAT_006E1784) = 4, refaz a lista (FUN_2B70B). `C DL C DR C DL C DR` (func_2a8c4) e fase == 6: flag |= 0x800 (REMIXCZ), grupo = 6, refaz a lista. Em teclas (Z=DL, C=DR, Q=UL, E=UR, S=centro): REMIXDB = Z Q Z S E C E S; REMIXCZ = S Z S C S Z S C.
- Outros codigos da mesma tabela (nao implementados): `C x8` limpa flags (&0xFFF); varios alternam bits de opcao de jogo (DAT 0x83CAEAA: 0x10, 0x20, 0x8, 0x4, 0x2, 0x40, 0x80, 0x100/0x200) e bits 0x1000/0x2000/0x4000 da flag de modo.
- game.c: `wheel_code` (historico de 8 toques, so com modo REMIX) troca a secao e refaz a lista.

## Atualizacao 28 - preview da musica na roda (empirico)
- Ao apontar um cartaz na roda de musicas (UL/UR = Q/E, numpad 7/9) toca o preview `AUDIO\Dxxx.AUD` da musica apontada (xxx = id do STAGE.CFG); repetir o mesmo lado (confirma) nao reinicia; sair da roda para a selecao para o preview. **Inferido/empirico**: a rotina original que dispara o preview ainda nao foi localizada no PIU.EXE (nao confirmado se rolar tambem muda o preview).
- game.c: `music_load_pfx("D", id)` em `S_WHEEL`.

## Atualizacao 29 - fase 7: da roda direto para a gameplay
- Confirmado (ELF func_2cdfa = FUN_2CDFA, fase 7 de 0xB): pega a musica (lista DAT_006E1782 + indice, FUN_2B09D), para a musica (FUN_294B9), monta o nome do passo por flags de modo (0x81206d2 / DOS DAT_0012B028) com `%d_E1/H1/H3/E2/H2/H4/DB.STF` e chama FUN_2058F ("run %s %s %d"). Nao ha tela de selecao de dificuldade: o **nivel** e o modo escolhido antes (EASY/HARD/REMIX/DOUBLE/CRAZY), a roda lista so as musicas da secao, e confirmar entra na gameplay.
- Regra: DOUBLE/REMIXDB -> DB; 1P: HARD/REMIX -> H1, CRAZY/REMIXCZ -> H3, senao E1; 2P: HARD/REMIX -> H2, CRAZY/REMIXCZ -> H4, senao E2. Ramo flag 0x4000 (batalha, %d_H1/E1 + "-S") nao tratado.
- game.c: `wheel_step` chama `start_song(di)`; a tela antiga S_SELECT so serve de fallback se faltar o arquivo.

- Tela de loading (FUN_2A533, ELF func_2a533): antes de iniciar, sprintf "T%03d" da musica, carrega TITLE\T%03d.PNZ e desenha em tela cheia (ac_game_draw_title); depois FUN_2058F inicia o jogo. game.c: estado S_LOAD (`load_step`/`draw_load`); duracao ~1,5 s e **empirica** (no original e o tempo de carga).

## Atualizacao 30 - quadro de gameplay (ELF func_24de6 = FUN_24DE6): mapa de desenho (parcial)
Confirmado no ELF (ainda nao conferido no PIU.EXE; enderecos `func_XXXXX` = VA do DOS). Ordem de desenho de um quadro:
1. Fundo: `func_2643c(0x8152006, frame)` (frame = tempo>>2; 50 chamadas de `func_25fa5`, camadas do BGA) - a menos que a flag 0x40 (batalha) esteja no ramo de "piscar" (fundo cheio vermelho/azul alternando a cada 3 quadros).
2. Campo(s) por jogador - `func_22674` (P1) / `func_226ad` (P2), ou `func_226f9` (double, duas metades): `glTranslatef(x, 378, 0)` com **x = 28 (P1), 348 (P2); double: 66 e 311** (y de baixo para cima: linha dos receptores a 378 = 102 do topo). Cada campo chama `func_221a4` (5 receptores nos x = 35,83,131,179,227 e y = 35 relativos, 48 px entre colunas, escala de pulso da tabela 0x81206EA indexada pelo estado 0x814AFCE.., sprites em 0x83C7226/0x83C907E/0x83C62FA/0x83C8152/0x83C9FAA com passo 60 bytes = objetos de sprite desenhados por `func_2EE67`), `func_22386` e `func_22502` (outras camadas do campo). `func_22dcd/22df1/22e20` -> `func_22840` (setas/notas, 498 linhas) desenham as notas quando o jogador nao esta em modo "apagado" (bit 0x80).
3. Julgamento: `func_22fd8(jogador)` - contador por jogador (0x83CAED8 + 0x29*p) decrementa, `glTranslatef(0, 240, ...)`, escala de 'aparecer' e sprite do julgamento (0x37 '7' e acima).
4. Barras de vida/pontuacao: `func_21e47(vida, 7)` em (176|573, 16) e `func_21885` em x=14 (P1) / x=560 (P2), y=52, tam 40 (so se nao for batalha e sem flag 0x1000).
5. `func_23a4b(jogador)` (combo?), `func_21915`, `func_20134` (HUD/contador), e, se DAT 0x8148DD6, linha de debug de tempo (`func_26658`, "%d:%02d:%d").
Estado do prototipo (draw_play): usa **xs=200** para 1 jogador (errado: P1 fica em x=28, P2 em 348), receptores/setas em coordenadas proprias e HUD de texto; falta ler `func_2EE67` (formato do sprite), as folhas de sprites (00.DAT/01.DAT/03.DAT?) e os desenhos de `func_22386/22502/22840/21885/21e47/23a4b/21915/20134`. **Proximo passo**: ler o formato de sprite e o carregador das folhas de gameplay.

## Atualizacao 31 - campo de gameplay com sprites SP2 e fim da tela antiga de selecao
- Confirmado (ELF func_21fe1/func_221a4/func_22386/func_22502/func_22840): os sprites do campo sao `BGA/00.DAT` ARROW541..545.SP2 (tiles b1/b2 receptor, n1..n6 nota, f1..f8 acerto, p1..p8 toque), um por direcao: DL=542, UL=541, centro=545, UR=543, DR=544. Receptor b1 por 3/4 do tempo e b2 no ultimo quarto de cada tempo; escala de pulso ao tocar (tabela 0x81206EA: 1,.97,.94,.91,.88,.85,.72,.69); p1..p8 em 16 quadros, f1..f8 em 24; nota n1..n6 = 6 quadros por tempo. Campo em x0=28 (P1) / 348 (P2), double 66 e 311; receptor a (x0+35+48k, 67 do topo); notas sobem de baixo.
- Inferido: quais julgamentos disparam os tiles f (aqui PERFECT/GREAT/GOOD); y da linha dos receptores (67) deduzido de translate 378+35 em y-para-cima.
- A tela antiga S_SELECT (lista + nivel) foi desativada: nao ha mais fallback nem retorno do resultado para ela (volta ao attract); so resta acessivel por `-skip` de depuracao.

## Atualizacao 32 - HUD de gameplay: vida, score, combo, julgamento (ELF func_23a4b, func_21e47/21885, func_22fd8, func_21d11)
Confirmado no ELF e igual no DOS (FUN_22FD8 tem os mesmos desvios). Coordenadas em y-para-cima; jogador P1 `0x83CAEB2`, P2 +0x29 por jogador (struct: +4 vida float, +8 vida inteira 0..1000, +0x14 combo, +0x18 serie de MISS, +0x26 ultimo julgamento, +0x24 temporizador).
- **Vida** (`func_23a4b`): moldura `gauge_b` (ARROW542, dec00.tga) em (32|352, 445); v=vida/1000, n=(int)(v*256), q=n/6; o preenchimento `gauge__` (mesma textura, v 32..64) e o retangulo x in [253-6q, 256] (P1) / [0, 6q+2] (P2) da moldura. Perigo: vida<180 acende `gauge_D` (event.tga) amarelo a cada 2 quadros (no original depende da opcao 0x86D6FC0; aqui sempre). O original pisca o preenchimento em quadros alternados (nao reproduzido: **inferido** que nao importa).
- **Score** (`func_21e47(valor,7)` em (176|573, 16)): 7 digitos de font.tga (celulas 32x39, v0=0.53125+(d/8)*0.15234375, u=(d%8)*0.125), de tras para a frente, passo -24; rotulo `score_` (ARROW541) em (14|560, 52).
- **Julgamento** (`func_22fd8`): 25 quadros; centro (160|480|320 em double, 240); escala = tabela 0x812071E[25-t] para t>10 (1.52..1.0), senao 1; tile = perfec/great_/good__/bad___/miss__ (ARROW542) pelo ultimo julgamento '5'..'9' ('A' = attack_, batalha).
- **Combo** (`func_21d11`): dentro da mesma escala, 70 px abaixo; 3 digitos de event.tga (32x48, v0=.375+(d/8)*.1875) com zeros a esquerda, passo -26 a partir de +10; so com combo>=4 (branco). Em seguida a serie de MISS (>=4) em azul (0.2,0.2,1), continuando a esquerda (a translacao nao e desfeita). O tile `combo_` existe no SP2 mas **nao e usado** no ELF nem no DOS durante o jogo (so os digitos).
- NUM do ARROW542.SP2 diz 43 mas ha 45 tiles; `spr_parse` agora conta as linhas T. `sp2_find` compara so 6 letras (gauge_b/__/D colidem): usar `sp2_find_x`.
- game.c: bloco HUD de `draw_play` (desenho em y-para-cima com `tile_up`). Nao tratado: icones de opcao (func_21915), tempo/credito (func_20134), modo batalha, 2 jogadores simultaneos.

## Atualizacao 33 - sprites PATTERN do BGA (ELF func_2efad tipo 3, func_2f4ef)
- Confirmado: `TYPE pattern a b c` (FUN_2F4EF): a -> [obj+0x24], b -> hi16 de [obj+0x28], c -> byte baixo. No desenho (tipo 3) a grade tem **c colunas x b linhas**, passo = (x+w, y+h) do tile 0 (translate x*coluna, linha*altura para baixo); indice do tile = `(a != 0 ? linha*c + coluna : b*coluna + linha) + (int)(n*(1-frac))` modulo n. Antes so se desenhava um tile (a Mobius Strip, 213, mostra 2x2 tiles 320x240 identicos).
- Tipo 1 (TILE): os tiles sao desenhados do ultimo ao primeiro (ordem invertida, confirmado no laco que decrementa).
- game.c: `Spr.pb/pc/a`, `bga_draw_n`.
