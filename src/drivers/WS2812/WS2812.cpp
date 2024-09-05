#include <stdio.h>
#include <vector>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "WS2812.pio.h" // This header file gets produced during compilation from the WS2812.pio file
#include "drivers/logging/logging.h"

#define LED_PIN 14

class LedArray
{
public:
    int number_of_leds;
    std::vector<uint32_t> led_data;

    LedArray(uint8_t leds) : number_of_leds(leds), led_data(leds, 0x00000000)
    {
        init();
    }

    void init()
    {
        for (size_t i = 0; i < number_of_leds; i++)
        {
            led_data[i] = 0x00000000;
        }
        update();
    }

    void set(uint8_t position, uint32_t colour)
    {
        if (position < number_of_leds)
        {
            led_data[position] = colour;
            // pio_sm_put_blocking(pio0, 0, colour); // Commented out, depends on your PIO implementation
        }
    }

    void update()
    {
        for (size_t i = 0; i < number_of_leds; i++)
        {
            // printf("0x%08x\n", led_data[i]);
            pio_sm_put_blocking(pio0, 0, led_data[i]);
            busy_wait_us(200);
        }
    }

    void off() const
    {
        for (size_t i = 0; i < number_of_leds; i++)
        {
            pio_sm_put_blocking(pio0, 0, 0x00000000);
        }
    }
};
