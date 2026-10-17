/**
 * @file ir_rx.c
 * @brief Operation Ghost Light Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "ir_rx.h"
uint32_t simulated_ir_rx_code = 0x00FF00FF;

uint32_t get_latest_ir_code(void) {
    return simulated_ir_rx_code;
}
