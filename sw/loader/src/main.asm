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

    ld   hl, execCommandCode
    ld   de, _execCommand
    ld   bc, execCommandCode.end - execCommandCode
    call copyMem
    ld   hl, execQuickbootCode
    ld   de, _execQuickboot
    ld   bc, execQuickbootCode.end - execQuickbootCode
    call copyMem

    ld   a, LCDC_ON | LCDC_BG_ON
    ldh  [rLCDC], a
    call waitVBlank

    ld   hl, defaultPalette
    call setBGPalette
    ld   hl, defaultPalette
    call setObjPalette

    jp   _main

HelloWorld:
    db "Hello World!", 0

defaultPalette:
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
    dw $7FFF, $5294, $35ad, $0000
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

#SECTION "ExecCommandCode", ROM0 {
execCommandCode:
    ; Execute the command in A and wait for it to complete.
    ld  hl, $0000
    ld  [hl], $0A ; enable SRAM
    ld  hl, $4000
    ld  [hl], $0F ; Switch to bank 15
    ld  hl, $BFFF
    ld  [hl], $FF ; Clear the ready flag
    dec hl
    ld  [hl], $5A ; Setup our bus available check
    dec hl
    ld  [hl], $A5 ; Setup our bus available check
    ld  hl, $6000
    ld  [hl], a   ; Request the command to be run.

    ; From this point on the cartridge bus might become unavailable while the cart is doing other things.
    ; So keep checking if the bus is available and if our result is available
.notReady:
    ld  hl, $0000
    ld  [hl], $0A ; reenable SRAM, MBC might have restarted
    ld  hl, $4000
    ld  [hl], $0F ; Switch to bank 15, MBC might have restarted
    ld  hl, $BFFF - 2
    ld  a, [hl+]
    cp  $A5
    jr  nz, .notReady
    ld  a, [hl+]
    cp  $5A
    jr  nz, .notReady
    ld  a, [hl-]
    ld  c, a
    cp  $FF
    jr  z, .notReady
    ld  a, [hl-]
    cp  $5A ; check our bus available bytes after the ready byte, as the bus might have gotten disabled between last check and ready read
    jr  nz, .notReady
    ld  a, [hl+]
    cp  $A5
    jr  nz, .notReady
    ld  a, c ; return the result
    ret
.end:
}
#SECTION "ExecQuickboot", ROM0 {
execQuickbootCode:
    call _execCommand ; First run the requested quickboot command
    and  a, a
    ret  nz ; first quickboot returned an error, so do not continue

    ; Next we request another command, but now we do a fixed delay and then jump to $0100
    ld  hl, $0000
    ld  [hl], $0A ; enable SRAM
    ld  hl, $4000
    ld  [hl], $0F ; Switch to bank 15
    ld  hl, $BFFF
    ld  [hl], $FF ; Clear the ready flag
    ld  hl, $6000
    ld  [hl], $F0 ; Request actual MBC switch

    ; From this point on the cartridge bus might become unavailable while the cart is resetting the MBC
    ; So delay a bit and then jump to the entry point
    ld  de, 0
:   dec e
    jr  nz, :-
    dec d
    jr  nz, :-
    ld  a, $11
    ld  sp, $FFFE

    jp  $0100
.end:
}

#SECTION "ExecCommandWRAM", WRAM0 {
_execCommand:
    ds 128
.end:
_execQuickboot:
    ds 128
.end:
#ASSERT _execCommand.end - _execCommand >= execCommandCode.end - execCommandCode
#ASSERT _execQuickboot.end - _execQuickboot >= execQuickbootCode.end - execQuickbootCode
}


#SECTION "Font", ROMX, BANK[1] {
fontData:
    #INCGFX "font.1bpp.png", BPP[1], COLORMAP[$000000, $FFFFFF]
.end:
}

#SECTION "SRAM", SRAM {
    ; For now ensure we have at least one SRAM section so the header has SRAM marked and BGB won't complain.
}
