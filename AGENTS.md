# AGENTS.md - PiuReconstructed

Recompilacao ("recomp") em C do **Pump It Up (build DOS de 18/12/2000)**: `game/PIU.EXE` (Watcom LE/DOS4GW, Glide2x). O objetivo e reproduzir as telas e o comportamento **exatamente como o original** (fidelidade primeiro). Responda em portugues (BR).

## Onde procurar a verdade (ordem)
1. **`game/PIU.EXE`** - fonte primaria. Abra com o REA/Ghidra pelo PE sintetico `piu_le.exe` (gerado por `tools/le_to_pe.py`; enderecos = VAs do DOS, ex. `FUN_0002C840`). Convencao de chamada Watcom: argumentos em EAX/EDX/EBX nao aparecem no pseudocodigo, leia o **assembly**.
2. **Se nao achar no PIU.EXE, procure no `game/piu` (ELF Linux, R5/v3.52)**. Ele nao e stripped: os simbolos `func_XXXXX` / `jump_XXXXX` / `vtable_XXXXX` usam **o mesmo endereco hexadecimal da funcao do DOS** (ex. `func_2c840` = `FUN_0002C840`), entao serve de tradutor de enderecos e de segunda opiniao. Ele contem tabelas e codigos que o decompilado do DOS esconde.
   - O Ghidra do REA no Windows **nao abre ELF**. Use `nm`, `readelf` e `objdump -d -M intel --no-show-raw-insn piu` (MinGW) e Python para ler tabelas: converta endereco virtual em offset pelos program headers.
   - Os dados do ELF ficam em outros enderecos (ex. flags de modo = `0x81206d2`; no DOS `DAT_0012B028`); descubra pelo codigo equivalente.
   - Ressalva: tabelas de **teclas** do ELF (`player1keys`...) sao o mapeamento do porte, nao os bits do pad do DOS.
3. `game/STAGE.CFG`, `game/*.DAT` (`tools/piu_res.py`, `tools/piu_bga.py`) e `game/EFFECTS/*.wav` para dados.
4. Observacao do usuario (jogando) vale como prova do comportamento; registre como "empirico".

Exemplo de achado so no ELF: codigos secretos REMIXDB / REMIXCZ (`func_2a72b`, tabela `0x81219AA`) - ver `docs/FUNCTION_MAP.md` (Atualizacao 27).

## Regras de trabalho
- Distinga **confirmado** (lido no codigo/dado) de **inferido**; marque inferencias nos comentarios e no doc.
- "Desativar" codigo = envolver em `/* */`. Nao refatore sem pedido.
- **Cada alteracao = 1 commit**; cena nova = +0.1.0 de versao (tag `v0.N.0`; atual v0.9.0). Mensagem termina com a linha `Co-Authored-By` definida pelo ambiente.
- Documente cada descoberta em `docs/FUNCTION_MAP.md` (secao "Atualizacao N").
- Teste: `build_msvc` (CMake/MSVC Release) gera `piu.exe`; copie para `game/PIUTESTE.EXE` (se "em uso", `PIUTESTE_NOVO.EXE`). Capturas: `-shot f.ppm -selftest SEG`, atalhos `-mode N`, `-conf N`, `-wheel N -wdir D`, `-service N`.
- Entradas: P1 `Z Q S E C` (bits 1/2 azuis Z/C = rolar, 8/0x10 Q/E = apontar, S = centro), P2 numpad `1 7 5 9 3`; F1 service, F2 test, F3 coin clear, F4 moeda; 1-8 velocidade.
- Sessoes do REA: `open_binary` em um unico alvo por vez; se der `cleanup_incomplete`, rode `close_binary` e abra de novo.
