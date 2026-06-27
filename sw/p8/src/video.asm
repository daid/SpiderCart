#MACRO STAT_WAIT {
:   ldh a, [rSTAT]
    and a, STAT_BUSY
    jr  nz, :-
}

#SECTION "VideoMem", WRAM0 {
wBGPalette:
_BGPalette:
    ds 8 * 4 * 2
}

#SECTION "Video", ROM0 {

lcd_off:
    ldh  a, [rLCDC]
    and  a, LCDC_ON
    ret  z
    call waitVBlank
    xor  a
    ldh  [rLCDC], a
    ret

_waitVBlank:
waitVBlank:
    xor  a
    ldh  [rIF], a
:   ldh  a, [rIF]
    and  a, IF_VBLANK
    jr   z, :-
    ret

setBGPalette:
_setBGPalette:
    ld   hl, wBGPalette
    ld   a, $80
    ldh  [rBCPS], a
    ld   c, 2 * 4 * 8
.copyLoop:   STAT_WAIT
    ld   a, [hl+]
    ldh  [rBCPD], a
    dec  c
    jr   nz, .copyLoop
    ret
}