/**
 * @file dazzler.c
 * @brief Operation Ghost Light Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "dazzler.h"
#include "ir_rx.h"
#include <stdio.h>
#include "pico/stdlib.h"

dazzler_t laser_sys;

void dazzler_init(void) {
    laser_sys.active_ir_code = 0x00FF00FF;
    laser_sys.is_locked_out = false;
}

/**
 * @brief Report that the dazzler system is locked out.
 * @param void No parameters.
 * @return void No return value.
 */
static void report_lockout_message(void) {
    printf("[!] SYSTEM LOCKED. REBOOT REQUIRED.\r\n");
}

/**
 * @brief Trigger the friendly fire lockout and halt the system.
 * @param void No parameters.
 * @return void No return value.
 */
static void trigger_lockout(void) {
    printf("[-] FRIENDLY FIRE LOCKOUT TRIGGERED. DAZZLER DISABLED.\r\n");
    laser_sys.is_locked_out = true;
    while (1) { tight_loop_contents(); }
}

/**
 * @brief Report the status of a non-lockout NEC command code.
 * @param code The decoded 32-bit NEC command code.
 * @return void No return value.
 */
static void report_code_status(uint32_t code) {
    if (code == 0xFF0055AA) {
        printf("[+] DAZZLER POWER LEVEL: MAX\r\n");
    } else if (code == 0xFF00AA55) {
        printf("[+] DAZZLER TRACKING: ENGAGED\r\n");
    } else if (code == 0xFF0011EE) {
        printf("[+] DAZZLER POWER LEVEL: LOW\r\n");
    } else if (code == 0x00FF00FF) {
        printf("[-] DAZZLER IDLE.\r\n");
    } else {
        printf("[?] UNKNOWN IR COMMAND: 0x%08X\r\n", code);
    }
}

/**
 * @brief Dispatch a decoded NEC command code to its handler.
 * @param code The decoded 32-bit NEC command code.
 * @return void No return value.
 */
static void dispatch_nec_code(uint32_t code) {
    if (code == 0xDEADBEEF) {
        trigger_lockout();
    } else {
        report_code_status(code);
    }
}

void __noinline parse_nec_code(void) {
    if (laser_sys.is_locked_out) {
        report_lockout_message();
        return;
    }
    // Decode and dispatch the latest NEC infrared command code.
    dispatch_nec_code(get_latest_ir_code());
}
