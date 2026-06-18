PADF_DOWN   = $80
PADF_UP     = $40
PADF_LEFT   = $20
PADF_RIGHT  = $10
PADF_START  = $08
PADF_SELECT = $04
PADF_B      = $02
PADF_A      = $01

PADB_DOWN   = $7
PADB_UP     = $6
PADB_LEFT   = $5
PADB_RIGHT  = $4
PADB_START  = $3
PADB_SELECT = $2
PADB_B      = $1
PADB_A      = $0

#SECTION "InputWRAM", WRAM0 {
_JoypadState:
wJoypadState:
    ds 1
_JoypadPressed:
wJoypadPressed:
    ds 1
}

#SECTION "InputCode", ROM0 {
; Call this routine once per frame to update the joypad related variables.
; Routine also returns the currently pressed buttons in the a register.
_updateJoypadState:
updateJoypadState:
    ld   hl, rP1
    ld   [hl], $10
    ; After the initial enable we need to read twice to ensure
    ; we get the proper hardware state on real hardware
    ld   a, [hl]
    ld   a, [hl]
    ld   [hl], $20
    cpl  ; Inputs are active low, so a bit being 0 is a button pressed. So we invert this.
    and  PADF_A | PADF_B | PADF_SELECT | PADF_START
    ld   c, a  ; Store the lower 4 button bits in c

    ; We need to read rP1 8 times to ensure the proper button state is available.
    ; This is only needed on real hardware, as it takes a while for the
    ; inputs to change state back from the first set.
    loop b, 8 {
        ld   a, [hl]
    }
    ld   [hl], $30 ; Disable the joypad inputs again, saves a tiny bit of power and allows the lines to settle before the next read

    swap a ; We want the directional keys as upper 4 bits, so swap the nibbles.
    cpl  ; Inputs are active low, so a bit being 0 is a button pressed. So we invert this.
    and  PADF_RIGHT | PADF_LEFT | PADF_UP | PADF_DOWN
    or   c
    ld   c, a

    ; Compare the new joypad state with the previous one, and store the
    ; new bits in wJoypadPressed
    ld   hl, wJoypadState
    xor  [hl]
    and  c
    ld   [wJoypadPressed], a
    ld   a, c
    ld   [wJoypadState], a

    ret
}