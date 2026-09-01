.setcpu "6502"
.include "atari.inc"


.export _init_dlist, _screen_memory



; ------------------------------------------------------------
; SCREEN: we keep screen RAM in a loaded rw segment
; ------------------------------------------------------------
        .segment "SCREEN"
.align 1024

screen_memory:
    .res( 380 ) ;    20 * (1 + 1 + 2 * 8 + 1 )
_screen_memory = screen_memory

        .segment "DLIST"
.align 1024
;  just a single ANTIC 5 screen
main_dlist:
        .byte $70, $70, $40          ; blank lines for overscan

        .byte $46                    ; LMS + sets ANTIC 6 (20x8x2)
        .word screen_memory          ; gives address of start of screen memory. ( DL and DL+1)
        .byte $20                ; 3 blank

        .byte $06                    ; ANTIC 6 (20x8x2)
        .byte $10                ; 2 blank

        .repeat 7 
            .byte $06                ; Antic 6 (20x8x2)
            .byte $06                ; 
            .byte $58                ; 6 blank
        .endrepeat
        .byte $06                ; Antic 
        .byte $06                ; Antic 
        .byte $10                ; 2 blank

        .byte $06                    ; ANTIC 6 (20x8x2)

        .word main_dlist             ; JVB ( vertical blank jump to start of display list


; ------------------------------------------------------------
; CODE: init routine called from C
; ------------------------------------------------------------
        .segment "CODE"

.proc _init_dlist

    lda #0                                       ; stop the dma
    sta DMACTL
    lda #<main_dlist                             ; install the new display list
    sta SDLSTL
    lda #>main_dlist
    sta SDLSTH
    lda #$22                                     ; resume the DMA
    sta DMACTL

    rts


.endproc
