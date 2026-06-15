#include "mbc.h"
#include "pins.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"


extern uint8_t ram_data[16 * 0x2000];


void __scratch_x("core1_handler") core1_mbc_handler()
{
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
                    case 0x6000: //
                        break;
                    }
                }
                while(!(gpio_get_all() & PIN_MASK_GB_WR)) {}
            }
        }
    }
}
