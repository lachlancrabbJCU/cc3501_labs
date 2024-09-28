#include <stdio.h>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "drivers/logging/logging.h"
#include "board.h"
#include "LIS3DH.h"

#define CTRL_REG1_ADDR 0x20
#define CTRL_REG1_SET 0x77
#define ACCEL_REG_START 0x28
#define FIFO_CTRL_REG_ADDR 0x2E
#define CTRL_REG5_ADDR 0x24
#define FIFO_SET_MODE 0x40
#define FIFO_ENABLE 0x40

using namespace std;

Accelerometer::Accelerometer()
{
    // for (size_t i = 0; i < 6; i++)
    // {
    //     raw_data[i] = 0;
    // }
}

void Accelerometer::init()
{
    i2c_init(ACCEL_I2C_INSTANCE, 400 * 1000);
    gpio_set_function(ACCEL_SDA, GPIO_FUNC_I2C);
    gpio_set_function(ACCEL_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(ACCEL_SDA);
    gpio_pull_up(ACCEL_SCL);
    write(CTRL_REG1_ADDR, CTRL_REG1_SET);
    // write(FIFO_CTRL_REG_ADDR, FIFO_SET_MODE);
    // write(CTRL_REG5_ADDR, FIFO_ENABLE);
}

bool Accelerometer::write(uint8_t write_reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = write_reg;
    buf[1] = data;
    int bytes_written = i2c_write_blocking(ACCEL_I2C_INSTANCE, ACCEL_I2C_ADDRRESS, buf, 2, true);
    if (bytes_written != 2)
    {
        log(LogLevel::ERROR, "lis3dh:: write_registers: Failed to select register address.");
        return false;
    }
    return true;
}

bool Accelerometer::read(uint8_t read_reg, uint8_t *data, uint8_t bytes_to_read)
{
    read_reg |= 0x80;
    int16_t number_of_bytes_read = 0;
    if (1 != i2c_write_blocking(ACCEL_I2C_INSTANCE, ACCEL_I2C_ADDRRESS, &read_reg, 1, true))
    {
        log(LogLevel::ERROR, "lis3dh:: read_registers: Failed to select register address.");
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
    uint8_t read_register = ACCEL_REG_START;
    int16_t number_of_bytes_read = 0;
    // for (size_t i = 0; i < 6; i++)
    // {
    read(read_register, data, 6);
    // cout << number_of_bytes_read;
    // read_register += 0x01;
    // data[i] = raw_data;
    // raw_data = 0;
    // }
    // cout << "Data: " << data[1];
    *x_data = (int16_t)(data[0] | (data[1] << 8)) >> 6;
    *y_data = (int16_t)(data[2] | (data[3] << 8)) >> 6;
    *z_data = (int16_t)(data[4] | (data[5] << 8)) >> 6;
    // cout << "x: " << x_data << ", y: " << y_data << ", z: " << z_data << endl;
    // printf("X: %i, Y: %i, Z: %i", x_data, y_data, z_data);

    // float x_accel = convert_data(x_data);
    // float y_accel = convert_data(y_data);
    // float z_accel = convert_data(z_data);
    // cout << "x: " << x_accel << ", y: " << y_accel << ", z: " << z_accel << endl;
    return 1;
}

float Accelerometer::convert_data(int16_t axis_data)
{
    return (float)(axis_data * 4.0 / 1000);
}