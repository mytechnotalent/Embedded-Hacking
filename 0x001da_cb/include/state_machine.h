/**
 * @file state_machine.h
 * @brief Operation Broken Wing Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#define STATE_IDLE 0
#define STATE_ARMED 1
typedef struct {
    int current_state;
    int current_altitude;
} drone_state_t;
extern drone_state_t drone;
void process_payload_state(void);
#endif
