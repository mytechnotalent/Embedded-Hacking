/**
 * @file rp2350_dht11.c
 * @brief DHT11 temperature and humidity sensor driver for RP2350.
 * @author Kevin Thomas
 * @date 2026
 *
 * Implements the single-wire DHT11 protocol using bare-metal
 * SIO GPIO control and TIMER0 microsecond timestamps. Reads
 * 40 bits (humidity high, humidity low, temperature high,
 * temperature low, checksum) with timeout protection.
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
#include "rp2350_dht11.h"
#include "rp2350_delay.h"

/**
  * @brief  Bit mask for the DHT11 data pin.
  */
#define DHT11_PIN_MASK  (1U << DHT11_PIN)

/**
  * @brief  Read the TIMER0 raw low register for a microsecond timestamp.
  * @retval uint32_t current microsecond count
  */
static uint32_t time_us(void)
{
  return TIMER0[TIMER_TIMERAWL_OFFSET];
}

/**
  * @brief  Set GPIO4 as output via SIO output-enable set register.
  * @retval None
  */
static void set_output(void)
{
  SIO[SIO_GPIO_OE_SET_OFFSET] = DHT11_PIN_MASK;
}

/**
  * @brief  Set GPIO4 as input via SIO output-enable clear register.
  * @retval None
  */
static void set_input(void)
{
  SIO[SIO_GPIO_OE_CLR_OFFSET] = DHT11_PIN_MASK;
}

/**
  * @brief  Drive GPIO4 low via SIO output clear register.
  * @retval None
  */
static void drive_low(void)
{
  SIO[SIO_GPIO_OUT_CLR_OFFSET] = DHT11_PIN_MASK;
}

/**
  * @brief  Drive GPIO4 high via SIO output set register.
  * @retval None
  */
static void drive_high(void)
{
  SIO[SIO_GPIO_OUT_SET_OFFSET] = DHT11_PIN_MASK;
}

/**
  * @brief  Read the current level of GPIO4 from SIO input register.
  * @retval bool true if pin is high, false if low
  */
static bool read_pin(void)
{
  return (SIO[SIO_GPIO_IN_OFFSET] & DHT11_PIN_MASK) != 0;
}

/**
  * @brief  Wait for the pin to leave a given logic level with timeout.
  * @param  level logic level to wait through (true=high, false=low)
  * @retval bool true once the level changed, false on timeout
  */
static bool wait_for_level(bool level)
{
  uint32_t count = DHT11_TIMEOUT;
  while (read_pin() == level)
    if (--count == 0)
      return false;
  return true;
}

/**
  * @brief  Send the 18 ms low / 40 us high start signal to the DHT11.
  * @retval None
  */
static void send_start_signal(void)
{
  set_output();
  drive_low();
  delay_ms(DHT11_START_LOW_MS);
  drive_high();
  delay_us(DHT11_START_HIGH_US);
  set_input();
}

/**
  * @brief  Wait for the DHT11 response (low-high-low handshake).
  * @retval bool true if response received, false on timeout
  */
static bool wait_response(void)
{
  if (!wait_for_level(true))
    return false;
  if (!wait_for_level(false))
    return false;
  if (!wait_for_level(true))
    return false;
  return true;
}

/**
  * @brief  Wait for low-to-high transition and measure high-pulse width.
  * @param  duration_out pointer to store the pulse duration in microseconds
  * @retval bool true on success, false on timeout
  */
static bool measure_high_pulse(uint32_t *duration_out)
{
  uint32_t start;
  if (!wait_for_level(false))
    return false;
  start = time_us();
  if (!wait_for_level(true))
    return false;
  *duration_out = time_us() - start;
  return true;
}

/**
  * @brief  Read a single bit by measuring the high-pulse duration.
  * @param  data 5-byte array accumulating received bits
  * @param  bit  bit index (0-39)
  * @retval bool true on success, false on timeout
  */
static bool read_bit(uint8_t *data, uint8_t bit)
{
  uint32_t duration;
  if (!measure_high_pulse(&duration))
    return false;
  data[bit / 8U] <<= 1;
  if (duration > DHT11_BIT_THRESHOLD_US)
    data[bit / 8U] |= 1U;
  return true;
}

/**
  * @brief  Read all 40 data bits from the DHT11 sensor.
  * @param  data 5-byte array filled with received data
  * @retval bool true if all bits read, false on timeout
  */
static bool read_40_bits(uint8_t *data)
{
  for (uint8_t i = 0; i < DHT11_DATA_BITS; i++)
    if (!read_bit(data, i))
      return false;
  return true;
}

/**
  * @brief  Verify the DHT11 checksum (sum of bytes 0-3 vs byte 4).
  * @param  data 5 bytes of received sensor data
  * @retval bool true if checksum matches, false otherwise
  */
static bool validate_checksum(const uint8_t *data)
{
  uint8_t sum;
  sum = data[0] + data[1] + data[2] + data[3];
  return data[4] == sum;
}

/**
  * @brief  Clear the TIMER0 reset bit in the reset controller.
  * @retval None
  */
static void timer_clear_reset(void)
{
  uint32_t value;
  value = RESETS->RESET;
  value &= ~(1U << RESETS_RESET_TIMER0_SHIFT);
  RESETS->RESET = value;
}

/**
  * @brief  Wait until TIMER0 is out of reset.
  * @retval None
  */
static void timer_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_TIMER0_SHIFT)) == 0) {}
}

/**
  * @brief  Configure GPIO4 pad: enable input, pull-up, clear isolation.
  * @retval None
  */
static void configure_pad(void)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[DHT11_PIN];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value |= (1U << PADS_BANK0_PUE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[DHT11_PIN] = value;
}

/**
  * @brief  Set GPIO4 funcsel to SIO for software-controlled IO.
  * @retval None
  */
static void configure_funcsel(void)
{
  IO_BANK0->GPIO[DHT11_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_SIO;
}

void dht11_timer_release_reset(void)
{
  timer_clear_reset();
  timer_wait_reset_done();
}

void dht11_timer_start_tick(void)
{
  TICKS[TICKS_TIMER0_CYCLES_OFFSET] = TICKS_TIMER0_CYCLES_12MHZ;
  TICKS[TICKS_TIMER0_CTRL_OFFSET] = TICKS_TIMER0_ENABLE;
}

void dht11_init(void)
{
  configure_pad();
  configure_funcsel();
  set_input();
}

/**
  * @brief  Acquire 40 bits from the DHT11 and validate the checksum.
  * @param  data 5-byte output array for sensor data
  * @retval bool true on success, false on timeout or checksum error
  */
static bool acquire_data(uint8_t *data)
{
  send_start_signal();
  if (!wait_response())
    return false;
  if (!read_40_bits(data))
    return false;
  return validate_checksum(data);
}

bool dht11_read(uint8_t *humidity, uint8_t *temperature)
{
  uint8_t data[DHT11_DATA_BYTES] = {0};
  if (!acquire_data(data))
    return false;
  *humidity = data[0];
  *temperature = data[2];
  return true;
}
