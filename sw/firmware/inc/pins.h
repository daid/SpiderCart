#pragma once

#define PIN_GB_A0  0
#define PIN_GB_A14 14
#define PIN_GB_A15 15

#define PIN_GB_RD  16
#define PIN_GB_WR  17
#define PIN_GB_CS  18

#define PIN_MBC_A0 19
#define PIN_MBC_A1 20
#define PIN_MBC_A2 21
#define PIN_MBC_A3 22
#define PIN_MBC_A4 23
#define PIN_MBC_A5 24
#define PIN_MBC_A6 25
//MBC_A7 and MBC_A8 are not connected in V1
#define PIN_MBC_A7 26
#define PIN_MBC_A8 27

#define PIN_MEM_CE 28
#define PIN_MEM_OE 29
#define PIN_MEM_WE 30

#define PIN_GB_D0  31
#define PIN_GB_D1  32
#define PIN_GB_D2  33
#define PIN_GB_D3  34
#define PIN_GB_D4  35
#define PIN_GB_D5  36
#define PIN_GB_D6  37
#define PIN_GB_D7  38

#define PIN_BUS_EN 39

#define PIN_GB_RST 40
#define PIN_RUMBLE 41
#define PIN_SDA    42
#define PIN_SCL    43
#define PIN_SD_MISO 44
#define PIN_SD_CS   45
#define PIN_SD_CLK  46
#define PIN_SD_MOSI 47

#define PIN_MASK_GB_A0 (1 << PIN_GB_A0)
#define PIN_MASK_GB_A14 (1 << PIN_GB_A14)
#define PIN_MASK_GB_A15 (1 << PIN_GB_A15)

#define PIN_MASK_GB_RD (1 << PIN_GB_RD)
#define PIN_MASK_GB_WR (1 << PIN_GB_WR)
#define PIN_MASK_GB_CS (1 << PIN_GB_CS)

#define PIN_MASK_MBC_A0 (1 << PIN_MBC_A0)
#define PIN_MASK_MBC_A1 (1 << PIN_MBC_A1)
#define PIN_MASK_MBC_A2 (1 << PIN_MBC_A2)
#define PIN_MASK_MBC_A3 (1 << PIN_MBC_A3)
#define PIN_MASK_MBC_A4 (1 << PIN_MBC_A4)
#define PIN_MASK_MBC_A5 (1 << PIN_MBC_A5)
#define PIN_MASK_MBC_A6 (1 << PIN_MBC_A6)
//MBC_A7 and MBC_A8 are not connected in V1
#define PIN_MASK_MBC_A7 (1 << PIN_MBC_A7)
#define PIN_MASK_MBC_A8 (1 << PIN_MBC_A8)

#define PIN_MASK_MEM_CE (1 << PIN_MEM_CE)
#define PIN_MASK_MEM_OE (1 << PIN_MEM_OE)
#define PIN_MASK_MEM_WE (1 << PIN_MEM_WE)

#define PIN_MASK_GB_D0 (1 << PIN_GB_D0)
#define PIN_MASK_GB_D1 (1 << PIN_GB_D1)
#define PIN_MASK_GB_D2 (1 << PIN_GB_D2)
#define PIN_MASK_GB_D3 (1 << PIN_GB_D3)
#define PIN_MASK_GB_D4 (1 << PIN_GB_D4)
#define PIN_MASK_GB_D5 (1 << PIN_GB_D5)
#define PIN_MASK_GB_D6 (1 << PIN_GB_D6)
#define PIN_MASK_GB_D7 (1 << PIN_GB_D7)

#define PIN_MASK_BUS_EN (1 << PIN_BUS_EN)

#define PIN_MASK_GB_RST (1 << PIN_GB_RST)
#define PIN_MASK_RUMBLE (1 << PIN_RUMBLE)
#define PIN_MASK_SDA (1 << PIN_SDA)
#define PIN_MASK_SCL (1 << PIN_SCL)
#define PIN_MASK_SD_MISO (1 << PIN_SD_MISO)
#define PIN_MASK_SD_CS (1 << PIN_SD_CS)
#define PIN_MASK_SD_CLK (1 << PIN_SD_CLK)
#define PIN_MASK_SD_MOSI (1 << PIN_SD_MOSI)
