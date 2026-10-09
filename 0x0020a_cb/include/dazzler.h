/**
 * @file dazzler.h
 * @brief Operation Ghost Light Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef DAZZLER_H
#define DAZZLER_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {
    uint32_t active_ir_code;
    bool is_locked_out;
} dazzler_t;
extern dazzler_t laser_sys;
void dazzler_init(void);
void parse_nec_code(void);
#endif
