/**
 * @file mbedtls_config.h
 * @brief Minimal Mbed TLS configuration for the 0x0026a bootloader and app.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 *
 * The Pico SDK points MBEDTLS_CONFIG_FILE at pico_mbedtls_config.h, which in
 * turn includes this file. Only the AES-128 building blocks needed for CMAC
 * authentication and CBC encryption are enabled, keeping the image small.
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include <limits.h>

#define MBEDTLS_AES_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CMAC_C

#endif
