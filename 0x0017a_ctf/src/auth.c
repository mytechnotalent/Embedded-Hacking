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
// GitHub:  https://github.com/mytechnotalent
// File:    auth.c
// Desc:    Implements the hardened Ouroboros authentication engine used as the
//          operator gate in the DEEPLINE Metro practice firmware.
// Created: 2026

#include "auth.h"
#include "demo_artifact.h"
#include "pico/stdlib.h"
#include "argon2.h"
#include "mbedtls/chachapoly.h"
#include <string.h>

// Non-zero once auth_init() has prepared the onboard LED GPIO. auth_execute()
// reports an internal error whenever this flag is not yet set, mirroring the
// reference construction from the encryption-c-rp2350 repository.
static bool g_auth_ready;

/**
 * @brief Clear a byte buffer.
 *
 * Writes zero to each byte in the caller-supplied buffer so derived keys
 * and plaintext are not left resident in memory longer than needed.
 *
 * @param buf Pointer to mutable byte buffer.
 * @param len Number of bytes to clear.
 * @return None.
 */
static void clear_bytes(uint8_t *buf, size_t len)
{
    size_t i;
    for (i = 0u; i < len; ++i) {
        buf[i] = 0u;
    }
}

/**
 * @brief Rotate a 32-bit value left.
 *
 * The HChaCha20 core uses 32-bit modular additions and left rotations in
 * its quarter-round primitive.
 *
 * @param value Input 32-bit word.
 * @param shift Rotation distance in bits.
 * @return uint32_t Rotated result.
 */
static uint32_t rotl32(uint32_t value, uint8_t shift)
{
    return (value << shift) | (value >> (32u - shift));
}

/**
 * @brief Load a 32-bit little-endian word from bytes.
 *
 * Converts four little-endian bytes into the word representation used by
 * the HChaCha20 state machine.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t load32_le(const uint8_t *src)
{
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Store a 32-bit word in little-endian byte order.
 *
 * Serializes one HChaCha20 state word into the caller-supplied output
 * buffer.
 *
 * @param dst Pointer to four writable bytes.
 * @param value 32-bit word to serialize.
 * @return None.
 */
