# PC_DIFF.md - Build PC (`pumpy.exe`, 16/04/2001) x Build Arcade (`PIU.EXE`, 18/12/2000)

Objetivo: dois executaveis no projeto - `piu` (dados Arcade) e `piu_pc` (dados PC), mesma base de codigo com `#ifdef PIU_BUILD_PC`.
Fontes: `E:\06_pc\pumpy_unpacked.exe` (dump desempacotado, ver Atualizacao 48), `game/PIU.EXE` + ELF `game/piu` (enderecos DOS), `tools/cmp_bga.py`, `tools/map_pc_dos.py`.
Legenda: **[C]** confirmado (lido no binario/dado) - **[I]** inferido - **[E]** empirico.

## 1. Conclusao principal [C]
O `pumpy.exe` e **o mesmo codigo-fonte do arcade portado para Win32** (MSVC 6, OpenGL, DirectInput/DirectSound, MP3 por DirectShow). A ordem das funcoes e as strings batem quase 1:1 com o DOS
(ex.: PC `0x403bef/0x403cb0/0x403da0` = DOS `0x28469/0x285d4/0x284e0`, intro/81/82). O menu de servico, o EEPROM (agora `c:\pump.ini`), `stage.cfg`, "run step_name", o loader de sprites (`.SPR/.SP2`, `TYPE/NUM/TILE/ANI/PATTERN`) e o carregador RES existem iguais. Logo o `game.c` atual serve de base; o trabalho e: (a) dados diferentes, (b) cenas com arte nova, (c) camada de plataforma.

## 2. Correspondencia de funcoes PC -> DOS (-> `game.c`)
| PC (pumpy) | DOS (PIU.EXE) | Papel | Status |
|---|---|---|---|
| 0x401310 | 0x2628F | loader `.BGA` (`fail - %s %s`) | igual (formato BGA identico: 108/108 parseiam com `tools/piu_bga.py`) |
| 0x401ad0 | 0x21FE1 / 0x264B1 | carrega `font.tga` | igual |
| 0x402340 / 0x40238F | 0x289B0 / 0x28A3C | `[%s] file not found`, decode | igual |
| 0x403BEF | 0x28469 | intro `bga\01.dat` | **alterado** (01.dat: 57 itens mudaram, +11/-4) |
| 0x403CB0 | 0x285D4 | logo `bga\81.dat` | igual (81.dat identico) |
| 0x403DA0 | 0x284E0 | titulo `bga\82.dat` | **alterado** (arte nova: PC 49 itens, 47 so PC; ver 3) |
| 0x4044A9 | 0x28843 | `bga\03.dat` | quase igual (so `03.BGA` e `HELP.PNG`) |
| 0x4046B0 / 0x40B880 | 0x2A533 | `TITLE\%s.PNZ` | igual |
| 0x404710 | - | `AUDIO\84.MP3` + `credits` | **novo** (creditos em MP3) |
| 0x405090 | 0x2087F | captura `.\cap\p5_%03d.TGA` | igual (debug, dead [I]) |
| 0x405220 | 0x20AB2 | HUD debug `%02d%c%02d,%5.1f %d/%d` | igual |
| 0x405400 | 0x20D38 | le `stage.cfg` | igual (Stage.cfg com 3 linhas de ordem trocada: 112/226/328) |
| 0x405720 / 0x405740 | 0x20F0F / 0x21170 | `idle %d` / `idle...` | igual |
| 0x405AA0..0x405F00 | - | MP3 via DirectShow (`RenderFile`, `AUDIO\%03d.MP3`, `AUDIO\D%03d.MP3`) | **novo** (arcade: `.AUD`/PIU.BIN) |
| 0x406040 | 0x21361 | carrega `STEP\%s.NOT` | igual |
| 0x4069E0.. | (gameplay) | gameplay/julgamento | **a verificar** (usuario: muito parecido; ver 5) |
| 0x409170..0x409259 | 0x24879..0x24CD2 | `run step_name -d`, `-demo/-demo2/-s1/-s2`, `BGA\%03d.DAT` | igual (argumentos de linha de comando do original; dead no jogo normal [I]) |
| 0x40AD00 / 0x40AD48 | 0x29A45 / 0x29A8C | `res_load`, `Memory insufficient` | igual (RES v3: `count`/`datasize` trocados) |
| 0x40C240 | 0x2BB5E / 0x2C6AC | MODE SELECT (`02.TGA`; PC lista `MM_BTL/CZ/DB/EZ/HD/NR.TGA`) | **alterado** (99.dat: PC 13 itens x Arcade 87; ver 3) |
| 0x40D680 | (2B6BC..) | `sel_music_init() - ErrMode`, `bga\099.DAT`, `bga\90.DAT` | igual [I] (nao achada por string no ELF) |
| 0x40DCE0 | 0x2CF60 / 0x2CE7D | roda -> `run %s %s %d`, nomes `%d_DB/E1/E2/H1..H4.STF` | igual (mas `Step/` so tem `.NOT`; `.STF` = dead [I]) |
| 0x40E970 | 0x2B633 | barras `BGA\90.DAT` | igual (90.dat identico) |
| 0x40EA30 | 0x2C689 / 0x2C663 | `LOAD%d`, `LOAD4`, `02.TGA`, `07.TGA` | **alterado** (099.dat: +43/-41 itens) |
| 0x40F5B0 | 0x2D14D | demo `run %d_e2.stf -demo %d` | igual |
| 0x40F7E0 .. 0x410280 | 0x2E4FF, 0x2D68B, 0x2D7CA, 0x2D847, 0x2D98D.., 0x2DB18, 0x2DC59, 0x2DCDC, 0x2DFC0, 0x2E167, 0x2E2BC | SERVICE: versao `PUMP IT UP (R%d/v%d.%02d)` (data **Apr 14 2001**), EEPROM, GAME SETTING, I/O TEST, EEPROM TEST, SCREEN TEST, GAME OPTION, COIN OPTION, SOUND TEST, DATE SETTING | igual (EEPROM em `c:\pump.ini`; `Lock OK:%d Err:%d`) |
| 0x411080 .. 0x4115CF | 0x2F54B.. | parser de sprites `sp2/spr/tga`, `NUM/TYPE/TILE/ANI/PATTERN` | igual |
| 0x4118D0 | - | `piukey.cfg` (teclas) | **novo** |
| 0x411A30 | - | DirectSound + `eff\*.wav` (16 arquivos) | **novo** (arcade: voz do chip, 35 WAV) |
| 0x411DD0 | - | relatorio `%s\%d%d%d.txt` (`PUMP IT UP Fusion v%s ... Windows %d.%d.%d`, build `Apr 15 2001`) | **novo** |
| 0x412080 | - | `AUDIO\083.MP3` + `bga\083.dat` (GO_*.SPR) | **novo** (Game Over proprio) [I] |
| 0x4015D0 .. 0x4016F0 | - | checagem do tapete (Atualizacao 48) | **novo**; nao reproduzir |
| 0x4027C0 .. 0x402970 | - | driver `gwiopm.sys` | **novo**; nao reproduzir |
| 0x402E10 .. 0x4035BD | - | janela/contexto OpenGL | **novo** (equivale ao SDL do `piu_pc`) |

