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
    std::vector<uint32_t> last_update_led_data;

    LedArray(uint8_t leds) : number_of_leds(leds), led_data(leds, 0x00000000), last_update_led_data(leds, 0x00000000)
    {
        init();
    }

    void init()
    {
        for (size_t i = 0; i < number_of_leds; i++)
        {
            led_data[i] = 0;
            last_update_led_data[i] = 0;
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

    void set_multiple(uint8_t position, uint32_t colour)
    {
        set(position, colour);
    }

    template <typename... Args>
    void set_multiple(uint8_t position, uint32_t colour, Args... args)
    {
        set(position, colour);
        set_multiple(args...);
    }

    void set_all(uint32_t colour)
    {
        for (size_t i = 0; i < NUMBER_OF_LEDS; i++)
        {
            set(i, colour);
        }
    }

    void update()
    {
        for (size_t i = 0; i < number_of_leds; i++)
        {
            // printf("0x%08x\n", led_data[i]);
            pio_sm_put_blocking(pio0, 0, led_data[i]);
            // mark LED data as set by setting last bit to 1
            last_update_led_data[i] = led_data[i];
            busy_wait_us(200);
        }
    }

    uint32_t status(uint8_t position)
    {
        return last_update_led_data[position];
    }

    bool is_updated(uint8_t position)
    {
        return (led_data[position] == last_update_led_data[position]);
    }

    void off() const
    {
        for (size_t i = 0; i < number_of_leds; i++)
        {
            pio_sm_put_blocking(pio0, 0, 0);
        }
    }
};
