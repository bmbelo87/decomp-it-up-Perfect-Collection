# PiuReconstructed — Pump It Up (PIU.EXE, DOS, "Dec 18 2000") recompilado em C

Como jogar: dê dois cliques em `jogar.bat` (usa a pasta `..\game`). Ou: `PiuReconstructed.exe -game <pasta do jogo>`.

## Controles
| | |
|---|---|
| Seleção | Cima/Baixo música (PgUp/PgDn ±10), Esquerda/Direita nível, **Enter** jogar, F1–F4 velocidade (x1/x2/x4/x8), **Esc** sair, F11 tela cheia |
| P1 (5 pads) | `Z` ↙, `Q` ↖, `S` centro, `E` ↗, `C` ↘ |
| P2 / Double | Numpad `1` ↙, `7` ↖, `5` centro, `9` ↗, `3` ↘ (em fases single o numpad também joga as 5 colunas) |
| Em jogo | Esc volta à seleção |

## Estado (Fase 1: fidelidade)
- Fiel (confirmado no assembly): tempo de 240 Hz, scroll, troca de BPM, janelas Perfect/Great/Good/Bad por BPM, regras de julgamento, combo/pontuação/vida, formatos `.NOT`, `.AUD`(MP3), `.PNZ`(PNG), `.DAT`(RES/BGA/SPR/SP2).
- Reconstrução funcional (o original não foi totalmente decompilado): menu de seleção, tela de resultado/nota, sprites exatos das setas, efeitos de acerto, notas longas (se existirem), modos/opções de jogo, transições e telas de attract/título.
- Renderização: OpenGL 1.x (SDL2). Ver docs/ para o mapa de endereços do PIU.EXE original.

## Build
`cmake -S . -B build_msvc -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static`  
`cmake --build build_msvc --config Release` (gera `piu.exe`; `piu_test.exe` valida todos os arquivos).  
Auto-teste: `piu.exe -game ../game -song 101 -diff 1 -selftest 25 -shot shot.ppm` (autoplay; `-noauto` desliga).
