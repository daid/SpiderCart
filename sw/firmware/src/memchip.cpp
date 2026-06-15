#include "memchip.h"
#include "pins.h"

#include "hardware/gpio.h"

#include "data_write.pio.h"
#include "data_read.pio.h"


void write_memchip(uint32_t addr, const uint8_t* data, size_t size)
{
    gpio_put(PIN_BUS_EN, false); // disable GB cartridge bus
    gpio_set_dir_out_masked(
        (0xFFFFULL << PIN_GB_A0)
    );
    data_write_set_out(pio0, 0);
    while(size) {
        gpio_put_masked(0x3FFF << PIN_GB_A0, (addr & 0x3FFF) << PIN_GB_A0);
        gpio_put_masked(0x01FF << PIN_MBC_A0, (addr & 0x7FC000) << (PIN_MBC_A0 - 14));
        pio_sm_put(pio0, 0, *data);
        gpio_put(PIN_MEM_WE, false);
        asm volatile("nop\nnop\nnop");
        gpio_put(PIN_MEM_WE, true);
        data++;
        addr++;
        size--;
    }
    gpio_set_dir_in_masked(
        (0xFFFFULL << PIN_GB_A0)
    );
    data_write_set_in(pio0, 0);
    gpio_put(PIN_BUS_EN, true); // enable GB cartridge bus
}

void read_memchip(uint32_t addr, uint8_t* data, size_t size)
{
    gpio_put(PIN_BUS_EN, false); // disable GB cartridge bus
    gpio_set_dir_out_masked64(
        (0xFFFFULL << PIN_GB_A0)
    );
    while(size) {
        gpio_put_masked(0x7FFF << PIN_GB_A0, (addr & 0x7FFF) << PIN_GB_A0);
        gpio_put_masked(0x01FF << PIN_MBC_A0, (addr & 0x7FC000) << (PIN_MBC_A0 - 14));
        gpio_put(PIN_MEM_OE, false);
        asm volatile("nop\nnop\nnop");
        *data = data_read(pio0, 1);
        gpio_put(PIN_MEM_OE, true);
        data++;
        addr++;
        size--;
    }
    gpio_set_dir_in_masked64(
        (0xFFFFULL << PIN_GB_A0)
    );
    gpio_put(PIN_BUS_EN, true); // enable GB cartridge bus
}