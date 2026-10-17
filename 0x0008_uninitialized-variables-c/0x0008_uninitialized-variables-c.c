#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 16 

int main(void)
{
    //   gpio_init(LED_PIN);
    ///  gpio_set_dir(LED_PIN, GPIO_IN);
    gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
    ///  gpio_put(LED_PIN, 0);
    gpioc_bit_out_put(LED_PIN, 0);
    ///  gpio_set_function(LED_PIN, GPIO_FUNC_SIO);
    hw_write_masked(&pads_bank0_hw->io[LED_PIN],
                   PADS_BANK0_GPIO0_IE_BITS,
                   PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
    );
    io_bank0_hw->io[LED_PIN].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[LED_PIN], PADS_BANK0_GPIO0_ISO_BITS);

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
