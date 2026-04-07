/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   SG90 servo motor sweep demonstration.
  *
  *          Demonstrates servo control using the servo driver. The servo
  *          sweeps from 0 to 180 degrees and back in 10-degree steps,
  *          reporting each angle over UART.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO6  -> Servo signal (orange/yellow)
  *            5V     -> Servo power (red)
  *            GND    -> Servo ground (brown/black)
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "rp2350_servo.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

#define STEP_DEGREES  10
#define STEP_DELAY_MS 150

/**
  * @brief  Convert a uint8_t to a decimal string.
  * @param  value number to convert (0-255)
  * @param  buf   output buffer (at least 4 bytes)
  * @retval None
  */
static void _uint8_to_str(uint8_t value, char *buf)
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
static void _print_angle(uint8_t angle)
{
  char buf[4];
  _uint8_to_str(angle, buf);
  uart_puts("Angle: ");
  uart_puts(buf);
  uart_puts(" deg\r\n");
}

/**
  * @brief  Sweep servo angle upward from 0 to 180 degrees.
  * @retval None
  */
static void _sweep_up(void)
{
  for (uint8_t angle = 0; angle <= 180; angle += STEP_DEGREES) 
  {
    servo_set_angle(angle);
    _print_angle(angle);
    delay_ms(STEP_DELAY_MS);
  }
}

/**
  * @brief  Sweep servo angle downward from 180 to 0 degrees.
  * @retval None
  */
static void _sweep_down(void)
{
  for (int16_t angle = 180; angle >= 0; angle -= STEP_DEGREES) 
  {
    servo_set_angle((uint8_t)angle);
    _print_angle((uint8_t)angle);
    delay_ms(STEP_DELAY_MS);
  }
}

int main(void)
{
  uart_puts("Servo driver initialized on GPIO6\r\n");
  uart_puts("Sweeping 0 -> 180 -> 0 degrees\r\n");
  while (1) 
  {
    _sweep_up();
    _sweep_down();
  }
}
