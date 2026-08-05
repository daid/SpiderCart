#include "mbc.h"
#include "pins.h"
#include "coprocessor.h"

#include "data_write.pio.h"
#include "data_read.pio.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"


#define MBC_FLAG_MBC2     0x0100
#define MBC_FLAG_NO_BANK0 0x0200
#define MBC_FLAG_MBC7     0x0400
#define MBC_FLAG_SPIDER   0x1000


#define MBC_FUNC(name) __attribute__((noinline)) __scratch_x("mbc_handler_spider") name

template<uint32_t FLAGS> static inline void __attribute__((always_inline)) mbc_handler_impl(uint32_t rom_mask, uint32_t ram_mask)
{
    uint8_t data;
    uint32_t high_bank_pins = PIN_MASK_MEM_WE | PIN_MASK_MBC_A0;
    uint8_t* ram_ptr = ram_data;
    uint32_t ram_addr_mask = 0x1FFF;
    if constexpr (FLAGS & MBC_FLAG_MBC2) ram_addr_mask = 0x01FF;
    if constexpr (FLAGS & MBC_FLAG_MBC7) ram_addr_mask = 0x10F0;
    bool ram_enabled = false;
    bool timer_active = false;
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
                if (ram_enabled && !(input_values & (PIN_MASK_GB_CS | PIN_MASK_GB_A14))) {
                    data_write_set_out(pio0, 0);
                    if ((FLAGS & MBC_FLAG_TIMER) && timer_active) {
                        while(!(input_values & (PIN_MASK_GB_CS | PIN_MASK_GB_A14))) {
                            input_values = gpio_get_all();
                            pio_sm_put(pio0, 0, *ram_ptr);
                        }
                    } else {
                        while(!(input_values & (PIN_MASK_GB_CS | PIN_MASK_GB_A14))) {
                            pio_sm_put(pio0, 0, ram_ptr[input_values & ram_addr_mask]);
                            input_values = gpio_get_all();
                        }
                    }
                    data_write_set_in(pio0, 0);
                }
            }
        } else {
            gpio_put_all(PIN_MASK_MEM_OE | PIN_MASK_MEM_WE);
            if (!(input_values & PIN_MASK_GB_WR)) {
                if (!(input_values & (PIN_MASK_GB_CS | PIN_MASK_GB_A14))) {
                    //RAM write
                    if (ram_enabled) {
                        if ((FLAGS & MBC_FLAG_TIMER) && timer_active) {
                            ram_ptr[0] = data_read(pio0, 1);
                            ram_ptr[8] = 1;
                            sio_hw->fifo_wr = 2; // Signal RTC register write
                            __sev();
                        } else {
                            if constexpr (FLAGS & MBC_FLAG_MBC2) {
                                ram_ptr[input_values & ram_addr_mask] = data_read(pio0, 1) | 0xF0;
                            } else {
                                ram_ptr[input_values & ram_addr_mask] = data_read(pio0, 1);
                            }
                            if constexpr (FLAGS & MBC_FLAG_BATTERY) {
                                sio_hw->fifo_wr = 0; // Signal RAM update for saving
                                __sev();
                            }
                            if constexpr (FLAGS & MBC_FLAG_MBC7) {
                                sio_hw->fifo_wr = 3; // Signal MBC7 write has been done
                                __sev();
                            }
                        }
                    }
                } else {
                    if constexpr (FLAGS & MBC_FLAG_MBC2)
                    {
                        //MBC2 doesn't follow standard convention and needs a lot of special handling.
                        if (input_values & 0x0100) {
                            data = data_read(pio0, 1) & rom_mask;
                            if constexpr (FLAGS & MBC_FLAG_NO_BANK0) {
                                if (data == 0) data = 1;
                            }
                            high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                        } else {
                            if constexpr (FLAGS & MBC_FLAG_RAM) {
                                ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                            }
                        }
                    } else {
                        switch(input_values & 0xF000) {
                        case 0x0000: //RAM Enable
                        case 0x1000:
                            if constexpr (FLAGS & MBC_FLAG_RAM) {
                                ram_enabled = (data_read(pio0, 1) & 0x0F) == 0x0A;
                            }
                            break;
                        case 0x2000: //ROM Bank nr
                        case 0x3000:
                            data = data_read(pio0, 1) & rom_mask;
                            if constexpr (FLAGS & MBC_FLAG_NO_BANK0) {
                                if (data == 0) data = 1;
                            }
                            high_bank_pins = PIN_MASK_MEM_WE | (data << PIN_MBC_A0);
                            break;
                        case 0x5000:
                            //MBC7 has an extra enable here, which we ignore for now.
                        case 0x4000: //RAM Bank nr
                            data = data_read(pio0, 1);
                            ram_ptr = ram_data + 0x2000 * (data & ram_mask);
                            if constexpr (FLAGS & MBC_FLAG_RUMBLE) {
                                if (data & 0x08)
                                    gpio_put(PIN_RUMBLE, true);
                                else
                                    gpio_put(PIN_RUMBLE, false);
                            }
                            if constexpr (FLAGS & MBC_FLAG_TIMER) {
                                if (data & 0x08) {
                                    ram_ptr = (ram_data + 0x2000 * 0x08) + (data & 0x07);
                                    timer_active = true;
                                } else {
                                    timer_active = false;
                                }
                            }
                            break;
                        case 0x6000:
                        case 0x7000:
                            if constexpr (FLAGS & MBC_FLAG_SPIDER) {
                                //Direct command to co-processor
                                //Direct IO access, if the fifo is full, command gets ignored instead of blocking here.
                                sio_hw->fifo_wr = data_read(pio0, 1) | 0x100;
                                __sev();
                            }
                            if constexpr (FLAGS & MBC_FLAG_TIMER) {
                                if (data_read(pio0, 1) == 0x01) {
                                    sio_hw->fifo_wr = 1; // signal core0 to update RTC registers
                                    __sev();
                                }
                            }
                            break;
                        }
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}

void MBC_FUNC(mbc_handler_spider)()
{
    //Spider MBC is:
    //  always 256 rom banks, fixed bank0, mbc5 style bank switching
    //  ram available mbc5 style with 16 banks, no automatic saving
    //  writes to 0x6000-0x7FFF for special commands
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_SPIDER>(0xFFFFFFFF, 0x0F);
}

void MBC_FUNC(mbc2_handler)(uint32_t rom_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_MBC2 | MBC_FLAG_NO_BANK0>(rom_mask, 0);
}

void MBC_FUNC(mbc2_handler_battery)(uint32_t rom_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_MBC2 | MBC_FLAG_NO_BANK0 | MBC_FLAG_BATTERY>(rom_mask, 0);
}

void MBC_FUNC(mbc3_handler)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc3_handler_battery)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_BATTERY>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc3_handler_timer)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_TIMER>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc3_handler_battery_timer)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_BATTERY | MBC_FLAG_TIMER>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc5_handler)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc5_handler_battery)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_BATTERY>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc5_handler_rumble)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_RUMBLE>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc5_handler_battery_rumble)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_BATTERY | MBC_FLAG_RUMBLE>(rom_mask, ram_mask);
}

