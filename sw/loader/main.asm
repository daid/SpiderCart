#INCLUDE "gbz80/all.asm"
#INCLUDE "gbz80/extra/if.asm"
#INCLUDE "gbz80/extra/loop.asm"
#INCLUDE "gbz80/extra/ld16.asm"

GBC_HEADER "SpiderCart", GB_MBC_ROM_RAM, entry

#SECTION "Entry", ROM0 {
entry:
    db $dd
}
