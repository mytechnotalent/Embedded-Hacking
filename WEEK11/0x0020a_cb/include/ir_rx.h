/**
 * @file ir_rx.h
 * @brief Operation Ghost Light Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef IR_RX_H
#define IR_RX_H
#include <stdint.h>

/**
 * @brief Get the most recently received infrared command code.
 * @param void No parameters.
 * @return The latest 32-bit infrared command code.
 */
uint32_t get_latest_ir_code(void);
#endif
