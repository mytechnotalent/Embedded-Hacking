/**
 * @file main.c
 * @brief PWM demonstration: LED breathing effect via duty-cycle sweep.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates PWM output using the PWM driver. A signal on
 * GPIO 25 (onboard LED) sweeps its duty cycle from 0% to 100%
 * and back to produce a smooth breathing effect. The current
 * duty is reported over UART.
 *
 * Wiring:
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO25 -> Onboard LED (no external wiring needed)
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
#include "rp2350_pwm.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/** @brief PWM duty cycle increment per step */
#define SWEEP_STEP    5
/** @brief Delay between PWM sweep steps in milliseconds */
#define SWEEP_DELAY_MS 50

/**
  * @brief  Convert a uint8_t to a decimal string.
  * @param  value number to convert (0-255)
  * @param  buf   output buffer (at least 4 bytes)
  * @retval None
  */
static void uint8_to_str(uint8_t value, char *buf)
{
  uint8_t idx = 0;
  if (value >= 100)
    buf[idx++] = (char)('0' + value / 100);
  if (value >= 10)
    buf[idx++] = (char)('0' + (value / 10) % 10);
  buf[idx++] = (char)('0' + value % 10);
  buf[idx] = '\0';
}

/**
  * @brief  Print a duty percentage value as a decimal string over UART.
  * @param  duty duty cycle value (0-100)
  * @retval None
  */
static void print_duty(uint8_t duty)
{
  char buf[4];
  uint8_to_str(duty, buf);
  uart_puts("Duty: ");
  uart_puts(buf);
  uart_puts("%\r\n");
}

/**
  * @brief  Sweep duty cycle upward from 0 to 100 percent.
  * @retval None
  */
static void sweep_up(void)
{
  for (uint8_t duty = 0; duty <= 100; duty += SWEEP_STEP) 
  {
    pwm_set_duty(duty);
    print_duty(duty);
    delay_ms(SWEEP_DELAY_MS);
  }
}

/**
  * @brief  Sweep duty cycle downward from 100 to 0 percent.
  * @retval None
  */
static void sweep_down(void)
{
  for (int8_t duty = 100; duty >= 0; duty -= SWEEP_STEP) 
  {
    pwm_set_duty((uint8_t)duty);
    print_duty((uint8_t)duty);
    delay_ms(SWEEP_DELAY_MS);
  }
}

int main(void)
{
  uart_puts("PWM initialized: GPIO25\r\n");
  while (1) 
  {
    sweep_up();
    sweep_down();
  }
}
