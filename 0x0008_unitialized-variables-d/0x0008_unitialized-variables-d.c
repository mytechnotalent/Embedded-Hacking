#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN 16 

int main(void)
{
    //   gpio_init(LED_PIN);
    ///  gpio_set_dir(LED_PIN, GPIO_IN);
    //// gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (LED_PIN), "r" (GPIO_OUT));
    ///  gpio_put(LED_PIN, 0);
    //// gpioc_bit_out_put(LED_PIN, 0);
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (LED_PIN), "r" (GPIO_OUT));
    ///  gpio_set_function(LED_PIN, GPIO_FUNC_SIO);
    //// hw_write_masked(&pads_bank0_hw->io[LED_PIN],
    ////                PADS_BANK0_GPIO0_IE_BITS,
    ////                PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
    //// );
    //// hw_xor_bits(addr, (*addr ^ values) & write_mask);
    pico_default_asm_volatile (
        "ldr r2, [%0]\n"                    // load current pad register
        "eor r2, r2, %1\n"                  // xor with IE bit
        "and r2, r2, %2\n"                  // mask with (IE|OD)
        "eor r2, r2, %1\n"                  // recombine (hw_xor_bits logic)
        "str r2, [%0]\n"                    // write back
        :
        : "r" (&pads_bank0_hw->io[LED_PIN]),
        "r" (PADS_BANK0_GPIO0_IE_BITS),
        "r" (PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS)
        : "r2", "memory"
    );

    ///  io_bank0_hw->io[LED_PIN].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    pico_default_asm_volatile (
        "str %1, [%0]\n"
        :
        : "r" (&io_bank0_hw->io[LED_PIN].ctrl),
        "r" (GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB)
        : "memory"
    );
    ///  hw_clear_bits(&pads_bank0_hw->io[LED_PIN], PADS_BANK0_GPIO0_ISO_BITS);
    pico_default_asm_volatile (
        "ldr r2, [%0]\n"                    // load current register value
        "bic r2, r2, %1\n"                  // clear the ISO bits (bit clear)
        "str r2, [%0]\n"                   // write back
        :
        : "r" (&pads_bank0_hw->io[LED_PIN]),
        "r" (PADS_BANK0_GPIO0_ISO_BITS)
        : "r2", "memory"
    );

    //   gpio_set_dir(LED_PIN, GPIO_OUT);
    ///  gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
    pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c4" : : "r" (LED_PIN), "r" (GPIO_OUT));

    while (true) {
        //   gpio_put(LED_PIN, 1);
        ///  gpioc_bit_out_put(LED_PIN, 1);
        pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c0" : : "r" (LED_PIN), "r" (1));
        ///  sleep_ms(500);
        sleep_us(500 * 1000ull);

        //   gpio_put(LED_PIN, 0);
        ///  gpioc_bit_out_put(LED_PIN, 0);
        pico_default_asm_volatile ("mcrr p0, #4, %0, %1, c0" : : "r" (LED_PIN), "r" (0));
        ///  sleep_ms(500);
        sleep_us(500 * 1000ull);
    }
}
