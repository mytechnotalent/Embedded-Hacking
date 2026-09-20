// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/encryption-c-rp2350
// File:    auth.h
// Desc:    Declares the Ouroboros authentication engine API for RP2350 firmware.
// Created: 2026

#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Onboard LED GPIO pin number.
 *
 * The RP2350 Pico 2 onboard LED is connected to GPIO 25. Driven high
 * on successful authentication and low on failure or idle.
 */
#define AUTH_LED_PIN 25u

/**
 * @brief Maximum accepted terminal passphrase length in bytes.
 *
 * The CLI accepts interactive human-entered passphrases up to 512 bytes,
 * matching the hardened host demo boundary before policy validation.
 */
#define AUTH_PASSPHRASE_MAX_LEN 512u

/**
 * @brief Required number of lowercase words in the hardened passphrase.
 *
 * The embedded hardened workflow matches the host-side policy exactly:
 * twelve lowercase ASCII words separated by whitespace.
 */
#define AUTH_REQUIRED_WORDS 12u

/**
 * @brief Hardened Argon2id salt size in bytes.
 *
 * Every demo artifact carries a per-ciphertext random 128-bit salt.
 */
#define AUTH_SALT_SIZE 16u

/**
 * @brief Hardened XChaCha20 nonce size in bytes.
 *
 * XChaCha20-Poly1305 consumes a 192-bit nonce in the outer construction.
 */
#define AUTH_NONCE_SIZE 24u

/**
 * @brief Subkey size in bytes derived from Argon2id.
 *
 * The AEAD key size is 256 bits.
 */
#define AUTH_KEY_SIZE 32u

/**
 * @brief AEAD authentication tag size in bytes.
 *
 * XChaCha20-Poly1305 appends a 128-bit authentication tag.
 */
#define AUTH_TAG_SIZE 16u

/**
 * @brief Plaintext payload size in bytes.
 *
 * The fixed dispatch payload is 48 bytes: LED state, UART bytes,
 * and trailing reserved bytes matching the Rust hardened demo layout.
 */
#define AUTH_PAYLOAD_SIZE 48u

/**
 * @brief Full ciphertext-plus-tag artifact size in bytes.
 *
 * The encrypted payload is 48 bytes followed by a 16-byte tag.
 */
#define AUTH_CIPHERTEXT_SIZE (AUTH_PAYLOAD_SIZE + AUTH_TAG_SIZE)

/**
 * @brief Authentication result codes returned by the hardened engine.
 *
 * These values let the CLI distinguish policy failures from
 * cryptographic authentication failures without guessing.
 */
typedef enum auth_result {
    AUTH_RESULT_SUCCESS = 0,
    AUTH_RESULT_POLICY_VIOLATION = 1,
    AUTH_RESULT_AUTHENTICATION_FAILED = 2,
    AUTH_RESULT_INTERNAL_ERROR = 3,
} auth_result_t;

/**
 * @brief Initialize the Ouroboros authentication module.
 *
 * Configures the onboard LED GPIO and marks the hardened engine as ready
 * for passphrase authentication.
 *
 * @param None.
 * @return bool true when initialization is successful, else false.
 */
bool auth_init(void);

/**
 * @brief Execute the hardened Ouroboros authentication pipeline.
 *
 * Validates the strict 12-word lowercase passphrase policy, derives the
 * 256-bit AEAD key with Argon2id using artifact parameters, decrypts the
 * embedded XChaCha20-Poly1305 ciphertext, and dispatches GPIO25/UART
 * payload bytes on success.
 *
 * @param passphrase Pointer to passphrase bytes.
 * @param passphrase_len Number of passphrase bytes.
 * @return auth_result_t Detailed authentication outcome for the caller.
 */
auth_result_t auth_execute(const uint8_t *passphrase,
                           size_t passphrase_len);

#endif // AUTH_H
