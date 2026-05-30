#INCLUDE "gbz80/all.asm"
#INCLUDE "gbz80/extra/if.asm"
#INCLUDE "gbz80/extra/loop.asm"
#INCLUDE "gbz80/extra/ld16.asm"

#INCLUDE "video.asm"

GBC_HEADER "SpiderCart", GB_MBC_ROM_RAM, entry

#SECTION "Entry", ROM0 {
entry:
    call lcd_off
    ld   de, fontData
    ld   hl, $9000
    ld   bc, fontData.end - fontData

.copyFontData:
    ld   a, [de]
    inc  de
    ld   [hl+], a
    ld   [hl+], a
    dec  bc
    ld   a, b
    or   c
    jr   nz, .copyFontData
    
loop:
    halt
    jr loop
}

#SECTION "Font", ROMX, BANK[1] {
fontData:
    #INCGFX "font.1bpp.png", BPP[1], COLORMAP[$000000, $FFFFFF], DEBUG
.end:
}

#SECTION "SRAM", SRAM {
    ; For now ensure we have at least one SRAM section so the header has SRAM marked and BGB won't complain.
}