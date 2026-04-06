/**
  ******************************************************************************
  * @file    rp2350_ir.c
  * @author  Kevin Thomas
  * @brief   NEC IR receiver driver implementation for RP2350.
  *
  *          Decodes NEC infrared remote frames using bare-metal SIO
  *          GPIO input and TIMER0 microsecond timestamps. Validates
  *          the 32-bit frame (addr, ~addr, cmd, ~cmd) and returns
  *          the command byte.
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

#include "rp2350_ir.h"

/**
  * @brief  Bit mask for the IR receiver pin.
  */
#define IR_PIN_MASK  (1U << IR_PIN)

/**
  * @brief  Read the TIMER0 raw low register for a microsecond timestamp.
  * @retval uint32_t current microsecond count
  */
static uint32_t _time_us(void)
{
  return TIMER0[TIMER_TIMERAWL_OFFSET];
}

/**
  * @brief  Read the current level of GPIO5 from SIO input register.
  * @retval bool true if pin is high, false if low
  */
static bool _read_pin(void)
{
  return (SIO[SIO_GPIO_IN_OFFSET] & IR_PIN_MASK) != 0;
}

/**
  * @brief  Wait for the pin to reach a given level with us timeout.
  * @param  level  desired logic level (true=high, false=low)
  * @param  timeout_us maximum wait in microseconds
  * @retval int32_t elapsed microseconds, or -1 on timeout
  */
static int32_t _wait_for_level(bool level, uint32_t timeout_us)
{
  uint32_t start = _time_us();
  while (_read_pin() != level) {
    if ((_time_us() - start) > timeout_us)
      return -1;
  }
  return (int32_t)(_time_us() - start);
}

/**
  * @brief  Validate the 9 ms leader mark pulse duration.
  * @retval bool true if within expected range, false on timeout or invalid
  */
static bool _validate_leader_mark(void)
{
  int32_t t = _wait_for_level(true, NEC_LEADER_MARK_TIMEOUT_US);
  if (t < (int32_t)NEC_LEADER_MARK_MIN_US)
    return false;
  return t <= (int32_t)NEC_LEADER_MARK_MAX_US;
}

/**
  * @brief  Validate the 4.5 ms leader space duration.
  * @retval bool true if within expected range, false on timeout or invalid
  */
static bool _validate_leader_space(void)
{
  int32_t t = _wait_for_level(false, NEC_LEADER_SPACE_TIMEOUT_US);
  if (t < (int32_t)NEC_LEADER_SPACE_MIN_US)
    return false;
  return t <= (int32_t)NEC_LEADER_SPACE_MAX_US;
}

/**
  * @brief  Wait for the NEC 9 ms leader pulse and 4.5 ms space.
  * @retval bool true if valid leader detected, false on timeout
  */
static bool _wait_leader(void)
{
  if (_wait_for_level(false, NEC_LEADER_WAIT_US) < 0)
    return false;
  if (!_validate_leader_mark())
    return false;
  return _validate_leader_space();
}

/**
  * @brief  Wait for the bit mark, then measure the space duration.
  * @param  duration_out pointer to store the space duration
  * @retval bool true on success, false on timeout or invalid
  */
static bool _measure_bit_space(int32_t *duration_out)
{
  if (_wait_for_level(true, NEC_BIT_MARK_TIMEOUT_US) < 0)
    return false;
  *duration_out = _wait_for_level(false, NEC_BIT_SPACE_TIMEOUT_US);
  return *duration_out >= (int32_t)NEC_BIT_SPACE_MIN_US;
}

/**
  * @brief  Read a single NEC-encoded bit by measuring space duration.
  * @param  data 4-byte array accumulating received bits
  * @param  bit  bit index (0-31)
  * @retval bool true on success, false on timeout
  */
static bool _read_nec_bit(uint8_t *data, uint8_t bit)
{
  int32_t t;
  if (!_measure_bit_space(&t))
    return false;
  if (t > (int32_t)NEC_BIT_ONE_THRESHOLD_US)
    data[bit / 8U] |= (1U << (bit % 8U));
  return true;
}

/**
  * @brief  Read all 32 data bits of an NEC frame.
  * @param  data 4-byte array filled with received data
  * @retval bool true if all bits read, false on timeout
  */
static bool _read_32_bits(uint8_t *data)
{
  for (uint8_t i = 0; i < NEC_DATA_BITS; i++)
    if (!_read_nec_bit(data, i))
      return false;
  return true;
}

/**
  * @brief  Validate NEC frame and extract command byte.
  * @param  data 4-byte NEC frame (addr, ~addr, cmd, ~cmd)
  * @retval int command byte (0-255) on success, -1 on failure
  */
static int _validate_nec_frame(const uint8_t *data)
{
  uint8_t addr_check = (uint8_t)(data[0] + data[1]);
  uint8_t cmd_check = (uint8_t)(data[2] + data[3]);
  if (addr_check == 0xFFU && cmd_check == 0xFFU)
    return (int)data[2];
  return -1;
}

/**
  * @brief  Clear the TIMER0 reset bit in the reset controller.
  * @retval None
  */
static void _timer_clear_reset(void)
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
static void _timer_wait_reset_done(void)
{
  while ((RESETS->RESET_DONE & (1U << RESETS_RESET_TIMER0_SHIFT)) == 0) {
  }
}

/**
  * @brief  Configure GPIO5 pad: enable input, pull-up, clear isolation.
  * @retval None
  */
static void _configure_pad(void)
{
  uint32_t value;
  value = PADS_BANK0->GPIO[IR_PIN];
  value &= ~(1U << PADS_BANK0_OD_SHIFT);
  value |= (1U << PADS_BANK0_IE_SHIFT);
  value |= (1U << PADS_BANK0_PUE_SHIFT);
  value &= ~(1U << PADS_BANK0_ISO_SHIFT);
  PADS_BANK0->GPIO[IR_PIN] = value;
}

/**
  * @brief  Set GPIO5 funcsel to SIO for software-controlled IO.
  * @retval None
  */
static void _configure_funcsel(void)
{
  IO_BANK0->GPIO[IR_PIN].CTRL = IO_BANK0_CTRL_FUNCSEL_SIO;
}

/**
  * @brief  Set GPIO5 as input via SIO output-enable clear register.
  * @retval None
  */
static void _set_input(void)
{
  SIO[SIO_GPIO_OE_CLR_OFFSET] = IR_PIN_MASK;
}

/**
  * @brief  Release TIMER0 from reset in the reset controller.
  * @retval None
  */
void ir_timer_release_reset(void)
{
  _timer_clear_reset();
  _timer_wait_reset_done();
}

/**
  * @brief  Start the TIMER0 tick generator for 1 us ticks at 12 MHz.
  * @retval None
  */
void ir_timer_start_tick(void)
{
  TICKS[TICKS_TIMER0_CYCLES_OFFSET] = TICKS_TIMER0_CYCLES_12MHZ;
  TICKS[TICKS_TIMER0_CTRL_OFFSET] = TICKS_TIMER0_ENABLE;
}

/**
  * @brief  Configure GPIO5 pad and funcsel for SIO input with pull-up.
  * @retval None
  */
void ir_init(void)
{
  _configure_pad();
  _configure_funcsel();
  _set_input();
}

/**
  * @brief  Block until a valid NEC frame is received or timeout.
  * @retval int command byte (0-255) on success, -1 on failure
  */
int ir_getkey(void)
{
  uint8_t data[NEC_DATA_BYTES] = {0};
  if (!_wait_leader())
    return -1;
  if (!_read_32_bits(data))
    return -1;
  return _validate_nec_frame(data);
}
