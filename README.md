# PiuReconstructed — Pump It Up (PIU.EXE, DOS, "Dec 18 2000") recompilado em C

Como jogar: dê dois cliques em `jogar.bat` (usa a pasta `..\game`). Ou: `PiuReconstructed.exe -game <pasta do jogo>`.

## Controles
| | |
|---|---|
| Seleção | Cima/Baixo música (PgUp/PgDn ±10), Esquerda/Direita nível, **Enter** jogar, **1–8** velocidade, **Esc** sair, F11 tela cheia |
| P1 (5 pads) | `Z` ↙, `Q` ↖, `S` centro, `E` ↗, `C` ↘ |
| P2 / Double | Numpad `1` ↙, `7` ↖, `5` centro, `9` ↗, `3` ↘ (em fases single o numpad também joga as 5 colunas) |
| Em jogo | Esc volta à seleção |
| Arcade | **F1** SERVICE (abre o menu de serviço e seleciona), **F2** TEST (move), **F3** Coin Clear, **F4** moeda (5/F5 também), Alt+X sai |

## Estado (Fase 1: fidelidade)
- Fiel (confirmado no assembly): tempo de 240 Hz, scroll, troca de BPM, janelas Perfect/Great/Good/Bad por BPM, regras de julgamento, combo/pontuação/vida, formatos `.NOT`, `.AUD`(MP3), `.PNZ`(PNG), `.DAT`(RES/BGA/SPR/SP2).
- Reconstrução funcional (o original não foi totalmente decompilado): menu de seleção, tela de resultado/nota, sprites exatos das setas, efeitos de acerto, notas longas (se existirem), modos/opções de jogo, transições e telas de attract/título.
- Renderização: OpenGL 1.x (SDL2). Ver docs/ para o mapa de endereços do PIU.EXE original.

## Build
`cmake -S . -B build_msvc -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static`  
`cmake --build build_msvc --config Release` (gera `piu.exe`; `piu_test.exe` valida todos os arquivos).  
Auto-teste: `piu.exe -game ../game -song 101 -diff 1 -selftest 25 -shot shot.ppm` (autoplay; `-noauto` desliga).


## Fluxo de telas (Atualizacao 13) - igual ao PIU.EXE
`idle -> logo (BGA/81.DAT, 500 quadros) -> idle -> intro (BGA/01.DAT, ate 945 quadros) -> titulo (BGA/82.DAT, loop de 180 quadros) -> selecao`.
Origem: laco principal `0x20F37` (estado em `DAT_00153710`). Sem credito o jogo fica em attract; **5 (ou F5) = moeda**,
**Enter/Espaco = START P1**, **Num Enter/Num + = START P2**; ~1 s depois da partida vai para a selecao. ESC volta ao idle (como o original), Alt+X sai.
Opcoes: `-freeplay` (credito infinito), `-coins N`, `-skip` (direto na selecao).
