#include <stdio.h>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "drivers/logging/logging.h"
#include "board.h"
#include "LIS3DH.h"

const uint I2cSpeed{400'000};
const uint8_t ControlRegister1Address{0x20};
const uint8_t ControlRegister1Set{0x77};
// const uint8_t ControlRegister2Address{0x24};
const uint8_t AccelerometerReadRegister{0x28};

using namespace std;

Accelerometer::Accelerometer()
{
    init();
}

void Accelerometer::init()
{
    i2c_init(ACCEL_I2C_INSTANCE, I2cSpeed);
    gpio_set_function(ACCEL_SDA, GPIO_FUNC_I2C);
    gpio_set_function(ACCEL_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(ACCEL_SDA);
    gpio_pull_up(ACCEL_SCL);
    write(ControlRegister1Address, ControlRegister1Set);
}

bool Accelerometer::write(uint8_t write_Register, uint8_t data)
{
    uint8_t buf[2]{write_Register, data};
    int bytes_written = i2c_write_blocking(ACCEL_I2C_INSTANCE, ACCEL_I2C_ADDRRESS, buf, 2, true);
    if (bytes_written != 2)
    {
        log(LogLevel::ERROR, "lis3dh:: write_registers: Failed to select register Addressess.");
        return false;
    }
    return true;
}

bool Accelerometer::read(uint8_t read_register, uint8_t *data, uint8_t bytes_to_read)
{
    read_register |= 0x80;
    int16_t number_of_bytes_read{};
    if (1 != i2c_write_blocking(ACCEL_I2C_INSTANCE, ACCEL_I2C_ADDRRESS, &read_register, 1, true))
    {
        log(LogLevel::ERROR, "lis3dh:: read_Registers: Failed to select Register Address.");
        return 0;
    }

    number_of_bytes_read = i2c_read_blocking(ACCEL_I2C_INSTANCE, ACCEL_I2C_ADDRRESS, data, bytes_to_read, false);
    if (number_of_bytes_read != bytes_to_read)
    {
        log(LogLevel::ERROR, "lis3dh::read_registers: Failed to read data.");
        return 0;
    }
    return 1;
}

bool Accelerometer::read_accel_data(int16_t *x_data, int16_t *y_data, int16_t *z_data)
{
    uint8_t data[6];
    uint8_t read_register = AccelerometerReadRegister;
    int16_t number_of_bytes_read{};

    read(read_register, data, 6);

    *x_data = (int16_t)(data[0] | (data[1] << 8)) >> 6;
    *y_data = (int16_t)(data[2] | (data[3] << 8)) >> 6;
    *z_data = (int16_t)(data[4] | (data[5] << 8)) >> 6;
    return 1;
}

float Accelerometer::convert_data(int16_t axis_data)
{
    return (float)(axis_data * 4.0 / 1000);
}