static void store32_le(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((value >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((value >> 24u) & 0xFFu);
}

/**
 * @brief Execute one ChaCha quarter-round.
 *
 * Mutates four state words in place according to the standard ChaCha20
 * ARX quarter-round used by the HChaCha20 subkey derivation.
 *
 * @param a Pointer to state word a.
 * @param b Pointer to state word b.
 * @param c Pointer to state word c.
 * @param d Pointer to state word d.
 * @return None.
 */
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    *a += *b; *d ^= *a; *d = rotl32(*d, 16u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 12u);
    *a += *b; *d ^= *a; *d = rotl32(*d, 8u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 7u);
}

/**
 * @brief Derive a 256-bit XChaCha20 subkey from key and nonce prefix.
 *
 * Runs the HChaCha20 core over the first 16 bytes of the 24-byte XChaCha
 * nonce and emits the derived 32-byte subkey.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to 24-byte XChaCha20 nonce.
 * @param subkey Output 32-byte subkey buffer.
 * @return None.
 */
static void hchacha20(const uint8_t key[32], const uint8_t nonce[24], uint8_t subkey[32])
{
    uint32_t state[16] = {
        0x61707865u, 0x3320646Eu, 0x79622D32u, 0x6B206574u,
        load32_le(&key[0]), load32_le(&key[4]), load32_le(&key[8]), load32_le(&key[12]),
        load32_le(&key[16]), load32_le(&key[20]), load32_le(&key[24]), load32_le(&key[28]),
        load32_le(&nonce[0]), load32_le(&nonce[4]), load32_le(&nonce[8]), load32_le(&nonce[12]),
    };
    uint8_t round;
    for (round = 0u; round < 10u; ++round) {
        quarter_round(&state[0], &state[4], &state[8], &state[12]);
        quarter_round(&state[1], &state[5], &state[9], &state[13]);
        quarter_round(&state[2], &state[6], &state[10], &state[14]);
        quarter_round(&state[3], &state[7], &state[11], &state[15]);
        quarter_round(&state[0], &state[5], &state[10], &state[15]);
        quarter_round(&state[1], &state[6], &state[11], &state[12]);
        quarter_round(&state[2], &state[7], &state[8], &state[13]);
        quarter_round(&state[3], &state[4], &state[9], &state[14]);
    }
    store32_le(&subkey[0], state[0]);
    store32_le(&subkey[4], state[1]);
    store32_le(&subkey[8], state[2]);
    store32_le(&subkey[12], state[3]);
    store32_le(&subkey[16], state[12]);
    store32_le(&subkey[20], state[13]);
    store32_le(&subkey[24], state[14]);
    store32_le(&subkey[28], state[15]);
}

/**
 * @brief Build the inner 96-bit nonce used by ChaCha20-Poly1305.
 *
 * XChaCha20 converts the last 8 bytes of the 24-byte outer nonce into the
 * final 12-byte IETF ChaCha nonce by prefixing four zero bytes.
 *
 * @param nonce Pointer to 24-byte XChaCha20 nonce.
 * @param out Output 12-byte nonce buffer.
 * @return None.
 */
static void build_inner_nonce(const uint8_t nonce[24], uint8_t out[12])
{
    memset(out, 0, 4u);
    memcpy(&out[4], &nonce[16], 8u);
}

/**
 * @brief Return true when a byte is ASCII whitespace used by the CLI.
 *
 * The firmware normalizes spaces, carriage returns, tabs, and newlines in
 * the same broad spirit as split-whitespace host parsing.
 *
 * @param ch Input byte.
 * @return bool true when byte is treated as whitespace.
 */
static bool is_space(uint8_t ch)
{
    return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
}

/**
 * @brief Return true when a byte is lowercase ASCII.
 *
 * Hardened passphrases accept only lowercase a-z characters in each word.
 *
 * @param ch Input byte.
 * @return bool true when byte is in the lowercase ASCII range.
 */
static bool is_lowercase_ascii(uint8_t ch)
{
    return (ch >= 'a') && (ch <= 'z');
}

/**
 * @brief Validate the strict hardened passphrase policy.
 *
 * Accepts only passphrases containing exactly 12 lowercase ASCII words
 * separated by whitespace.
 *
 * @param passphrase Pointer to passphrase bytes.
 * @param passphrase_len Number of passphrase bytes.
 * @return bool true when the passphrase satisfies the policy.
 */
static bool validate_hardened_passphrase(const uint8_t *passphrase, size_t passphrase_len)
{
    size_t i = 0u;
    uint8_t words = 0u;
    if ((passphrase == NULL) || (passphrase_len == 0u) || (passphrase_len > AUTH_PASSPHRASE_MAX_LEN)) {
        return false;
    }
    while (i < passphrase_len) {
        while ((i < passphrase_len) && is_space(passphrase[i])) {
            ++i;
        }
        if (i == passphrase_len) {
            break;
        }
        ++words;
        while ((i < passphrase_len) && !is_space(passphrase[i])) {
            if (!is_lowercase_ascii(passphrase[i])) {
                return false;
            }
            ++i;
        }
    }
    return words == AUTH_REQUIRED_WORDS;
}

/**
 * @brief Derive the 32-byte hardened key with Argon2id.
 *
 * Uses the generated artifact parameters and salt to derive the AEAD key
 * that protects the embedded ciphertext.
 *
 * @param passphrase Pointer to passphrase bytes.
 * @param passphrase_len Number of passphrase bytes.
 * @param key_out Output 32-byte key buffer.
 * @return bool true when derivation succeeds.
 */
static bool derive_hardened_key(const uint8_t *passphrase, size_t passphrase_len, uint8_t key_out[32])
{
    return argon2id_hash_raw(
        DEMO_ITERATIONS,
        DEMO_MEMORY_KIB,
        DEMO_PARALLELISM,
        passphrase,
        passphrase_len,
        DEMO_SALT,
        AUTH_SALT_SIZE,
        key_out,
        AUTH_KEY_SIZE) == ARGON2_OK;
}

/**
 * @brief Decrypt the embedded artifact with XChaCha20-Poly1305.
 *
 * Derives the XChaCha20 subkey with HChaCha20, converts the outer nonce to
 * the inner 96-bit nonce, and verifies/decrypts the payload in one shot.
 *
 * @param key Pointer to 32-byte Argon2id-derived key.
 * @param payload_out Output 48-byte plaintext payload buffer.
 * @return bool true when tag verification and decryption succeed.
 */
static bool decrypt_artifact(const uint8_t key[32], uint8_t payload_out[AUTH_PAYLOAD_SIZE])
{
    bool ok;
    int rc;
    uint8_t subkey[32];
    uint8_t inner_nonce[12];
    mbedtls_chachapoly_context ctx;
    hchacha20(key, DEMO_NONCE, subkey);
    build_inner_nonce(DEMO_NONCE, inner_nonce);
    mbedtls_chachapoly_init(&ctx);
    rc = mbedtls_chachapoly_setkey(&ctx, subkey);
    if (rc == 0) {
        rc = mbedtls_chachapoly_auth_decrypt(
            &ctx,
            AUTH_PAYLOAD_SIZE,
            inner_nonce,
            NULL,
            0u,
            &DEMO_CIPHERTEXT_AND_TAG[AUTH_PAYLOAD_SIZE],
            DEMO_CIPHERTEXT_AND_TAG,
            payload_out);
    }
    mbedtls_chachapoly_free(&ctx);
    clear_bytes(subkey, sizeof(subkey));
    clear_bytes(inner_nonce, sizeof(inner_nonce));
    ok = (rc == 0);
    if (!ok) {
        clear_bytes(payload_out, AUTH_PAYLOAD_SIZE);
    }
    return ok;
}

/**
 * @brief Dispatch the decrypted payload to GPIO25 and UART.
 *
 * Mirrors the Rust demo payload contract: byte 0 controls the LED, and
 * bytes 1..7 are transmitted verbatim over UART.
 *
 * @param payload Pointer to decrypted 48-byte payload.
 * @return None.
 */
static void dispatch_payload(const uint8_t payload[AUTH_PAYLOAD_SIZE])
{
    uint8_t i;
    gpio_put(AUTH_LED_PIN, payload[0] ? 1 : 0);
    for (i = 1u; i < 8u; ++i) {
        putchar_raw((char)payload[i]);
    }
}

/**
 * @brief Initialize hardened authentication runtime state.
 *
 * Marks the module as ready, configures the LED GPIO pin as output, and
 * forces the LED low before any authentication attempts.
 *
 * @param None.
 * @return bool true when initialization completed.
 */
bool auth_init(void)
{
    g_auth_ready = true;
    gpio_init(AUTH_LED_PIN);
    gpio_set_dir(AUTH_LED_PIN, GPIO_OUT);
    gpio_put(AUTH_LED_PIN, 0);
    return true;
}

/**
 * @brief Execute one hardened authentication attempt.
 *
 * Validates passphrase policy, derives the Argon2id key, performs
 * authenticated decryption, dispatches GPIO/UART payload bytes on success,
 * and clears transient sensitive buffers.
 *
 * @param passphrase Pointer to passphrase bytes.
 * @param passphrase_len Number of passphrase bytes.
 * @return auth_result_t Detailed outcome for caller-side handling.
 */
auth_result_t auth_execute(const uint8_t *passphrase, size_t passphrase_len)
{
    uint8_t key[AUTH_KEY_SIZE];
    uint8_t payload[AUTH_PAYLOAD_SIZE];
    if (!g_auth_ready) {
        return AUTH_RESULT_INTERNAL_ERROR;
    }
    if (!validate_hardened_passphrase(passphrase, passphrase_len)) {
        return AUTH_RESULT_POLICY_VIOLATION;
    }
    if (!derive_hardened_key(passphrase, passphrase_len, key)) {
        return AUTH_RESULT_INTERNAL_ERROR;
    }
    if (!decrypt_artifact(key, payload)) {
        clear_bytes(key, sizeof(key));
        return AUTH_RESULT_AUTHENTICATION_FAILED;
    }
    dispatch_payload(payload);
    clear_bytes(payload, sizeof(payload));
    clear_bytes(key, sizeof(key));
    return AUTH_RESULT_SUCCESS;
}