void MBC_FUNC(mbc7_handler)(uint32_t rom_mask, uint32_t ram_mask)
{
    mbc_handler_impl<MBC_FLAG_RAM | MBC_FLAG_MBC7>(rom_mask, ram_mask);
}

void core1_start_mbc(MBC_Type base_type, uint32_t flags, uint32_t rom_bank_mask, uint32_t ram_bank_mask)
{
    switch(base_type)
    {
    case MBC_Type::Spider:
        mbc_handler_spider();
        break;
    case MBC_Type::MBC2:
        switch(flags & (MBC_FLAG_BATTERY)) {
        case 0:
            mbc2_handler(rom_bank_mask);
            break;
        case MBC_FLAG_BATTERY:
            mbc2_handler_battery(rom_bank_mask);
            break;
        }
        break;
    case MBC_Type::MBC3:
        switch(flags & (MBC_FLAG_BATTERY | MBC_FLAG_TIMER)) {
        case 0:
            mbc3_handler(rom_bank_mask, ram_bank_mask);
            break;
        case MBC_FLAG_BATTERY:
            mbc3_handler_battery(rom_bank_mask, ram_bank_mask);
            break;
        case MBC_FLAG_TIMER:
            mbc3_handler_timer(rom_bank_mask, ram_bank_mask);
            break;
        case MBC_FLAG_BATTERY | MBC_FLAG_TIMER:
            mbc3_handler_battery_timer(rom_bank_mask, ram_bank_mask);
            break;
        }
        break;
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
    case MBC_Type::MBC7:
        mbc7_handler(rom_bank_mask, ram_bank_mask);
        break;
    default:
    case MBC_Type::Unknown:
    case MBC_Type::MBC1:
        mbc5_handler(rom_bank_mask, ram_bank_mask);
        break;
    }
}
