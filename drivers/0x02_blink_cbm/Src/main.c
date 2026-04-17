/**
 * @file main.c
 * @brief Blink demonstration: toggle onboard LED every 500 ms.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates GPIO output control using the blink driver.
 * The onboard LED on GPIO 25 is toggled every 500 ms and the
 * current state is reported over UART.
 *
 * MIT License
 *
 * Copyright (c) 2026 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "rp2350_led.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/** @brief Delay between LED state toggles in milliseconds */
#define BLINK_DELAY_MS 500

/**
  * @brief  Print the current LED state over UART.
  * @param  pin GPIO pin number to query
  * @retval None
  */
static void print_led_state(uint32_t pin)
{
  if (led_get_state(pin))
    uart_puts("LED: ON\r\n");
  else
    uart_puts("LED: OFF\r\n");
}

int main(void)
{
  led_init(LED_PIN);
  uart_puts("LED driver initialized on GPIO 25\r\n");
  while (1) 
  {
    led_toggle(LED_PIN);
    print_led_state(LED_PIN);
    delay_ms(BLINK_DELAY_MS);
  }
}
