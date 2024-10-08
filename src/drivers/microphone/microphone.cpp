#include <stdio.h>
#include <vector>
#include <cmath>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/adc.h"
#include "drivers/logging/logging.h"
#include "microphone.h"
#include "board.h"

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

void Microphone::read()
{
    adc_run(true);
    adc_fifo_get_blocking();
    busy_wait_ms(10);
    adc_run(false);
    adc_fifo_drain();
}