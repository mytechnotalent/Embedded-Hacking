/**
 * @file state_machine.c
 * @brief Operation Broken Wing Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "state_machine.h"
#include "pwm_ctrl.h"
#include <stdio.h>

drone_state_t drone = {
    .current_state = STATE_ARMED,
    .current_altitude = 0, // Grounded
};

void __noinline process_payload_state(void) {
    if (drone.current_state == STATE_ARMED && drone.current_altitude > 500) {
        deploy_payload();
    } else {
        lock_payload(drone.current_altitude);
    }
}