## 3. Diferencas de dados (ver `E:\06_pc\comparacao_BGA_arcade_vs_pc.txt`)
- Container RES v3 = v2 com `count`/`datasize` trocados; mesma cifra [C]. Imagens PC tem quantizacao leve (dif. max 4) [I: reducao de cor].
- 98 de 118 `.dat` com conteudo identico (musicas 101..517 quase todas); `83.dat`, `90.dat`, `97.dat`, `81.dat` identicos.
- Ausentes no PC: `091`-`096`, `098`, `91`, `98` (so Arcade). Presente so no PC: `083.dat` (+ `AUDIO\083.MP3`).
- Telas com arte/estrutura diferente: **`00.dat`** (+`00.BGA`, `MAH01..06`, setas redesenhadas), **`01.dat`** (intro), **`82.dat`** (titulo: `00..20.PNG`, `FORPC.SPR`, `LOGO_01/02`, `PCDF`), **`99.dat`** (menu: 13 itens, sem `MENU_BG*`/`MMB*`), **`099.dat`** (modo/roda: `A01..F01.SPR`, `BB/CB/DB/HB/NB` redesenhados), `03.dat` (`HELP`), `413.dat` (sem `HFONT1/2`), `109/506/511` (so o `.BGA`).
- `Stage.cfg`: posicoes de 112/226/328 diferentes; `STEP/` 472 `.NOT`, `AUDIO/` 215 `.mp3` (+ `D4xx.MP3`).

## 4. Plataforma [C]
OpenGL (opengl32) em vez de Glide; DirectInput (`piukey.cfg`) em vez de leitura de pad; DirectSound `eff\*.wav` + DirectShow MP3; EEPROM em `c:\pump.ini`; checagem de tapete + driver `gwiopm`. O `piu_pc` usa SDL2 (ja equivalente); teclado = `piukey.cfg` (formato a decifrar).

## 5. Gameplay e Dance Grade (hipotese do usuario: muito parecidos) - verificar
Nada contradiz ate agora [I]: mesmos `.NOT`, mesmo `83.dat`, mesmos nomes de rank (`eff\A..F.wav` = RANK_A..F), `piu_play.c` ja usa o formato `.NOT`. Falta: comparar numeros concretos (janelas de julgamento, pontuacao/combo, barra de vida) na vizinhanca de `0x4069E0` com `piu_play.c` e marcar igual/diferente.

## 6. Plano proposto (um commit por item)
1. [feito] Dois alvos (`piu`, `piu_pc`) + CI + leitura RES v3 + pasta `Eff` + caminhos sem caixa.
2. Nomear a IAT no Ghidra e confirmar a tabela da secao 2 (anotar enderecos PC em `docs/FUNCTION_MAP.md`).
3. Titulo/attract PC (`82.dat`, `01.dat`, `00.dat`).
4. MODE SELECT PC (`99.dat`, `MM_*`) e roda (`099.dat`, `A01..F01.SPR`).
5. Audio: MP3 por id (`Audio/%03d.mp3`, `D%03d`), efeitos `Eff/*`.
6. Gameplay/Grade: validar igualdade; ajustar o que divergir.
7. Entrada: decifrar `piukey.cfg`.
8. Game Over PC (`083.dat`/`083.mp3`).
