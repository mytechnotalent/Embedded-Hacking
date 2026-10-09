#include <stdio.h>
#include "pico/stdlib.h"

uint32_t simulated_ir_rx_code = 0x00FF00FF;

static void parse_nec_code(uint32_t code) {
    if (code == 0xFF0055AA) {
        printf("DAZZLER POWER LEVEL: MAX\r\n");
    } else if (code == 0xFF00AA55) {
        printf("DAZZLER TRACKING: ENGAGED\r\n");
    } else if (code == 0xDEADBEEF) {
        printf("FRIENDLY FIRE LOCKOUT TRIGGERED. DAZZLER DISABLED.\r\n");
        while(1); // Permanent lockout
    } else {
        printf("UNKNOWN IR COMMAND.\r\n");
    }
}

int main(void) {
    stdio_init_all();
    while (true) {
        parse_nec_code(simulated_ir_rx_code);
        sleep_ms(1000);
    }
}
