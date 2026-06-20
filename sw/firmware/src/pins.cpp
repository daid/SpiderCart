#include "pins.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/pio.h"

#include "data_write.pio.h"
#include "data_read.pio.h"


void hwInit()
{
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
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_init(PIN_SD_CS);
    gpio_set_dir(PIN_SD_CS, true);
    gpio_put(PIN_SD_CS, true);
    gpio_set_function(PIN_SD_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SD_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SD_MOSI, GPIO_FUNC_SPI);
    gpio_pull_up(PIN_SD_MISO);

    // Configure pio0 for pins 16-47, and setup our pio programs to allow one cycle read/write of the data bus.
    pio_set_gpio_base(pio0, 16);
    data_read_program_init(pio0, 1, pio_add_program(pio0, &data_read_program), PIN_GB_D0);
    data_write_program_init(pio0, 0, pio_add_program(pio0, &data_write_program), PIN_GB_D0);
}