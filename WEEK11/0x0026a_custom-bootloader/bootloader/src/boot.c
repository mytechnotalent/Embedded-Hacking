/**
 * @file boot.c
 * @brief Secure boot console and application handoff helpers.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "boot.h"
#include "image.h"

void boot_init(void) {
    stdio_init_all();
    sleep_ms(2000);
    printf("\n=== 0x0026a secure bootloader ===\n");
}

void boot_halt(const char *reason) {
    printf("BOOT FAILED: %s\n", reason);
    while (true) {
        sleep_ms(500);
    }
}

void boot_launch(const fw_manifest_t *manifest) {
    printf("signature valid, entering app at 0x%08lx\n",
           (unsigned long)manifest->app_base);
    image_start(manifest->app_base);
}