#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include <iostream>

#include "WS2812.pio.h" // This header file gets produced during compilation from the WS2812.pio file
#include "drivers/logging/logging.h"
#include "drivers/WS2812/WS2812.cpp"
#include "colours.h"

#define LED_PIN 14
#define NUMBER_OF_LEDS 12

using namespace std;

int main()
{
    stdio_init_all();

    // Initialise PIO0 to control the LED chain
    uint pio_program_offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, pio_program_offset, LED_PIN, 800000, false);
    LedArray led_array(NUMBER_OF_LEDS);

    for (;;)
    {
        // Test the log system
        log(LogLevel::INFORMATION, "Hello world");

        // Turn on the first LED to be a certain colour

        led_array.set(1, LED_RED);

        cout << led_array.is_updated(1) << endl;

        led_array.set(2, led_array.status(1));

        led_array.set(3, LED_GREEN);

        led_array.set(7, led_array.status(3));

        led_array.set_multiple(4, LED_ORANGE, 5, LED_INDIGO, 6, ~LED_ORANGE);

        // for (size_t i = 0; i < NUMBER_OF_LEDS; i++)
        // {
        //     for (size_t j = 0; j < NUMBER_OF_LEDS - i; j++)
        //     {
        //         led_array.set(j, LED_BLUE);
        //         // if (j == 0)
        //         // {
        //         //     led_array.set(NUMBER_OF_LEDS - 1, LED_OFF);
        //         // }
        //         if (j > 0)
        //         {
        //             led_array.set(j - 1, LED_OFF);
        //         }
        //         led_array.update();
        //         sleep_ms(50);
        //     }
        // }

        // for (size_t i = (NUMBER_OF_LEDS + 1); i > 0; i--)
        // {
        //     for (size_t j = NUMBER_OF_LEDS; j > (NUMBER_OF_LEDS - i); j--)
        //     {
        //         led_array.set((j - 1), LED_OFF);
        //         if (j < NUMBER_OF_LEDS)
        //         {
        //             led_array.set(j, LED_BLUE);
        //         }
        //         led_array.update();
        //         sleep_ms(50);
        //     }
        // }

        led_array.update();
        cout << led_array.is_updated(1) << endl;
        // sleep_ms(500);

        // led_array.off();

        sleep_ms(5000);
    }
    return 0;
}
