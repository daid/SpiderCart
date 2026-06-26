#INCLUDE "gbz80/all.asm"
#INCLUDE "gbz80/extra/if.asm"
#INCLUDE "gbz80/extra/loop.asm"
#INCLUDE "gbz80/extra/ld16.asm"
#INCLUDE "gbz80/extra/pushpop.asm"

#INCLUDE "video.asm"
#INCLUDE "input.asm"
#INCLUDE "copy.asm"

GBC_HEADER "SpiderCart", GB_MBC5_RAM, entry

#SECTION "SpiderCartIndicator", ROM0[$150] {
    ; Most ROMs have the entry directly after the header. For a SpiderCart compatible ROM
    ; We put a special indicator here to indicate to the cart should run with special features enabled
    db $DD ; Indicator 1, this is an invalid instruction.
    db "SPIDER"
}

#SECTION "HRAM", HRAM {
hGBC: ds 1
}

#MACRO POPSLIDE {
    ld  [hl+],a
    ld  a, d
    ld  [hl+], a
    pop de
    ld  a, e
}

#MACRO HALT_FOR _FLAG {
    ld   a, _FLAG
    ldh  [rIE], a
    xor  a
    ldh  [rIF], a
    ld   a, e ; prepare for popslide
    halt
    nop
}

#MACRO WAIT_HBLANK {
:   ldh a, [rSTAT]
    and a, STAT_MODE_MASK
    jr  nz, :- ; != STAT_HBLANK
}

#MACRO WAIT_VBLANK {
:   ldh a, [rSTAT]
    and a, STAT_MODE_MASK
    cp  STAT_VBLANK
    jr  nz, :-
}

#SECTION "Entry", ROM0 {
entry:
    cp   a, $11
    jr   z, .gbc
    xor  a
.gbc:
    ldh  [hGBC], a
    ld   sp, $e000

    call lcd_off
    ld   hl, $8000
    ld   bc, $2000
    call clearMem

    ld   hl, fontData
    ld   de, $9000
    ld   bc, fontData.end - fontData
    call copy1BPP

    ; Enable SRAM and switch to bank 15
    ld   a, $0A
    ld   [$0000], a
    ld   a, $0F
    ld   [$4000], a

    ld   hl, $A000
    ld   de, $8000
    ld   bc, 16 * 16 * 16
    call copyMem ; Copy initial screen

    ldh  a, [hGBC]
    and  a, a
    if   nz {
        ld   a, 1
        ldh  [rVBK], a
        ; Set all tiles to 2nd VRAM bank except for the 128x128 render area
        ld   hl, $9800
        ld   bc, $0400
        ld   a, $08
        call setMem

        ld   a, 00
        ld   hl, $9800
        loop b, 16 {
            loop c, 16 {
                ld   [hl+], a
            }
            ld  de, 16
            add hl, de
        }
        xor  a
        ldh  [rVBK], a

    } else {
        ld   a, %00_00_00_00
        ldh  [rBGP], a
        ldh  [rOBP0], a
        ldh  [rOBP1], a
    }

    ; Center the view screen
    ld   a, -16
    ldh  [rSCX], a
    ld   a, -8
    ldh  [rSCY], a

    ; Setup the tile indexes for the render area
    ld   a, 00
    ld   hl, $9800
    loop b, 16 {
        loop c, 16 {
            ld   [hl+], a
            inc  a
        }
        ld  de, 16
        add hl, de
    }
    ; Clear the 2nd tilemap
    ld   hl, $9C00
    ld   bc, $0400
    ld   a, $00
    call setMem

    ldh  a, [hGBC]
    and  a, a
    jp   z, .startDMG

    ld   a, LCDC_ON | LCDC_WIN_9C00 | LCDC_BG_ON | LCDC_BLOCK01
    ldh  [rLCDC], a
    call waitVBlank

    ld   hl, defaultPalette
    call setBGPalette

.loopGBC:
    call updateJoypadState
    ld   a, [wJoypadState]
    ld   [$A000 + $1FF0], a
    ld   a, $80 ; COMMAND_P8_CYCLE_60
    ld   [$6000], a
    
    ; Wait for VBLank
    ld   a, IE_VBLANK
    ldh  [rIE], a
    xor  a
    ldh  [rIF], a
    halt
    
    ld   a, $01
    ld   [$A000 + $1FF1], a
    ; Copy 128 tiles with HDMA in direct copy
    ld   a, $A0
    ldh  [rHDMA1], a
    ld   a, $00
    ldh  [rHDMA2], a
    ld   a, $80
    ldh  [rHDMA3], a
    ld   a, $00
    ldh  [rHDMA4], a
    ld   a, 127
    ldh  [rHDMA5], a
    ; Now we are nearing end of VBlank, so copy the rest in HBlank
    ld   a, 127 | $80
    ldh  [rHDMA5], a

    ; Check for errors
    ld   a, [$BFFF]
    and  a, a
    jp   nz, displayError

    jr .loopGBC

