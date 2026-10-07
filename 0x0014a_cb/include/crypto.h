/**
 * @file crypto.h
 * @brief Lightweight rolling cipher interface for Operation Zero Hour ESA
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stddef.h>

#define TOKEN_LEN 8

void decrypt_auth_token(uint8_t *dest);

#endif /* CRYPTO_H */
