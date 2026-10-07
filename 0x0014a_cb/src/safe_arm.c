/**
 * @file safe_arm.c
 * @brief Electronic Safe-and-Arm (ESA) controller implementation
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "safe_arm.h"
#include "crypto.h"
#include "gpio_ctrl.h"
#include "lora.h"
#include <stdio.h>
#include <string.h>

static uint8_t s_disarm_token[TOKEN_LEN + 1];

static void print_banner_armed(void) {
    printf("\r\n+-----------------------------------------------------------------+\r\n");
    printf("|               OPERATION ZERO HOUR // ESA CONTROLLER             |\r\n");
    printf("| STATUS: WARHEAD ARMED      [GP16 RED: ON]   LORA RECEIVER: LIVE |\r\n");
    printf("+-----------------------------------------------------------------+\r\n");
}

static void print_banner_safe(const uint8_t *token) {
    printf("\r\n+-----------------------------------------------------------------+\r\n");
    printf("| WARHEAD DISARMED // SAFE                   [GP17 GREEN: ON]     |\r\n");
    printf("| RECOVERED TOKEN: %-8.8s // FUZE NEUTRALIZED                   |\r\n", token);
    printf("+-----------------------------------------------------------------+\r\n");
}

static void print_banner_trip(void) {
    printf("\r\n+-----------------------------------------------------------------+\r\n");
    printf("| TAMPER DETONATION TRIP                     [GP16 RED: FLASH]    |\r\n");
    printf("| STATUS: COMMAND DETONATION ACTIVATED // EXPLOSIVE TRAIN ENGAGED |\r\n");
    printf("+-----------------------------------------------------------------+\r\n");
}

static void eval_command(const char *cmd, uint8_t *state, uint8_t *faults) {
    if (strncmp(cmd, (const char *)s_disarm_token, TOKEN_LEN) == 0) {
        *state = STATE_WARHEAD_SAFE;
        lora_send("WARHEAD DISARMED // SAFE");
        print_banner_safe(s_disarm_token);
    } else {
        (*faults)++;
    }
}

static void check_tamper(uint8_t faults, uint8_t *state) {
    if (*state == STATE_WARHEAD_ARMED && (faults >= 3 || !is_tamper_wire_intact())) {
        *state = STATE_TAMPER_TRIP;
        print_banner_trip();
    }
}

static void handle_packet(bool has_pkt, const char *pkt, uint8_t *state, uint8_t *faults) {
    if (has_pkt && *state != STATE_WARHEAD_SAFE) {
        eval_command(pkt, state, faults);
        check_tamper(*faults, state);
    }
}

static void update_feedback(uint8_t state, uint32_t tick_idx) {
    if (state == STATE_WARHEAD_SAFE) {
        set_led_safe();
    } else if (state == STATE_TAMPER_TRIP) {
        set_led_tamper_flash((tick_idx % 2) == 0);
    } else {
        set_led_armed();
    }
}

void process_safe_arm_tick(void) {
    static uint8_t warhead_state = STATE_WARHEAD_ARMED;
    static uint8_t tamper_faults = 0;
    static uint32_t loop_ticks = 0;
    char rx_pkt[64] = {0};
    bool has_pkt = lora_poll_packet(rx_pkt, sizeof(rx_pkt));
    handle_packet(has_pkt, rx_pkt, &warhead_state, &tamper_faults);
    check_tamper(tamper_faults, &warhead_state);
    update_feedback(warhead_state, loop_ticks++);
}

void init_safe_arm_controller(void) {
    decrypt_auth_token(s_disarm_token);
    s_disarm_token[TOKEN_LEN] = '\0';
    init_lora();
    print_banner_armed();
}
