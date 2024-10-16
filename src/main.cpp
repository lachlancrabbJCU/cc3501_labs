#include <stdio.h>
#include <array>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include <iostream>
#include "WS2812.pio.h" // This header file gets produced during compilation from the WS2812.pio file
#include "drivers/logging/logging.h"
#include "drivers/WS2812/WS2812.h"
#include "drivers/LIS3DH/LIS3DH.h"
#include "drivers/microphone/microphone.h"
#include "colours.h"
#include "board.h"
#include "arm_math.h"

// volatile uint8_t current_state{};

using namespace std;

void spirit_level(Accelerometer accel, LedArray led_array, uint32_t x_level_colour, uint32_t y_level_colour)
{
    int16_t x_data{}, y_data{}, z_data{};
    float x_accel{}, y_accel{}, z_accel{};

    accel.read_accel_data(&x_data, &y_data, &z_data);

    x_accel = accel.convert_data(x_data);
    y_accel = accel.convert_data(y_data);
    z_accel = accel.convert_data(z_data);

    cout << "x: " << x_accel << ", y: " << y_accel << ", z: " << z_accel << "\n";

    uint8_t led_scaling_factor = abs(x_accel) * 255;
    uint32_t low_colour{(static_cast<uint32_t>(led_scaling_factor) << 24)};
    uint32_t high_colour{(static_cast<uint32_t>(led_scaling_factor) << 16)};
    led_array.set_all(LED_OFF);

    if (x_accel > 0 & x_accel < 0.1)
    {
        led_array.set(6, x_level_colour);
    }
    else if (x_accel > 0)
    {
        led_array.set(7, x_level_colour);
    }
    else if ((x_accel < 0) & (x_accel > -0.1))
    {
        led_array.set(5, x_level_colour);
    }
    else if (x_accel < 0)
    {
        led_array.set(4, x_level_colour);
    }
    else
    {
        led_array.set_multiple(5, x_level_colour, 6, x_level_colour);
    }

    if (y_accel > 0 & y_accel < 0.1)
    {
        led_array.set(2, y_level_colour);
    }
    else if (y_accel > 0)
    {
        led_array.set(3, y_level_colour);
    }
    else if ((y_accel < 0) & (y_accel > -0.1))
    {
        led_array.set(1, y_level_colour);
    }
    else if (y_accel < 0)
    {
        led_array.set(0, y_level_colour);
    }
    else
    {
        led_array.set_multiple(1, y_level_colour, 2, y_level_colour);
    }
    led_array.update();
    sleep_ms(200);
}

void led_stackup(LedArray led_array, uint32_t led_colour)
{
    for (size_t i = 0; i < NUMBER_OF_LEDS; i++)
    {
        for (size_t j = 0; j < NUMBER_OF_LEDS - i; j++)
        {
            led_array.set(j, led_colour);
            // if (j == 0)
            // {
            //     led_array.set(NUMBER_OF_LEDS - 1, LED_OFF);
            // }
            if (j > 0)
            {
                led_array.set(j - 1, LED_OFF);
            }
            led_array.update();
            sleep_ms(50);
        }
    }

    for (size_t i = (NUMBER_OF_LEDS + 1); i > 0; i--)
    {
        for (size_t j = NUMBER_OF_LEDS; j > (NUMBER_OF_LEDS - i); j--)
        {
            led_array.set((j - 1), LED_OFF);
            if (j < NUMBER_OF_LEDS)
            {
                led_array.set(j, led_colour);
            }
            led_array.update();
            sleep_ms(50);
        }
    }
}

// static auto gpio_irq_callback(uint gpio, uint32_t event_mask) -> void
// {
//     printf("Callback on GPIO %d\n", gpio);
//     if (current_state > 1)
//     {
//         current_state = 0;
//     }
//     else
//     {
//         current_state += 1;
//     }
//     busy_wait_ms(10);
// }

int main()
{
    stdio_init_all();
    Accelerometer accel;
    LedArray led_array(NUMBER_OF_LEDS);
    const uint32_t YLevelColour{LED_GREEN};
    const uint32_t XLevelColour{LED_BLUE};

    gpio_init(SW1);
    gpio_set_dir(SW1, GPIO_IN);

    // uint32_t event_mask = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
    // uint32_t event_mask = GPIO_IRQ_EDGE_RISE;
    // gpio_set_irq_enabled_with_callback(SW1, event_mask, true, &gpio_irq_callback);
    uint8_t current_state{1};

    Microphone microphone;
    microphone.init();
    // std::array<uint16_t, 1024> microphone_samples{};

    arm_rfft_instance_q15 microphone_fft_instance{};
    arm_rfft_init_q15(&microphone_fft_instance, 1024, 1, 1);

    while (true)
    {

        // spirit_level(accel, led_array, XLevelColour, YLevelColour);
        // led_stackup(led_array, LED_BLUE);

        // uint32_t random_colour{get_rand_32()};
        // for (size_t i = 0; i < NUMBER_OF_LEDS; i++)
        // {
        //     led_array.set(i, random_colour);
        // }
        //     switch (current_state)
        //     {
        //     case 0:
        //         led_stackup(led_array, LED_BLUE);
        //         break;
        //     case 1:
        //         spirit_level(accel, led_array, XLevelColour, YLevelColour);
        //         break;
        //     default:
        //         current_state = 0;
        //         break;
        //     }
        // }
        // return 0;
        uint16_t microphone_samples[1024]{};
        microphone.read(microphone_samples);
        int16_t time_domain[1024]{};
        microphone.apply_dc_offset(microphone_samples, time_domain);
        microphone.apply_window_function(time_domain);
        q15_t freq_domain[1026]{};

        arm_rfft_q15(&microphone_fft_instance, time_domain, freq_domain);
        for (int32_t i{}; i < 1026; i++)
        {
            freq_domain[i] = static_cast<q15_t>((static_cast<q31_t>(freq_domain[i]) << 10)); // convert 11.5 format to 1.15 format
        }
        q15_t cplx_mag_domain[1026]{};
        arm_cmplx_mag_squared_q15(freq_domain, cplx_mag_domain, 1026);
        for (int32_t i{}; i < 1026; i++)
        {
            cplx_mag_domain[i] = static_cast<q15_t>((static_cast<q31_t>(cplx_mag_domain[i]) << 2)); // convert 11.5 format to 1.15 format
        }
        for (int32_t sample : cplx_mag_domain)
        {
            std::cout << sample << "\n";
        }
        // cout << dc_bias << "\n";
    }
}
