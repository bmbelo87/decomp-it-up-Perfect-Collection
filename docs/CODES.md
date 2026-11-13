**Idiomas:** **Português (BR)** · [English](CODES.en.md) · [Español](CODES.es.md)

# Códigos secretos (setas do Arcade)

Sprites extraídos de `BGA/00.DAT` (`ARROW01.PNG` / `ARROW03.PNG`, quadro 0, 54×54) por `tools/make_arrow_sprites.py`, salvos em `docs/img/arrows/`. Pad: **UL** = cima-esquerda, **UR** = cima-direita, **C** = centro, **DL** = baixo-esquerda, **DR** = baixo-direita (teclas de teste: `Q E S Z C` no P1, `7 9 5 1 3` no P2).

<img src="img/arrows/UL.png" width="28" alt="UL"> UL · <img src="img/arrows/UR.png" width="28" alt="UR"> UR · <img src="img/arrows/C.png" width="28" alt="C"> C · <img src="img/arrows/DL.png" width="28" alt="DL"> DL · <img src="img/arrows/DR.png" width="28" alt="DR"> DR

## Arcade (`PIU.EXE`)

Executar a sequência na tela de **modo** ou de **músicas**, com as últimas 8 teclas (tabela `0x81219AA` do ELF, `func_2a72b`). Repetir o código desliga o efeito (alterna). Cada efeito de velocidade substitui o outro.

| Efeito | Sequência (8 toques) |
|---|---|
| **CANCEL** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **VANISH** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **MIRROR** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> |
| **8X** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **4X** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **2X** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **RANDOM VELOCITY** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> |
| **NONSTEP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **SYNCHRO** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **COUPLE** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **UNION** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **RANDOM VANISH** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **RANDOM STEP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> |
| **REMIXDB** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **REMIXCZ** | <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> |

REMIXDB só com o modo REMIX e 1 jogador; REMIXCZ só com o modo REMIX na roda de músicas. A linha 16 da tabela (`10 4 1 10 8 4 2 8`) não é usada.

## PC (`pumpy.exe`)

Executar na tela de **modo** ou de **músicas** (histórico de 5 toques em `0x42DFF0` e de 3 toques em `0x42E020`; cada tela tem o seu buffer). As setas abaixo são as do Arcade; a PC usa arte própria.

| Efeito | Sequência |
|---|---|
| **CANCEL** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/DL.png" width="28" alt="DL"> |
| **VANISH** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **MIRROR** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **RANDOM VELOCITY** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **NONSTOP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> |
| **SYNCHRO** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **COUPLE** | <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **UNION** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> |
| **RANDOM VANISH** | <img src="img/arrows/DL.png" width="28" alt="DL"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DR.png" width="28" alt="DR"> <img src="img/arrows/C.png" width="28" alt="C"> <img src="img/arrows/DL.png" width="28" alt="DL"> |
| **SPEED UP** | <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UL.png" width="28" alt="UL"> <img src="img/arrows/UL.png" width="28" alt="UL"> |
| **SPEED DOWN** | <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UR.png" width="28" alt="UR"> <img src="img/arrows/UR.png" width="28" alt="UR"> |

SPEED UP: cada repetição sobe 2X → 4X → 8X → RANDOM STEP → nada; SPEED DOWN faz o caminho inverso.

## Observações

Confirmado no código: as sequências e o bit de cada efeito. Os **nomes** vêm da lista do usuário (inferido). Aplicados no jogo hoje: 2X/4X/8X, MIRROR, RANDOM STEP, VANISH (inferido) e RANDOM VANISH (inferido); os demais só ligam a flag.
