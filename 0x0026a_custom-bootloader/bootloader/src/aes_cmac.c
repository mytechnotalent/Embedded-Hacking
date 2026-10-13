/**
 * @file aes_cmac.c
 * @brief AES-128-CMAC primitives implemented on top of the SDK Mbed TLS.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "mbedtls/cmac.h"
#include "mbedtls/cipher.h"
#include "aes_cmac.h"

int aes128_cmac(const uint8_t *key, const uint8_t *data, size_t len,
                uint8_t *tag) {
    const mbedtls_cipher_info_t *info;
    info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
    if (info == NULL) return -1;
    return mbedtls_cipher_cmac(info, key, 128, data, len, tag);
}

bool cmac_tags_equal(const uint8_t *a, const uint8_t *b) {
    uint8_t diff = 0;
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }
    return diff == 0;
}
