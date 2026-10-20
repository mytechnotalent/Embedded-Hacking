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
// File:    main.c
// Desc:    DEEPLINE Metro interlocking host for the FINAL-PRACTICE exercise.
//          Chains the strict Ouroboros operator gate (Argon2id plus
//          XChaCha20-Poly1305) around the frozen relay telemetry puzzles.
// Created: 2026

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "pico/stdlib.h"

// Block deviation classification ceiling below which an automated train
// release is permitted. BUG: PALLAS compiled in 95; the engineering limit
// is 60 (two redundant cmp sites in the ship image).
#define SAFE_THRESHOLD 95u

// Number of 32-bit ARX state words used by the signal-key machinery.
#define ARX_WORDS 4u

// Spec value of the derived signal key minted in the incident report. This is
// the key the honest build derives from GATE_SEED_GOOD and the derived IV.
#define SIGNAL_SPEC 0x2D879291u

// ChaCha expand word used by the console-side derivation path.
#define GATE_SEED_GOOD 0x6B206574u

// Track-circuit current deviation frozen by the dead pilot wire (amperes).
// A live reading is recalculated in the field; this image holds a wrong
// snapshot that engineers must decode from registers and SRAM.
static volatile uint32_t g_block_current = 87u;

// Operator-facing block classification (drives the BLOCK STATE line).
static volatile uint32_t g_operator_state = 0u;

// Automatic train release decision (drives the AUTO TRAIN line).
static volatile uint32_t g_dispatch_state = 0u;

// Static per-cycle poll counter retained in .bss. Watch this from GDB.
static volatile uint32_t g_fault_polls = 0u;

// ARX seed substituted by the poisoned build up to the signal derivation.
// BUG: 0x0A0A0A0A was fused in place of the "te k" expand word 0x6B206574.
static volatile uint32_t g_auth_seed = 0x0A0A0A0Au;

// Derived signal key printed each cycle and checked against SIGNAL_SPEC.
static uint32_t g_signal_key = 0u;

/**
 * @brief Hold one track-block telemetry record for the interlocking.
 *
 * Stores the measured block length, the condition flag word, and the
 * crossing identifier used by the automatic train protection logic.
 */
typedef struct telemetry_t {
    double block_length_km;
    uint32_t block_flags;
    uint16_t crossing;
} telemetry_t;

// Live telemetry record. BUG: block_length_km shipped as 3.2 km; the real
// BRIDGE-4 block is 0.32 km, far below the minimum release spacing.
static volatile telemetry_t g_telemetry = { 3.2, 0x3u, 7u };

// Interactive passphrase buffer and parser cursor for the operator gate.
static char g_linebuf[AUTH_PASSPHRASE_MAX_LEN];
static size_t g_lineidx = 0u;

/**
 * @brief Rotate a 32-bit value left.
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
 * @brief Apply one additive-rotate-xor phase of a ChaCha quarter-round.
 *
 * @param a Pointer to state word A.
 * @param b Pointer to state word B.
 * @param c Pointer to state word C.
 * @param d Pointer to state word D.
 * @param shift Rotation distance in bits.
 * @return None.
 */
static void qr_phase(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d, uint8_t shift)
{
    *a += *b;
    *d ^= *a;
    *d = rotl32(*d, shift);
    *c += *d;
    *b ^= *c;
    *b = rotl32(*b, shift);
}

/**
 * @brief Execute one full ChaCha ARX quarter-round.
 *
 * @param a Pointer to state word A.
 * @param b Pointer to state word B.
 * @param c Pointer to state word C.
 * @param d Pointer to state word D.
 * @return None.
 */
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    qr_phase(a, b, c, d, 16u);
    qr_phase(a, b, c, d, 12u);
    qr_phase(a, b, c, d, 8u);
    qr_phase(a, b, c, d, 7u);
}

/**
 * @brief Run r full quarter-rounds over a four-word ARX state.
 *
 * @param a Pointer to state word A.
 * @param b Pointer to state word B.
 * @param c Pointer to state word C.
 * @param d Pointer to state word D.
 * @param rounds Number of full quarter-rounds to run.
 * @return None.
 */
static void run_rounds(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d, uint8_t rounds)
{
    uint8_t r;
    for (r = 0u; r < rounds; ++r) {
        quarter_round(a, b, c, d);
    }
}

/**
 * @brief Derive a 32-bit ARX session key from a seed and IV.
 *
 * Runs four ChaCha quarter-rounds over the seed, the IV, and the ChaCha
 * expand constants; returns state word A exclusive-or state word D.
 *
 * @param seed 32-bit seed word.
 * @param iv 32-bit IV word.
 * @return uint32_t Derived session key word.
 */
static uint32_t derive_session_key(uint32_t seed, uint32_t iv)
{
    uint32_t a = seed;
    uint32_t b = iv;
    uint32_t c = 0x61707865u;
    uint32_t d = 0x3320646Eu;
    run_rounds(&a, &b, &c, &d, 4u);
    return a ^ d;
}

/**
 * @brief Derive the runtime IV from the good console seed word.
 *
 * @param None.
 * @return uint32_t Derived IV word (0xC0F89829 in an honest image).
 */
static uint32_t derive_state_iv(void)
{
    return derive_session_key(GATE_SEED_GOOD, 0u);
}

/**
 * @brief Refresh the runtime signal key from the live seed and IV.
 *
 * Captures the freshly derived IV, then derives the final key word from
 * the substituted seed. Break after each derivation to read the register.
 *
 * @param None.
 * @return None.
 */
