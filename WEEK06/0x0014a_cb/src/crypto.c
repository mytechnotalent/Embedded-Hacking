/**
 * @file crypto.c
 * @brief Lightweight rolling cipher implementation for Operation Zero Hour ESA
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "crypto.h"

static const uint8_t CIPHERTEXT[TOKEN_LEN] = {
    0x07, 0x03, 0x03, 0x12, 0x07, 0x7F, 0x70, 0x7A
};

static const uint8_t CIPHER_KEY[4] = {
    0x54, 0x41, 0x43, 0x54
};

void decrypt_auth_token(uint8_t *dest) {
    volatile const uint8_t *src = CIPHERTEXT;
    volatile const uint8_t *key = CIPHER_KEY;
    for (size_t i = 0; i < TOKEN_LEN; i++) {
        dest[i] = src[i] ^ (key[i % 4] + (uint8_t)i);
    }
}
