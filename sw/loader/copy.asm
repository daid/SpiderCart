
#SECTION "ClearMem", ROM0 {
    ; Clear HL with the size of BC, requires BC!=0
clearMem:
    xor a
    ; Set HL with the size of BC to A
setMem:
    ; Apply some trickery to set bc in such a way that dec c/b works correctly
    inc b
    dec bc
    inc c
.loop:
    ld  [hl+], a
    dec c
    jr  nz, .loop
    dec b
    jr  nz, .loop
    ret
}

#SECTION "ClearMemSmall", ROM0 {
    ; Clear HL with the size of C, requires C!=0
clearMemSmall:
    xor a
    ; Set HL with the size of BC to A
setMemSmall:
.loop:
    ld  [hl+], a
    dec c
    jr  nz, .loop
    ret
}

#SECTION "CopyData", ROM0 {
    ; Copy HL to DE with size of BC, requires bc!=00
copyMem:
    ; Apply some trickery to set bc in such a way that dec c/b works correctly
    inc b
    dec bc
    inc c
.loop:
    ld  a, [hl+]
    ld  [de], a
    inc de
    dec c
    jr  nz, .loop
    dec b
    jr  nz, .loop
    ret
}

#SECTION "CopyData1BPP", ROM0 {
    ; Copy HL to DE with size of BC, duplicating each byte from HL, requires bc!=00
copy1BPP:
    ; Apply some trickery to set bc in such a way that dec c/b works correctly
    inc b
    dec bc
    inc c
.loop:
    ld  a, [hl+]
    ld  [de], a
    inc de
    ld  [de], a
    inc de
    dec c
    jr  nz, .loop
    dec b
    jr  nz, .loop
    ret
}

#SECTION "CopyDataSmall", ROM0 {
    ; Copy HL to DE with size of C, requires c!=00
copyMemSmall:
.loop:
    ld  a, [hl+]
    ld  [de], a
    inc de
    dec c
    jr  nz, .loop
    ret
}
