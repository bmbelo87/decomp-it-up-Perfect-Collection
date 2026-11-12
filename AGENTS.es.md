**Idiomas:** [Português (BR)](AGENTS.md) · [English](AGENTS.en.md) · **Español**

# AGENTS.md - PiuReconstructed

Recompilación ("recomp") en C de **Pump It Up (build DOS del 18/12/2000)**: `game/PIU.EXE` (Watcom LE/DOS4GW, Glide2x). El objetivo es reproducir las pantallas y el comportamiento **exactamente como el original** (fidelidad primero). Responde en portugués (BR).

## Dónde buscar la verdad (en orden)
1. **`game/PIU.EXE`** - fuente primaria. Ábrelo con REA/Ghidra mediante el PE sintético `piu_le.exe` (generado por `tools/le_to_pe.py`; direcciones = VAs del DOS, ej. `FUN_0002C840`). Convención de llamada Watcom: los argumentos en EAX/EDX/EBX no aparecen en el pseudocódigo, lee el **ensamblador**.
2. **Si no está en PIU.EXE, busca en `game/piu` (ELF Linux, R5/v3.52)**. No está stripped: los símbolos `func_XXXXX` / `jump_XXXXX` / `vtable_XXXXX` usan **la misma dirección hexadecimal de la función del DOS** (ej. `func_2c840` = `FUN_0002C840`), así que sirve de traductor de direcciones y de segunda opinión. Contiene tablas y códigos que el decompilado del DOS esconde.
   - El Ghidra de REA en Windows **no abre ELF**. Usa `nm`, `readelf` y `objdump -d -M intel --no-show-raw-insn piu` (MinGW) y Python para leer tablas: convierte dirección virtual en offset con los program headers.
   - Los datos del ELF están en otras direcciones (ej. flags de modo = `0x81206d2`; en DOS `DAT_0012B028`); descúbrelas por el código equivalente.
   - Salvedad: las tablas de **teclas** del ELF (`player1keys`...) son el mapeo del port, no los bits del pad del DOS.
3. `game/STAGE.CFG`, `game/*.DAT` (`tools/piu_res.py`, `tools/piu_bga.py`) y `game/EFFECTS/*.wav` para datos.
4. La observación del usuario (jugando) vale como prueba del comportamiento; regístrala como "empírico".

Ejemplo de hallazgo solo en el ELF: códigos secretos REMIXDB / REMIXCZ (`func_2a72b`, tabla `0x81219AA`) - ver `docs/FUNCTION_MAP.md` (Actualización 27).

## Reglas de trabajo
- Distingue **confirmado** (leído en código/dato) de **inferido**; marca las inferencias en los comentarios y en el doc.
- "Desactivar" código = envolverlo en `/* */`. No refactorices sin que lo pidan.
- **Cada cambio = 1 commit**; escena nueva = +0.1.0 de versión (tag `v0.N.0`; actual v0.9.0). El mensaje termina con la línea `Co-Authored-By` definida por el entorno.
- Documenta cada descubrimiento en `docs/FUNCTION_MAP.md` (sección "Actualización N").
- Prueba: `build_msvc` (CMake/MSVC Release) genera `piu.exe`; cópialo a `game/PIUTESTE.EXE` (si está "en uso", `PIUTESTE_NOVO.EXE`). Capturas: `-shot f.ppm -selftest SEG`, atajos `-mode N`, `-conf N`, `-wheel N -wdir D`, `-service N`.
- Entradas: P1 `Z Q S E C` (bits 1/2 azules Z/C = desplazar, 8/0x10 Q/E = apuntar, S = centro), P2 teclado numérico `1 7 5 9 3`; F1 service, F2 test, F3 coin clear, F4 moneda; 1-8 velocidad.
- **Documentación en 3 idiomas**: todo `.md` tiene versiones pt-BR (`X.md`, original), English (`X.en.md`) y Español (`X.es.md`), con la línea de idiomas arriba. Al editar uno, actualiza los tres en el mismo commit.
- Sesiones de REA: `open_binary` en un único objetivo a la vez; si da `cleanup_incomplete`, ejecuta `close_binary` y abre de nuevo.
