#include "mbc.h"
#include "pins.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"


extern uint8_t ram_data[16 * 0x2000];

#define MBC_FUNC(name) __attribute__((noinline)) __scratch_x("mbc_handler_spider") name

void MBC_FUNC(mbc_handler_spider)()
{
    //Spider MBC is:
    //  always 256 rom banks, fixed bank0, mbc5 style bank switching
    //  ram available mbc5 style with 16 banks, no automatic saving
    //  writes to 0x6000-0x7FFF for special commands
    uint8_t data;
    uint32_t high_bank_pins = PIN_MASK_MEM_WE | PIN_MASK_MBC_A0;
    uint8_t* ram_ptr = ram_data;
    bool ram_enabled = false;
    while(true) {
        auto input_values = gpio_get_all();
        if (!(input_values & PIN_MASK_GB_RD)) {
            if (!(input_values & PIN_MASK_GB_A15)) {
                if (!(input_values & PIN_MASK_GB_A14)) {
                    gpio_put_all(PIN_MASK_MEM_WE);
                } else {
                    gpio_put_all(high_bank_pins);
                }
            } else {
                gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
                if (ram_enabled && !(input_values & PIN_MASK_GB_CS)) {
                    data_write_set_out(pio0, 0);
                    while(!(input_values & PIN_MASK_GB_CS)) {
                        input_values = gpio_get_all();
                        pio_sm_put(pio0, 0, ram_ptr[input_values & 0x1FFF]);
                    }
                    data_write_set_in(pio0, 0);
                }
            }
        } else {
            gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
            if (!(input_values & PIN_MASK_GB_WR)) {
                if (!(input_values & PIN_MASK_GB_CS)) {
                    //RAM write
                    if (ram_enabled)
                        ram_ptr[input_values & 0x1FFF] = data_read(pio0, 1);
                } else {
                    switch(input_values & 0xF000) {
                    case 0x0000: //RAM Enable
                        ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                        break;
                    case 0x2000: //ROM Bank nr
                        data = data_read(pio0, 1);
                        //if (!data) data = 1;
                        high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                        break;
                    case 0x4000: //RAM Bank nr
                        ram_ptr = ram_data + 0x2000 * (data_read(pio0, 1) & 0x0F);
                        break;
                    case 0x6000: //Direct command to co-processor
                        //Direct IO access, if the fifo is full, command gets ignored instead of blocking here.
                        sio_hw->fifo_wr = data_read(pio0, 1) | 0x100;
                        __sev();
                        break;
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}

void MBC_FUNC(mbc5_handler)(uint32_t rom_mask, uint32_t ram_mask)
{
    //MBC5 handler without battery or rumble
    uint8_t data;
    uint32_t high_bank_pins = PIN_MASK_MEM_WE | PIN_MASK_MBC_A0;
    uint8_t* ram_ptr = ram_data;
    bool ram_enabled = false;
    while(true) {
        auto input_values = gpio_get_all();
        if (!(input_values & PIN_MASK_GB_RD)) {
            if (!(input_values & PIN_MASK_GB_A15)) {
                if (!(input_values & PIN_MASK_GB_A14)) {
                    gpio_put_all(PIN_MASK_MEM_WE);
                } else {
                    gpio_put_all(high_bank_pins);
                }
            } else {
                gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
                if (ram_enabled && !(input_values & PIN_MASK_GB_CS)) {
                    data_write_set_out(pio0, 0);
                    while(!(input_values & PIN_MASK_GB_CS)) {
                        input_values = gpio_get_all();
                        pio_sm_put(pio0, 0, ram_ptr[input_values & 0x1FFF]);
                    }
                    data_write_set_in(pio0, 0);
                }
            }
        } else {
            gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
            if (!(input_values & PIN_MASK_GB_WR)) {
                if (!(input_values & PIN_MASK_GB_CS)) {
                    //RAM write
                    if (ram_enabled)
                        ram_ptr[input_values & 0x1FFF] = data_read(pio0, 1);
                } else {
                    switch(input_values & 0xF000) {
                    case 0x0000: //RAM Enable
                        ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                        break;
                    case 0x2000: //ROM Bank nr
                        data = data_read(pio0, 1) & rom_mask;
                        high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                        break;
                    case 0x4000: //RAM Bank nr
                        ram_ptr = ram_data + 0x2000 * (data_read(pio0, 1) & ram_mask);
                        break;
                    case 0x6000:
                        break;
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}

void MBC_FUNC(mbc5_handler_battery)(uint32_t rom_mask, uint32_t ram_mask)
{
    //MBC5 handler with battery, no rumble
    uint8_t data;
    uint32_t high_bank_pins = PIN_MASK_MEM_WE | PIN_MASK_MBC_A0;
    uint8_t* ram_ptr = ram_data;
    bool ram_enabled = false;
    while(true) {
        auto input_values = gpio_get_all();
        if (!(input_values & PIN_MASK_GB_RD)) {
            if (!(input_values & PIN_MASK_GB_A15)) {
                if (!(input_values & PIN_MASK_GB_A14)) {
                    gpio_put_all(PIN_MASK_MEM_WE);
                } else {
                    gpio_put_all(high_bank_pins);
                }
            } else {
                gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
                if (ram_enabled && !(input_values & PIN_MASK_GB_CS)) {
                    data_write_set_out(pio0, 0);
                    while(!(input_values & PIN_MASK_GB_CS)) {
                        input_values = gpio_get_all();
                        pio_sm_put(pio0, 0, ram_ptr[input_values & 0x1FFF]);
                    }
                    data_write_set_in(pio0, 0);
                }
            }
        } else {
            gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
            if (!(input_values & PIN_MASK_GB_WR)) {
                if (!(input_values & PIN_MASK_GB_CS)) {
                    //RAM write
                    if (ram_enabled) {
                        ram_ptr[input_values & 0x1FFF] = data_read(pio0, 1);
                        sio_hw->fifo_wr = 0; // Signal RAM update for saving
                        __sev();
                    }
                } else {
                    switch(input_values & 0xF000) {
                    case 0x0000: //RAM Enable
                        ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                        break;
                    case 0x2000: //ROM Bank nr
                        data = data_read(pio0, 1) & rom_mask;
                        high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                        break;
                    case 0x4000: //RAM Bank nr
                        ram_ptr = ram_data + 0x2000 * (data_read(pio0, 1) & ram_mask);
                        break;
                    case 0x6000:
                        break;
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}

void MBC_FUNC(mbc5_handler_rumble)(uint32_t rom_mask, uint32_t ram_mask)
{
    //MBC5 handler without battery, but with rumble
    uint8_t data;
    uint32_t high_bank_pins = PIN_MASK_MEM_WE | PIN_MASK_MBC_A0;
    uint8_t* ram_ptr = ram_data;
    bool ram_enabled = false;
    while(true) {
        auto input_values = gpio_get_all();
        if (!(input_values & PIN_MASK_GB_RD)) {
            if (!(input_values & PIN_MASK_GB_A15)) {
                if (!(input_values & PIN_MASK_GB_A14)) {
                    gpio_put_all(PIN_MASK_MEM_WE);
                } else {
                    gpio_put_all(high_bank_pins);
                }
            } else {
                gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
                if (ram_enabled && !(input_values & PIN_MASK_GB_CS)) {
                    data_write_set_out(pio0, 0);
                    while(!(input_values & PIN_MASK_GB_CS)) {
                        input_values = gpio_get_all();
                        pio_sm_put(pio0, 0, ram_ptr[input_values & 0x1FFF]);
                    }
                    data_write_set_in(pio0, 0);
                }
            }
        } else {
            gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
            if (!(input_values & PIN_MASK_GB_WR)) {
                if (!(input_values & PIN_MASK_GB_CS)) {
                    //RAM write
                    if (ram_enabled) {
                        ram_ptr[input_values & 0x1FFF] = data_read(pio0, 1);
                    }
                } else {
                    switch(input_values & 0xF000) {
                    case 0x0000: //RAM Enable
                        ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                        break;
                    case 0x2000: //ROM Bank nr
                        data = data_read(pio0, 1) & rom_mask;
                        high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                        break;
                    case 0x4000: //RAM Bank nr
                        data = data_read(pio0, 1);
                        ram_ptr = ram_data + 0x2000 * (data & ram_mask);
                        if (data & 0x08)
                            gpio_put(PIN_RUMBLE, true);
                        else
                            gpio_put(PIN_RUMBLE, false);
                        break;
                    case 0x6000:
                        break;
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}

void MBC_FUNC(mbc5_handler_battery_rumble)(uint32_t rom_mask, uint32_t ram_mask)
{
    //MBC5 handler without battery, but with rumble
    uint8_t data;
    uint32_t high_bank_pins = PIN_MASK_MEM_WE | PIN_MASK_MBC_A0;
    uint8_t* ram_ptr = ram_data;
    bool ram_enabled = false;
    while(true) {
        auto input_values = gpio_get_all();
        if (!(input_values & PIN_MASK_GB_RD)) {
            if (!(input_values & PIN_MASK_GB_A15)) {
                if (!(input_values & PIN_MASK_GB_A14)) {
                    gpio_put_all(PIN_MASK_MEM_WE);
                } else {
                    gpio_put_all(high_bank_pins);
                }
            } else {
                gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
                if (ram_enabled && !(input_values & PIN_MASK_GB_CS)) {
                    data_write_set_out(pio0, 0);
                    while(!(input_values & PIN_MASK_GB_CS)) {
                        input_values = gpio_get_all();
                        pio_sm_put(pio0, 0, ram_ptr[input_values & 0x1FFF]);
                    }
                    data_write_set_in(pio0, 0);
                }
            }
        } else {
            gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
            if (!(input_values & PIN_MASK_GB_WR)) {
                if (!(input_values & PIN_MASK_GB_CS)) {
                    //RAM write
                    if (ram_enabled) {
                        ram_ptr[input_values & 0x1FFF] = data_read(pio0, 1);
                        sio_hw->fifo_wr = 0; // Signal RAM update for saving
                        __sev();
                    }
                } else {
                    switch(input_values & 0xF000) {
                    case 0x0000: //RAM Enable
                        ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                        break;
                    case 0x2000: //ROM Bank nr
                        data = data_read(pio0, 1) & rom_mask;
                        high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                        break;
                    case 0x4000: //RAM Bank nr
                        data = data_read(pio0, 1);
                        ram_ptr = ram_data + 0x2000 * (data & ram_mask);
                        if (data & 0x08)
                            gpio_put(PIN_RUMBLE, true);
                        else
                            gpio_put(PIN_RUMBLE, false);
                        break;
                    case 0x6000:
                        break;
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}

void core1_start_mbc(MBC_Type base_type, uint32_t flags, uint32_t rom_bank_mask, uint32_t ram_bank_mask)
{
    switch(base_type)
    {
    case MBC_Type::Spider:
        mbc_handler_spider();
        break;
    case MBC_Type::Unknown:
    case MBC_Type::MBC1:
    case MBC_Type::MBC2:
    case MBC_Type::MBC3:
    case MBC_Type::MBC5:
        switch(flags & (MBC_FLAG_BATTERY | MBC_FLAG_RUMBLE)) {
        case 0:
            mbc5_handler(rom_bank_mask, ram_bank_mask);
            break;
        case MBC_FLAG_BATTERY:
            mbc5_handler_battery(rom_bank_mask, ram_bank_mask);
            break;
        case MBC_FLAG_RUMBLE:
            mbc5_handler_rumble(rom_bank_mask, ram_bank_mask);
            break;
        case MBC_FLAG_BATTERY | MBC_FLAG_RUMBLE:
            mbc5_handler_battery_rumble(rom_bank_mask, ram_bank_mask);
            break;
        }
        break;
    }
}
