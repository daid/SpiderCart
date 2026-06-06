#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pins.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void write_memchip(uint32_t addr, const uint8_t* data, size_t size)
{
    gpio_put(PIN_BUS_EN, false); // disable GB cartridge bus
    gpio_set_dir_out_masked64(
        (0xFFFFULL << PIN_GB_A0) |
        (0x00FFULL << PIN_GB_D0)
    );
    while(size) {
        gpio_put_masked(0x3FFF << PIN_GB_A0, (addr & 0x3FFF) << PIN_GB_A0);
        gpio_put_masked(0x01FF << PIN_MBC_A0, (addr & 0x7FC000) << (PIN_MBC_A0 - 14));
        gpio_put_masked64(0x00FFULL << PIN_GB_D0, uint64_t(*data) << PIN_GB_D0);
        gpio_put(PIN_MEM_WE, false);
        sleep_us(1);
        gpio_put(PIN_MEM_WE, true);
        data++;
        addr++;
        size--;
    }
    gpio_set_dir_in_masked64(
        (0xFFFFULL << PIN_GB_A0) |
        (0x00FFULL << PIN_GB_D0)
    );
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
        sleep_us(1);
        *data = gpio_get_all64() >> PIN_GB_D0;
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

int main() {
    gpio_init_mask(
        (0xFFFF << PIN_GB_A0) |
        (0x0001 << PIN_GB_RD) |
        (0x0001 << PIN_GB_WR) |
        (0x0001 << PIN_GB_CS) |
        (0x01FF << PIN_MBC_A0) |
        (0x0001 << PIN_MEM_CE) |
        (0x0001 << PIN_MEM_OE) |
        (0x0001 << PIN_MEM_WE)
    );
    gpio_set_dir_out_masked(
        (0x01FF << PIN_MBC_A0) |
        (0x0001 << PIN_MEM_CE) |
        (0x0001 << PIN_MEM_OE) |
        (0x0001 << PIN_MEM_WE)
    );
    gpio_put(PIN_MEM_CE, false);
    gpio_put(PIN_MEM_OE, true);
    gpio_put(PIN_MEM_WE, true);
    gpio_init(PIN_GB_D0);
    gpio_init(PIN_GB_D1);
    gpio_init(PIN_GB_D2);
    gpio_init(PIN_GB_D3);
    gpio_init(PIN_GB_D4);
    gpio_init(PIN_GB_D5);
    gpio_init(PIN_GB_D6);
    gpio_init(PIN_GB_D7);
    gpio_init(PIN_BUS_EN);
    gpio_set_dir(PIN_BUS_EN, true);
    gpio_put(PIN_BUS_EN, true);
    gpio_init(PIN_GB_RST);
    gpio_set_dir(PIN_GB_RST, true);
    gpio_put(PIN_GB_RST, false);
    gpio_init(PIN_RUMBLE);
    gpio_set_dir(PIN_RUMBLE, true);
    gpio_put(PIN_RUMBLE, false);

    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);

    spi_init(spi1, 100 * 1000);
    //spi_set_format(spi1, 8, 
    gpio_set_function(PIN_SD_MISO, GPIO_FUNC_SPI);
    gpio_init(PIN_SD_CS);
    gpio_set_dir(PIN_SD_CS, true);
    gpio_put(PIN_SD_CS, true);
    gpio_set_function(PIN_SD_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SD_MOSI, GPIO_FUNC_SPI);

    stdio_init_all();
    while(1) {
        gpio_put(PIN_RUMBLE, true);
        sleep_ms(250);
        gpio_put(PIN_RUMBLE, false);
        sleep_ms(250);

        auto c = stdio_getchar_timeout_us(0);
        if (c == 'S') {
            printf("\nI2C Bus Scan\n");
            printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
            for (int addr = 0; addr < (1 << 7); ++addr) {
                if (addr % 16 == 0) {
                    printf("%02x ", addr);
                }

                // Perform a 1-byte dummy read from the probe address. If a slave
                // acknowledges this address, the function returns the number of bytes
                // transferred. If the address byte is ignored, the function returns
                // -1.

                // Skip over any reserved addresses.
                int ret;
                uint8_t rxdata;
                if (reserved_addr(addr))
                    ret = PICO_ERROR_GENERIC;
                else
                    ret = i2c_read_blocking(i2c1, addr, &rxdata, 1, false);

                printf(ret < 0 ? "." : "@");
                printf(addr % 16 == 15 ? "\n" : "  ");
            }
            printf("Done.\n");
        }
        if (c == 'M') {
            for(uint32_t addr = 0; addr < 0x8000; addr+=4) {
                write_memchip(addr, (uint8_t*)&addr, 4);
            }
            for(uint32_t addr = 0; addr < 0x8000; addr+=4) {
                uint32_t tmp = 0;
                read_memchip(addr, (uint8_t*)&tmp, 4);
                if (tmp != addr)
                    printf("Fail: %08x != %08x\n", addr, tmp);
            }
            printf("MEMCHECK DONE\n");
        }
        if (c == 'A') {
            uint8_t data[6];
            data[0] = 0x20; // CTRL_REG1
            data[1] = 0x57;
            i2c_write_blocking(i2c1, 0x19, data, 2, false);
            for(int n=0; n<32; n++) {
                data[0] = 0x28 | 0x80;
                i2c_write_blocking(i2c1, 0x19, data, 1, true);
                i2c_read_blocking(i2c1, 0x19, data, 6, false);
                printf("%x ", data[0] | (data[1] << 8));
                printf("%x ", data[2] | (data[3] << 8));
                printf("%x\n", data[4] | (data[5] << 8));
                sleep_ms(10);
            }
        }
        if (c == 'C') {
            uint8_t data[16];
            data[0] = 0x00;
            i2c_write_blocking(i2c1, 0x51, data, 1, true);
            i2c_read_blocking(i2c1, 0x51, data, 16, false);
            for(int n=0; n<16; n++) printf("%02x: %02x\n", n, data[n]);
        }
    }
    return 0;
}