static void set_signal_state(void)
{
    uint32_t iv = derive_state_iv();
    g_signal_key = derive_session_key(g_auth_seed, iv);
}

/**
 * @brief Classify the frozen current reading against the compiled limit.
 *
 * Compares the frozen track-circuit current reading against SAFE_THRESHOLD
 * and assigns the resulting boolean status to both g_operator_state and
 * g_dispatch_state.
 *
 * @param None.
 * @return None.
 */
static void classify_blocks(void)
{
    g_operator_state = (g_block_current < SAFE_THRESHOLD) ? 1u : 0u;
    g_dispatch_state = (g_block_current < SAFE_THRESHOLD) ? 1u : 0u;
}

/**
 * @brief Print the relay boot identity and unconditional signal line.
 *
 * Emits the DEEPLINE authority banner, adaptive signal window, serial console
 * configuration string, and nominal track status prompt over the console.
 *
 * @param None.
 * @return None.
 */
static void print_identity(void)
{
    printf("DEEPLINE METRO AUTHORITY\r\n");
    printf("ADAPTIVE SIGNAL WINDOW: 38 MINUTES\r\n");
    printf("USB-CDC 115200 8N1 | AUTHORIZED LAB CONSOLE\r\n");
    printf("TRACK: NORMAL\r\n");
}

/**
 * @brief Print the recurring interlocking status report once per cycle.
 *
 * Computes block length in meters, increments the fault poll counter, prints
 * block state, train authorization status, fault poll tally, and signal key
 * validation report, and re-emits the command prompt.
 *
 * @param None.
 * @return None.
 */
static void print_status(void)
{
    uint32_t metres = (uint32_t)(g_telemetry.block_length_km * 1000.0);
    g_fault_polls += 1u;
    printf("BLOCK STATE: %s\r\n", g_operator_state ? "STABLE" : "CRITICAL");
    printf("AUTO TRAIN: %s\r\n", g_dispatch_state ? "AUTHORIZED" : "HELD");
    printf("BLOCK LENGTH: %u M\r\n", metres);
    printf("FAULT POLLS: %u\r\n", g_fault_polls);
    printf("SIGNAL KEY: 0x%08X %s\r\n", g_signal_key,
           (g_signal_key == SIGNAL_SPEC) ? "OK" : "MISMATCH");
    printf("RESPONSE> ");
}

/**
 * @brief Append one received character to the passphrase buffer.
 *
 * Stores printable characters up to the maximum passphrase length boundary
 * and echoes the character back to the console for interactive typing feedback.
 *
 * @param ch Input character value.
 * @return None.
 */
static void append_char(int ch)
{
    if (g_lineidx + 1u >= AUTH_PASSPHRASE_MAX_LEN) {
        return;
    }
    g_linebuf[g_lineidx] = (char)ch;
    g_lineidx += 1u;
    putchar_raw((char)ch);
}

/**
 * @brief Remove one character from the passphrase buffer.
 *
 * Decrements the buffer index and emits a backspace-space-backspace escape
 * sequence to erase the character on the user's terminal.
 *
 * @param None.
 * @return None.
 */
static void drop_char(void)
{
    if (g_lineidx == 0u) {
        return;
    }
    g_lineidx -= 1u;
    printf("\b \b");
}

/**
 * @brief Authenticate the completed passphrase against the Ouroboros gate.
 *
 * Terminates the string buffer, invokes auth_execute, handles policy violation
 * or authentication failure outputs, and resets the line buffer for the next input.
 *
 * @param None.
 * @return None.
 */
static void submit_gate(void)
{
    auth_result_t result;
    putchar_raw('\r');
    putchar_raw('\n');
    g_linebuf[g_lineidx] = '\0';
    result = auth_execute((const uint8_t *)g_linebuf, g_lineidx);
    if (result == AUTH_RESULT_POLICY_VIOLATION) {
        gpio_put(AUTH_LED_PIN, 0);
        printf("Enter exactly 12 lowercase words separated by spaces.\r\n");
    } else if (result == AUTH_RESULT_SUCCESS) {
        printf("AUTHORITY FRAME: VERIFIED\r\n");
    } else {
        gpio_put(AUTH_LED_PIN, 0);
        printf("Authentication failed.\r\n");
    }
    g_lineidx = 0u;
    printf("RESPONSE> ");
}

/**
 * @brief Poll the console for one passphrase input event.
 *
 * Reads a single character from standard input with zero timeout and routes
 * backspace, newline/carriage return, or printable characters to their respective
 * handlers.
 *
 * @param None.
 * @return None.
 */
static void poll_console(void)
{
    int ch = getchar_timeout_us(0);
    while (ch != PICO_ERROR_TIMEOUT) {
        if ((ch == '\b') || (ch == 127)) {
            drop_char();
        } else if ((ch == '\r') || (ch == '\n')) {
            submit_gate();
        } else {
            append_char(ch);
        }
        ch = getchar_timeout_us(0);
    }
}

/**
 * @brief Drive the DEEPLINE relay console and operator gate forever.
 *
 * Initializes standard I/O and the authentication engine, classifies the track
 * blocks, emits the initial system banner, and enters an infinite loop refreshing
 * the signal state, reporting status, and servicing the console every two seconds.
 *
 * @param None.
 * @return int Process exit code (never returns during normal operation).
 */
int main(void)
{
    stdio_init_all();
    auth_init();
    classify_blocks();
    print_identity();
    while (true) {
        set_signal_state();
        print_status();
        poll_console();
        sleep_ms(2000u);
    }
}