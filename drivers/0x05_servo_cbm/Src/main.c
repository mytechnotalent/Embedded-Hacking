/**
 * @file main.c
 * @brief SG90 servo motor sweep demonstration.
 * @author Kevin Thomas
 * @date 2026
 *
 * Demonstrates servo control using the servo driver. The servo
 * sweeps from 0 to 180 degrees and back in 10-degree steps,
 * reporting each angle over UART.
 *
 * Wiring:
 *  GPIO0  -> UART TX (USB-to-UART adapter RX)
 *  GPIO1  -> UART RX (USB-to-UART adapter TX)
 *  GPIO6  -> Servo signal (orange/yellow)
 *  5V     -> Servo power (red)
 *  GND    -> Servo ground (brown/black)
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
#include "rp2350_servo.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

/** @brief Servo angle increment per step in degrees */
#define STEP_DEGREES  10
/** @brief Delay between servo sweep steps in milliseconds */
#define STEP_DELAY_MS 150

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
  * @brief  Print an angle value as a decimal string over UART.
  * @param  angle angle in degrees (0-180)
  * @retval None
  */
static void print_angle(uint8_t angle)
{
  char buf[4];
  uint8_to_str(angle, buf);
  uart_puts("Angle: ");
  uart_puts(buf);
  uart_puts(" deg\r\n");
}

/**
  * @brief  Sweep servo angle upward from 0 to 180 degrees.
  * @retval None
  */
static void sweep_up(void)
{
  for (uint8_t angle = 0; angle <= 180; angle += STEP_DEGREES) 
  {
    servo_set_angle(angle);
    print_angle(angle);
    delay_ms(STEP_DELAY_MS);
  }
}

/**
  * @brief  Sweep servo angle downward from 180 to 0 degrees.
  * @retval None
  */
static void sweep_down(void)
{
  for (int16_t angle = 180; angle >= 0; angle -= STEP_DEGREES) 
  {
    servo_set_angle((uint8_t)angle);
    print_angle((uint8_t)angle);
    delay_ms(STEP_DELAY_MS);
  }
}

int main(void)
{
  uart_puts("Servo driver initialized on GPIO6\r\n");
  uart_puts("Sweeping 0 -> 180 -> 0 degrees\r\n");
  while (1) 
  {
    sweep_up();
    sweep_down();
  }
}
