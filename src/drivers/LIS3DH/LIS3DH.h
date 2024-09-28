#pragma once

#include <stdio.h>

class Accelerometer
{
public:
    Accelerometer();
    void init();
    bool write(uint8_t write_reg, uint8_t data);
    bool read(uint8_t read_reg, uint8_t *data, uint8_t bytes_to_read);
    bool read_accel_data(int16_t *x_data, int16_t *y_data, int16_t *z_data);
    float convert_data(int16_t axis_data);

private:
};