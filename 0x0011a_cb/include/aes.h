// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// File:    aes.h
// Desc:    Declares AES-128-ECB single-block decryption.
// Created: 2026

#ifndef AES_H
#define AES_H

#include <stdint.h>

/**
 * @brief Decrypt one 16-byte block with AES-128-ECB.
 *
 * @param in 16-byte ciphertext.
 * @param key 16-byte key.
 * @param out 16-byte plaintext output.
 * @return None.
 */
void aes128_ecb_decrypt_block(const uint8_t in[16], const uint8_t key[16], uint8_t out[16]);

#endif // AES_H
