/**
 * @file secret.h
 * @brief AES-128-CBC protected runtime banner for the application.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef SECRET_H
#define SECRET_H

#include <stdint.h>

/** @brief AES block size in bytes used by the runtime banner. */
#define APP_AES_BLOCK 16U

/** @brief Length in bytes of the encrypted runtime banner. */
#define APP_BANNER_LEN 32U

/**
 * @brief Decrypt the embedded AES-128-CBC banner and print it.
 * @param void No parameters.
 * @return int Zero on success, non-zero when decryption fails.
 */
int secret_print_banner(void);

#endif
