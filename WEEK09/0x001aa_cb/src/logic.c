/**
 * @file logic.c
 * @brief Operation Iron Net Firmware
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "logic.h"
#include "dht11.h"
#include "crypto.h"
#include "gpio_ctrl.h"
#include "lora.h"
#include <stdio.h>
#include <string.h>

static volatile const uint8_t s_env_mask = 0xAF;
static volatile const uint8_t s_env_pattern = 0x18;
static uint8_t s_auth_token[TOKEN_LEN + 1];
static uint8_t g_strike_state = STATE_ENV_ARMED;
static uint8_t g_tamper_faults = 0;
static bool g_env_ok = false;
static bool g_authorized = false;

/**
 * @brief Print the armed status banner to the console.
 * @param void No parameters.
 * @return void No return value.
 */
static void print_banner_armed(void) {
    printf("\r\n+-----------------------------------------------------------------+\r\n");
    printf("|               OPERATION IRON NET // SENSOR NODE                 |\r\n");
    printf("| STATUS: ARMED       [GP16 RED: ON]      LORA RECEIVER: LIVE     |\r\n");
    printf("+-----------------------------------------------------------------+\r\n");
}

/**
 * @brief Print the payload-authorized banner to the console.
 * @param void No parameters.
 * @return void No return value.
 */
static void print_banner_authorized(void) {
    printf("\r\n+-----------------------------------------------------------------+\r\n");
    printf("| STRIKE AUTHORIZED // MARKER DEPLOYED       [GP17 GREEN: ON]     |\r\n");
    printf("+-----------------------------------------------------------------+\r\n");
}

/**
 * @brief Print the tamper lockout banner to the console.
 * @param void No parameters.
 * @return void No return value.
 */
static void print_banner_lockout(void) {
    printf("\r\n+-----------------------------------------------------------------+\r\n");
    printf("| NODE LOCKOUT                                [GP16 RED: FLASH]   |\r\n");
    printf("| STATUS: TAMPER INTEGRITY FAULT // REBOOT REQUIRED               |\r\n");
    printf("+-----------------------------------------------------------------+\r\n");
}

/**
 * @brief Copy the low byte of the DHT11 frame into a stored sample.
 * @param reading Pointer to the latest validated sensor reading.
 * @return true when the bitwise environment gate is satisfied.
 */
static bool env_matches(const dht11_data_t *reading) {
    uint8_t calculated = (reading->humidity_int ^ s_env_mask) & 0x3C;
    return calculated == s_env_pattern;
}

/**
 * @brief Print the sensor frame invalid message and increment the fault counter.
 * @param faults Pointer to the tamper fault counter.
 * @return void No return value.
 */
static void log_invalid_frame(uint8_t *faults) {
    printf("[-] SENSOR FRAME INVALID. FAULT LOGGED.\r\n");
    (*faults)++;
}

/**
 * @brief Print the sensor reading data to the console.
 * @param reading Pointer to the validated sensor reading.
 * @return void No return value.
 */
static void log_reading(const dht11_data_t *reading) {
    printf("[*] ENV RH %u.%u%% T %u.%uC\r\n", reading->humidity_int,
           reading->humidity_dec, reading->temp_int, reading->temp_dec);
}

/**
 * @brief Read the sensor and update the environment gate result.
 * @param faults Pointer to the tamper fault counter.
 * @return void No return value.
 */
static void evaluate_sensor(uint8_t *faults) {
    dht11_data_t *reading = dht11_read();
    if (!reading->valid) {
        log_invalid_frame(faults);
        return;
    }
    log_reading(reading);
    if (!g_env_ok) {
        g_env_ok = env_matches(reading);
    }
}

/**
 * @brief Poll the radio and set the authorization flag on a token match.
 * @param faults Pointer to the tamper fault counter.
 * @return void No return value.
 */
static void evaluate_uplink(uint8_t *faults) {
    char pkt[64] = {0};
    if (!lora_poll_packet(pkt, sizeof(pkt))) {
        return;
    }
    if (strncmp(pkt, (const char *)s_auth_token, TOKEN_LEN) == 0) {
        g_authorized = true;
        printf("[+] AUTHORIZATION TOKEN ACCEPTED.\r\n");
    } else {
        printf("[-] INVALID AUTHORIZATION TOKEN.\r\n");
        (*faults)++;
    }
}

/**
 * @brief Trip the node when fault limits or the tamper loop demand it.
 * @param faults Current tamper fault count.
 * @return void No return value.
 */
static void check_trap(uint8_t faults) {
    if (g_strike_state == STATE_ENV_LOCKOUT) {
        return;
    }
    if (faults >= 3 || !is_tamper_wire_intact()) {
        g_strike_state = STATE_ENV_LOCKOUT;
        print_banner_lockout();
    }
}

/**
 * @brief Deploy the payload when the environment and token both pass.
 * @param void No parameters.
 * @return void No return value.
 */
static void try_deploy(void) {
    if (g_strike_state == STATE_ENV_AUTHORIZED) {
        return;
    }
    if (g_env_ok && g_authorized) {
        g_strike_state = STATE_ENV_AUTHORIZED;
        payload_deploy();
        lora_send("STRIKE AUTHORIZED // MARKER DEPLOYED");
        print_banner_authorized();
    } else {
        printf("[-] ENVIRONMENT SUB-OPTIMAL. HOLDING.\r\n");
    }
}

/**
 * @brief Drive LED feedback for the current strike state.
 * @param state Current strike state byte.
 * @param tick_idx Monotonic loop tick index for flash timing.
 * @return void No return value.
 */
static void update_feedback(uint8_t state, uint32_t tick_idx) {
    if (state == STATE_ENV_AUTHORIZED) {
        set_led_authorized();
    } else if (state == STATE_ENV_LOCKOUT) {
        set_led_tamper_flash((tick_idx % 2) == 0);
    } else {
        set_led_armed();
    }
}

void evaluate_environment(void) {
    static uint32_t ticks = 0;
    if (g_strike_state != STATE_ENV_LOCKOUT) {
        evaluate_sensor(&g_tamper_faults);
        evaluate_uplink(&g_tamper_faults);
        check_trap(g_tamper_faults);
        try_deploy();
    }
    update_feedback(g_strike_state, ticks++);
}

void init_strike_controller(void) {
    decrypt_auth_token(s_auth_token);
    s_auth_token[TOKEN_LEN] = '\0';
    init_lora();
    print_banner_armed();
}
