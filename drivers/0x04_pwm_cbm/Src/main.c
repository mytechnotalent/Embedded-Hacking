/**
  ******************************************************************************
  * @file    main.c
  * @author  Kevin Thomas
  * @brief   PWM demonstration: LED breathing effect via duty-cycle sweep.
  *
  *          Demonstrates PWM output using the PWM driver. A signal on
  *          GPIO 25 (onboard LED) sweeps its duty cycle from 0% to 100%
  *          and back to produce a smooth breathing effect. The current
  *          duty is reported over UART.
  *
  *          Wiring:
  *            GPIO0  -> UART TX (USB-to-UART adapter RX)
  *            GPIO1  -> UART RX (USB-to-UART adapter TX)
  *            GPIO25 -> Onboard LED (no external wiring needed)
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

#include "rp2350_pwm.h"
#include "rp2350_uart.h"
#include "rp2350_delay.h"

#define SWEEP_STEP    5
#define SWEEP_DELAY_MS 50

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
  * @brief  Print a duty percentage value as a decimal string over UART.
  * @param  duty duty cycle value (0-100)
  * @retval None
  */
static void _print_duty(uint8_t duty)
{
  char buf[4];
  _uint8_to_str(duty, buf);
  uart_puts("Duty: ");
  uart_puts(buf);
  uart_puts("%\r\n");
}

/**
  * @brief  Sweep duty cycle upward from 0 to 100 percent.
  * @retval None
  */
static void _sweep_up(void)
{
  for (uint8_t duty = 0; duty <= 100; duty += SWEEP_STEP) 
  {
    pwm_set_duty(duty);
    _print_duty(duty);
    delay_ms(SWEEP_DELAY_MS);
  }
}

/**
  * @brief  Sweep duty cycle downward from 100 to 0 percent.
  * @retval None
  */
static void _sweep_down(void)
{
  for (int8_t duty = 100; duty >= 0; duty -= SWEEP_STEP) 
  {
    pwm_set_duty((uint8_t)duty);
    _print_duty((uint8_t)duty);
    delay_ms(SWEEP_DELAY_MS);
  }
}

int main(void)
{
  uart_puts("PWM initialized: GPIO25\r\n");
  while (1) 
  {
    _sweep_up();
    _sweep_down();
  }
}