.startDMG:
    ld   a, STAT_MODE_0
    ldh  [rSTAT], a

    ld   a, $01
    ld   hl, $9830
    loop c, 16 {
        loop b, 16 {
            ld   [hl+], a
        }
        ld  de, 48
        add hl, de
    }

    ld   hl, dmgOAM
    ld   de, $FE00
    ld   bc, 40 * 4
    call copyMem

    ld   a, LCDC_ON | LCDC_WIN_9C00 | LCDC_BG_ON | LCDC_BLOCK01 | LCDC_OBJ_ON | LCDC_OBJ_16
    ldh  [rLCDC], a

    call waitVBlank
.loopDMG:
    call updateJoypadState
    ld   a, [wJoypadState]
    ld   [$A000 + $1FF0], a
    ld   a, $81 ; COMMAND_P8_CYCLE_30
    ld   [$6000], a

    ld   sp, $A000
    ld   hl, $8000
    pop  de

    ; This code is tuned to copy exactly 128 tiles per frame in VBlank and HBlank time.
    #FOR FRAME_LOOP, 0, 2 {    
        HALT_FOR IE_VBLANK
        ld   a, e
        #FOR n, 0, 93 {
            POPSLIDE
        }
        ; There is a bit of spare time here.
        loop c, 7 {
            HALT_FOR IE_STAT
            #FOR n, 0, 6 {
                POPSLIDE
            }
        }
        HALT_FOR IE_STAT
        ld   a, %11_10_01_00
        ldh  [rBGP], a
        loop c, 127 {
            HALT_FOR IE_STAT
            #FOR n, 0, 7 {
                POPSLIDE
            }
        }
        HALT_FOR IE_STAT
        ld   a, %00_00_00_00
        ldh  [rBGP], a
        ; We are only at line 136, so we have some spare time till VBlank here.
    }
    ld   sp, $E000

    ; Check for errors
    ld   a, [$BFFF]
    and  a, a
    jp   nz, displayError

    jp .loopDMG

defaultPalette:
    dw $7FFF, $5294, $35ad, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $0000, $34C6, $34B1, $3260
    dw $25D4, $31AE, $5ED6, $6B9F
    dw $301F, $033F, $23FF, $27E0
    dw $7E48, $4DF1, $51FF, $533F

dmgOAM:
    #FOR Y, 0, 8 {
        db Y * 16 + 24, 8, 0, 0
        db Y * 16 + 24, 16, 0, 0
        db Y * 16 + 24, 128 + 24, 0, 0
        db Y * 16 + 24, 128 + 32, 0, 0
    }
    #FOR N, 0, 8 {
        db 0, 0, 0, 0
    }
}

#SECTION "ErrorDisplay", ROM0 {
displayError:
    ld   sp, $E000
    call lcd_off

    xor  a
    ldh  [rSCX], a
    ldh  [rSCY], a
    ld   hl, $9C00
    ld   bc, $0400
    call clearMem

    ; Copy error message
    ld   de, $BF00
    ld   hl, $9C00
    loop {
        ld   a, [de]
        sub  a, $20
        jr   c, .errorDone
        ld   [hl+], a
        inc  de
        ld   a, l
        and  a, $1F
        cp   20
        if   nc {
            ld  bc, 32 - 20
            add hl, bc
        }
    }

.errorDone:
    ld   a, LCDC_ON | LCDC_BG_9C00 | LCDC_BG_ON | LCDC_BLOCK21
    ldh  [rLCDC], a

.loop:
    xor  a
    ldh  [rIF], a
    ldh  [rIE], a
    halt
    nop
    jr .loop
}

#SECTION "PrintStr", ROM0 {
_printStr: ; print the string at BC on the background at DE=YX
    ld  h, d
    xor a ; clear carry
    rr  h
    rra
    rr  h
    rra
    rr  h
    rra
    or  e
    ld  l, a
    ld  a, $98
    add a, h
    ld  h, a

.loop:
    ld  a, [bc]
    sub a, $20
    ret c
    inc bc

    ld  e, a
    STAT_WAIT
    ld  a, e
    ld  [hl+], a
    jr  .loop
}

#SECTION "Font", ROMX, BANK[1] {
fontData:
    #INCGFX "font.1bpp.png", BPP[1], COLORMAP[$000000, $FFFFFF]
.end:
}

#SECTION "SRAM", SRAM {
    ; For now ensure we have at least one SRAM section so the header has SRAM marked and BGB won't complain.
}
