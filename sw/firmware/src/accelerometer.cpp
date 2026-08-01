#include "hardware/i2c.h"


void accelerometer_read(uint16_t data[3])
{
    //Read out 16bit values, at +-2G full scale, LSB first
    uint8_t buffer[6];
    buffer[0] = 0x20; // CTRL_REG1
    buffer[1] = 0x57; // Enable all axis, 100Hz mode
    i2c_write_timeout_us(i2c1, 0x19, buffer, 2, false, 1000000);
    buffer[0] = 0x28 | 0x80;
    i2c_write_timeout_us(i2c1, 0x19, buffer, 1, true, 1000000);
    i2c_read_timeout_us(i2c1, 0x19, buffer, 6, false, 1000000);
    data[0] = buffer[0] | (buffer[1] << 8);
    data[1] = buffer[2] | (buffer[3] << 8);
    data[2] = buffer[4] | (buffer[5] << 8);
}