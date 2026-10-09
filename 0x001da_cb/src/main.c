#include <stdio.h>
#include "pico/stdlib.h"

#define STATE_IDLE 0
#define STATE_ARMED 1

int current_state = STATE_ARMED;
int current_altitude = 0; // On the ground

static void process_payload_state() {
    if (current_state == STATE_ARMED && current_altitude > 500) {
        printf("PWM ACTIVE: PAYLOAD DEPLOYED.\r\n");
        // Trigger SG90 Servo PWM here
    } else {
        printf("PWM LOCKED: SAFETY ENGAGED.\r\n");
    }
}

int main(void) {
    stdio_init_all();
    while (true) {
        process_payload_state();
        sleep_ms(1000);
    }
}
