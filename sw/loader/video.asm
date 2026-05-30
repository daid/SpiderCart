#SECTION "Video", ROM0 {

lcd_off:
    ldh a, [rLCDC]
    and a, LCDC_ON
    ret z
.statWait:
    ldh a, [rSTAT]
    and a, STAT_VBLANK
    jr  z, .statWait

    xor a
    ldh [rLCDC], a
    ret

}