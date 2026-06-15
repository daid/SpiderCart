#INCLUDE "gbz80/all.asm"
#INCLUDE "gbz80/extra/if.asm"
#INCLUDE "gbz80/extra/loop.asm"
#INCLUDE "gbz80/extra/ld16.asm"
#INCLUDE "gbz80/extra/pushpop.asm"

#INCLUDE "video.asm"
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

    ld   a, LCDC_ON | LCDC_BG_ON
    ldh  [rLCDC], a
    call waitVBlank

    ld   hl, defaultPalette
    call setBGPalette

    ld   a, $0A
    ld   [$0000], a
    ld   a, $0F
    ld   [$4000], a
    ld   a, $FF
    ld   [$BFFF], a
    ld   a, $01
    ld   [$6000], a

:   ld   a, [$BFFF]
    cp   $FF
    jr   z, :-

    ld   a, $00
    ld   [$4000], a

    ld   hl, $A000
    loop {
        ld   a, [hl]
        and  a, a
        jr   z, loop
        pushpop hl {
            inc  hl
            ld   e, l
            ld   a, h
            add  a, $98 - $A0
            ld   d, a
            call printStr
        }
        ld  de, $20
        add hl, de
    }

loop:
    halt
    jr loop

HelloWorld:
    db "Hello World!", 0

defaultPalette:
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
    dw $0000, $FFFF, $0000, $FFFF
}

#SECTION "PrintStr", ROM0 {
printStr: ; print the string at HL on the background at DE
    ld  a, [hl+]
    sub a, $20
    ret c
    ld  c, a
    STAT_WAIT
    ld  a, c
    ld  [de], a
    inc de
    jr  printStr
}

#SECTION "Font", ROMX, BANK[1] {
fontData:
    #INCGFX "font.1bpp.png", BPP[1], COLORMAP[$000000, $FFFFFF]
.end:
}

#SECTION "SRAM", SRAM {
    ; For now ensure we have at least one SRAM section so the header has SRAM marked and BGB won't complain.
}
