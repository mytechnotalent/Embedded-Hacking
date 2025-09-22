#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 16 

int main(void)
{
    //   gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_IN);
    gpio_put(LED_PIN, 0);
    gpio_set_function(LED_PIN, GPIO_FUNC_SIO);

    //   gpio_set_dir(LED_PIN, GPIO_OUT);
    gpioc_bit_oe_put(LED_PIN, GPIO_OUT);

    while (true) {
        //   gpio_put(LED_PIN, 1);
        gpioc_bit_out_put(LED_PIN, 1);
        //   sleep_ms(500);
        sleep_us(500 * 1000ull);

        //   gpio_put(LED_PIN, 0);
        gpioc_bit_out_put(LED_PIN, 0);
        //   sleep_ms(500);
        sleep_us(500 * 1000ull);
    }
}
