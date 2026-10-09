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

void __noinline parse_nec_code(void) {
    if (laser_sys.is_locked_out) {
        printf("[!] SYSTEM LOCKED. REBOOT REQUIRED.\r\n");
        return;
    }
    
    uint32_t code = get_latest_ir_code();
    
    if (code == 0xFF0055AA) {
        printf("[+] DAZZLER POWER LEVEL: MAX\r\n");
    } else if (code == 0xFF00AA55) {
        printf("[+] DAZZLER TRACKING: ENGAGED\r\n");
    } else if (code == 0xFF0011EE) {
        printf("[+] DAZZLER POWER LEVEL: LOW\r\n");
    } else if (code == 0xDEADBEEF) {
        printf("[-] FRIENDLY FIRE LOCKOUT TRIGGERED. DAZZLER DISABLED.\r\n");
        laser_sys.is_locked_out = true;
        while(1) { tight_loop_contents(); }
    } else if (code == 0x00FF00FF) {
        printf("[-] DAZZLER IDLE.\r\n");
    } else {
        printf("[?] UNKNOWN IR COMMAND: 0x%08X\r\n", code);
    }
}
