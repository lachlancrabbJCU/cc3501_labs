#include <stdio.h>
#include <vector>
#include <array>
#include <cmath>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/adc.h"
#include "drivers/logging/logging.h"
#include "microphone.h"
#include "board.h"
#include "arm_math.h"

Microphone::Microphone()
{
    init();
}

void Microphone::init()
{
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);
    adc_set_clkdiv(ClockDivider);
    adc_fifo_setup(true, false, false, false, false);
}

void Microphone::read(uint16_t *adc_sample)
{
    // std::array<uint16_t, 1024> adc_samples{};
    adc_run(true);
    for (int i = 0; i < 1024; i++)
    {
        adc_sample[i] = adc_fifo_get_blocking();
        busy_wait_us(AdcPeriodus);
    }
    // busy_wait_ms(10);
    adc_run(false);
    adc_fifo_drain();
}

void Microphone::apply_dc_offset(uint16_t *adc_samples, int16_t *time_domain)
{
    // Calculate the mean
    int32_t dc_bias{};
    for (int i = 0; i < 1024; i++)
    {
        dc_bias += adc_samples[i];
    }
    dc_bias = dc_bias / 1024;

    // Subtract off-set
    for (int i = 0; i < 1024; i++)
    {
        time_domain[i] = (adc_samples[i] - dc_bias) << 3;
    }
}

void Microphone::apply_window_function(int16_t *time_domain)
{
    // Apply a window function
    for (int i = 0; i < 1024; i++)
    {
        time_domain[i] = (static_cast<int32_t>(time_domain[i]) * static_cast<int32_t>(HanningWindow[i])) >> 15;
    }
}