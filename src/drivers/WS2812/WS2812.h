#pragma once

#include <stdio.h>
#include <vector>

class LedArray
{
public:
    LedArray(int led_pin, int number_of_leds);
    void init();
    void set(uint8_t position, uint32_t colour);
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
    void set_all(uint32_t colour);
    uint32_t convert_hsv_rgb(uint16_t hue, float saturation, float value);
    void update();
    uint32_t status(uint8_t position);
    bool is_updated(uint8_t position);
    void off();

private:
    int led_gpio_pin;
    int number_of_leds;
    std::vector<uint32_t> led_data;
    std::vector<uint32_t> last_update_led_data;
};
