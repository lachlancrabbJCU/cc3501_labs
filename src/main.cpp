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

volatile int current_task{2}; // Global variable to track the current task

// Flag to track first entry into each task
bool first_entry{true}; // Will be reset when switching cases

// ISR for button press to switch between cases
void gpio_callback(uint gpio, uint32_t events);

void spirit_level(Accelerometer accel, LedArray led_array, uint32_t x_level_colour, uint32_t y_level_colour);

void led_stackup(LedArray led_array, uint32_t led_colour);

void audio_visualiser(Microphone microphone, LedArray led_array);

int main()
{
    stdio_init_all();
    Accelerometer accel;
    LedArray led_array(NUMBER_OF_LEDS);
    const uint32_t YLevelColour{LED_GREEN};
    const uint32_t XLevelColour{LED_BLUE};

    gpio_init(SW1);
    gpio_set_dir(SW1, GPIO_IN);

    uint8_t current_state{1};

    Microphone microphone;
    microphone.init();

    // Setup the switch GPIO for input with a pull-up resistor
    gpio_init(SW1);
    gpio_set_dir(SW1, GPIO_OUT);
    gpio_pull_up(SW1);

    // Set up an interrupt on the SWITCH_PIN for a falling edge (button press)
    gpio_set_irq_enabled_with_callback(SW1, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while (true)
    {
        switch (current_task)
        {
        case 0: // Lab 7: led_stackup
        {
            led_stackup(led_array, LED_CHARTREUSE);
        }
        case 1: // Lab 8: spirit_level
        {
            spirit_level(accel, led_array, XLevelColour, YLevelColour);
        }
        case 2: // Lab 9: Audio visualiser
        {
            audio_visualiser(microphone, led_array);
        }
        case 4: // Reset flags
        {
            first_entry = true;
            std::cout << "Entry flag reset." << std::endl;
            break;
        }

        default:
        {
            if (first_entry)
            {
                sleep_ms(1000);
                first_entry = false;
            }
            std::cout << "Invalid task." << std::endl;
            break;
        }
        }
    }
}

void audio_visualiser(Microphone microphone, LedArray led_array)
{
    uint16_t microphone_samples[1024]{};
    microphone.read(microphone_samples);
    int16_t time_domain[1024]{};
    microphone.apply_dc_offset(microphone_samples, time_domain);
    microphone.apply_window_function(time_domain);

    q15_t freq_domain[1026]{};
    microphone.apply_fft(time_domain, freq_domain);

    q15_t cplx_mag_domain[1026]{};
    microphone.get_complex_magnitude(freq_domain, cplx_mag_domain);

    uint32_t bin[12]{};
    microphone.process_results(cplx_mag_domain, bin);

    for (int i{}; i < 12; i++)
    {
        bin[i] <<= 8;
    }
    for (int i{}; i < 12; i++)
    {
        led_array.set(i, bin[i]);
    }

    // led_array.set_multiple(0, bin_6, 1, bin_8, 2, bin_11, 3, bin_16, 4, bin_24, 5, bin_35, 6, bin_51, 7, bin_75, 8, bin_110, 9, bin_161, 10, bin_237, 11, bin_349, 12, bin_513);
    led_array.update();
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
        busy_wait_ms(50);
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

void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == SW1)
    {
        current_task = (current_task + 1) % NUMBER_OF_TASKS;
        first_entry = true; // Set first_entry to true when switching to a new task
    }
}