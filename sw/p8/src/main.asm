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

#SECTION "Entry", ROM0 {
entry:
    call lcd_off
    ld   hl, $8000
    ld   bc, $2000
    call clearMem

    ld   hl, fontData
    ld   de, $9000
    ld   bc, fontData.end - fontData
    call copy1BPP

    ld   a, $0A
    ld   [$0000], a
    ld   a, $00
    ld   [$4000], a
    ld   hl, $A000
    ld   de, $8000
    ld   bc, 16 * 16 * 16
    call copyMem

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

    ; Setup the tile indexes for the render area
    xor  a
    ldh  [rVBK], a
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

    ld   a, -16
    ldh  [rSCX], a
    ld   a, -8
    ldh  [rSCY], a

    ld   a, LCDC_ON | LCDC_BG_ON | LCDC_BLOCK01
    ldh  [rLCDC], a
    call waitVBlank

    ld   hl, defaultPalette
    call setBGPalette

.loop:
    call updateJoypadState
    ld   a, [wJoypadState]
    ld   [$A000 + $1FF0], a
    
    call waitVBlank
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

    jr .loop

defaultPalette:
    dw $7FFF, $5294, $35ad, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $0000, $34C6, $34B1, $3260
    dw $25D4, $31AE, $5ED6, $6B9F
    dw $301F, $033F, $23FF, $27E0
    dw $7E48, $4DF1, $51FF, $533F
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
