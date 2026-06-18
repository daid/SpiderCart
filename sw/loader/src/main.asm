#INCLUDE "gbz80/all.asm"
#INCLUDE "gbz80/extra/if.asm"
#INCLUDE "gbz80/extra/loop.asm"
#INCLUDE "gbz80/extra/ld16.asm"
#INCLUDE "gbz80/extra/pushpop.asm"

#INCLUDE "video.asm"
#INCLUDE "input.asm"
#INCLUDE "copy.asm"

#INCSDCC "main.rel"

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

    ld   a, LCDC_ON | LCDC_BG_ON
    ldh  [rLCDC], a
    call waitVBlank

    ld   hl, defaultPalette
    call setBGPalette

    jp   _main

HelloWorld:
    db "Hello World!", 0

defaultPalette:
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
    dw $FFFF, $0000, $FFFF, $0000
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
