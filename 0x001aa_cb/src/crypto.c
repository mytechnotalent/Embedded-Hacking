/**
 * @file crypto.c
 * @brief Lightweight rolling cipher implementation for Operation Iron Net
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "crypto.h"

static const uint8_t CIPHERTEXT[TOKEN_LEN] = {
    0x1A, 0x07, 0x03, 0x18, 0x06, 0x12, 0x65, 0x64
};

static const uint8_t CIPHER_KEY[4] = {
    0x49, 0x52, 0x4F, 0x4E
};

void decrypt_auth_token(uint8_t *dest) {
    volatile const uint8_t *src = CIPHERTEXT;
    volatile const uint8_t *key = CIPHER_KEY;
    for (size_t i = 0; i < TOKEN_LEN; i++) {
        dest[i] = src[i] ^ (key[i % 4] + (uint8_t)i);
    }
}
