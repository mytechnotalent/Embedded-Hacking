/**
 * @file secret.c
 * @brief Decrypts a runtime banner with the SDK Mbed TLS AES-128-CBC API.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 *
 * -----------------------------------------------------------------------------
 *
 * This module shows how to use the AES implementation that ships with the Pico
 * SDK. A short banner is stored in flash only as AES-128-CBC ciphertext; the
 * key and IV live in the same image. At startup secret_print_banner() rebuilds
 * the plaintext with mbedtls_aes_crypt_cbc() and writes it to the console.
 *
 * The plaintext is exactly APP_BANNER_LEN bytes (two whole AES blocks), so no
 * padding routine is required. The ciphertext was produced on the host with the
 * matching key and IV, for example:
 *
 *   printf '%s' 'SECRET: IR LINK SECURED ONLINE  ' \
 *     | openssl enc -aes-128-cbc -K 49726f6e4e6574426f6f744b65792121 \
 *         -iv 41707053656372657449563132333435 -nopad | xxd -p
 */

#include <stdio.h>
#include <string.h>
#include "mbedtls/aes.h"
#include "secret.h"

/** @brief AES-128 key, the same 16 bytes the bootloader uses for CMAC. */
static const uint8_t banner_key[APP_AES_BLOCK] = {
    0x49, 0x72, 0x6F, 0x6E, 0x4E, 0x65, 0x74, 0x42,
    0x6F, 0x6F, 0x74, 0x4B, 0x65, 0x79, 0x21, 0x21
};

/** @brief Initialization vector used when the banner was encrypted. */
static const uint8_t banner_iv[APP_AES_BLOCK] = {
    0x41, 0x70, 0x70, 0x53, 0x65, 0x63, 0x72, 0x65,
    0x74, 0x49, 0x56, 0x31, 0x32, 0x33, 0x34, 0x35
};

/** @brief AES-128-CBC ciphertext of the runtime banner. */
static const uint8_t banner_ct[APP_BANNER_LEN] = {
    0xBA, 0xEC, 0x4A, 0x19, 0x25, 0x86, 0x1B, 0xB9,
    0x2D, 0xFD, 0x9F, 0x13, 0xF7, 0x9B, 0x3E, 0x80,
    0xCF, 0x12, 0xB6, 0x98, 0xDB, 0x91, 0x65, 0xBE,
    0x8F, 0xD7, 0xC3, 0x47, 0x7B, 0xB2, 0xE7, 0x6E
};

/**
 * @brief Run the AES-128-CBC decrypt in place over a working buffer.
 * @param aes Pointer to an AES context holding an already-expanded key.
 * @param iv  Pointer to a mutable copy of the initialization vector.
 * @param buf Pointer to the buffer holding ciphertext, then plaintext.
 * @return int Zero on success, non-zero on failure.
 */
static int secret_cbc(mbedtls_aes_context *aes, uint8_t *iv, uint8_t *buf) {
    int rc = mbedtls_aes_crypt_cbc(aes, MBEDTLS_AES_DECRYPT, APP_BANNER_LEN,
                                   iv, buf, buf);
    return rc;
}

/**
 * @brief Decrypt the banner ciphertext into a working buffer.
 * @param buf Buffer of at least APP_BANNER_LEN bytes.
 * @return int Zero on success, non-zero on failure.
 */
static int secret_decrypt(uint8_t *buf) {
    mbedtls_aes_context aes;
    uint8_t iv[APP_AES_BLOCK];
    memcpy(iv, banner_iv, APP_AES_BLOCK);
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, banner_key, 128);
    int rc = secret_cbc(&aes, iv, buf);
    mbedtls_aes_free(&aes);
    return rc;
}

int secret_print_banner(void) {
    uint8_t buf[APP_BANNER_LEN + 1];
    memcpy(buf, banner_ct, APP_BANNER_LEN);
    buf[APP_BANNER_LEN] = '\0';
    int rc = secret_decrypt(buf);
    if (rc != 0) return rc;
    printf("app banner: %s\n", (char *)buf);
    return 0;
}
