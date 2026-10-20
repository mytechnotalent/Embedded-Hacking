/**
 * @file aes_cmac.h
 * @brief AES-128-CMAC authentication tag helpers backed by Mbed TLS.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef AES_CMAC_H
#define AES_CMAC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/** @brief AES block size in bytes. */
#define AES_BLOCK_SIZE 16U

/**
 * @brief Compute an AES-128-CMAC tag over a memory region.
 * @param key  Pointer to the 16-byte AES-128 key.
 * @param data Pointer to the message bytes, which may live in XIP flash.
 * @param len  Message length in bytes.
 * @param tag  Output buffer that receives the 16-byte CMAC tag.
 * @return int Zero on success, non-zero when the cipher is unavailable.
 */
int aes128_cmac(const uint8_t *key, const uint8_t *data, size_t len,
                uint8_t *tag);

/**
 * @brief Compare two CMAC tags without short-circuiting on the first byte.
 * @param a Pointer to the first tag.
 * @param b Pointer to the second tag.
 * @return bool True when the tags are byte-for-byte identical.
 */
bool cmac_tags_equal(const uint8_t *a, const uint8_t *b);

#endif
