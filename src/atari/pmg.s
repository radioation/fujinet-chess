.setcpu "6502"
.include "atari.inc"

.export _pmg_memory


; ------------------------------------------------------------
; PMG: Player Missile Graphics.
; ------------------------------------------------------------
    .segment "PMG"
_pmg_memory: .res 1024


