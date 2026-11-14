/**
 * FILE: auth.s
 *
 * DESCRIPTION:
 * PALE HORSE Ghost Stage (runs from SRAM).
 *
 * BRIEF:
 * The authentication stage is linked at 0x20040000, decrypted into SRAM by
 * the stage-2 loader, and executed from RAM so it never appears in a flash
 * dump. It maintains a 32-bit xorshift LFSR, issues a challenge, and checks
 * the response with a rotate/multiply transform.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

/**
 * Uninitialized ghost state (inside the SRAM ghost region).
 */
.section .bss                                    // BSS section
.align 2                                         // align to 4-byte boundary
g_lfsr:
  .space 4                                       // LFSR state
g_challenge:
  .space 4                                       // last challenge issued

/**
 * Initialize the .text section.
 */
.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Ghost entry (must be first in the RAM stage).
 *
 * @details Seeds the LFSR from TIMER0 and clears the challenge word.
 *
 * @param   None
 * @retval  None
 */
.global Ghost_Entry
.type Ghost_Entry, %function
Ghost_Entry:
.Ghost_Entry_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.Ghost_Entry_Seed:
  ldr   r0, =TIMER0_BASE                         // TIMER0 base
  ldr   r1, [r0, #0x28]                          // TIMER0 TIMELR (low word)
  ldr   r2, =0x50414c45                          // XOR with "PALE"
  eor   r1, r1, r2                               // scramble the seed
  cmp   r1, #0                                   // LFSR must be non-zero
  bne   .Ghost_Entry_Store                       // ok -> store
  mov   r1, #1                                   // substitute non-zero
.Ghost_Entry_Store:
  ldr   r0, =g_lfsr                              // LFSR state address
  str   r1, [r0]                                 // store seed
  ldr   r0, =g_challenge                         // challenge address
  mov   r1, #0                                   // clear
  str   r1, [r0]                                 // challenge = 0
.Ghost_Entry_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return to caller

/**
 * @brief   Step the LFSR once and return the new state.
 *
 * @details xorshift32 with shifts 13, 17, 5.
 *
 * @param   None
 * @retval  r0 - new LFSR state
 */
.global Ghost_Step
.type Ghost_Step, %function
Ghost_Step:
.Ghost_Step_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =g_lfsr                              // LFSR state address
  ldr   r1, [r0]                                 // load state
  eor   r2, r1, r1, lsl #13                      // x ^= x << 13
  eor   r2, r2, r2, lsr #17                      // x ^= x >> 17
  eor   r2, r2, r2, lsl #5                       // x ^= x << 5
  str   r2, [r0]                                 // store new state
  mov   r0, r2                                   // return new state
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Issue a challenge.
 *
 * @details Steps the LFSR, stores the value as the current challenge, and
 *          returns it.
 *
 * @param   None
 * @retval  r0 - the challenge
 */
.global Ghost_Challenge
.type Ghost_Challenge, %function
Ghost_Challenge:
.Ghost_Challenge_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  bl    Ghost_Step                               // advance the LFSR
  ldr   r1, =g_challenge                         // challenge address
  str   r0, [r1]                                 // remember the challenge
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return the challenge in r0

/**
 * @brief   Check a response against the current challenge.
 *
 * @details expected = ((challenge ^ GHOST_KEY) ROTL 7) * 0x01000193.
 *
 * @param   r0 - response
 * @retval  r0 - 0 if correct, 1 otherwise
 */
.global Ghost_Check
.type Ghost_Check, %function
Ghost_Check:
.Ghost_Check_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r1, =g_challenge                         // challenge address
  ldr   r1, [r1]                                 // load current challenge
  ldr   r2, =GHOST_KEY                           // load secret
  eor   r1, r1, r2                               // challenge ^ secret
  ror   r1, r1, #25                              // rotate left seven bits
  ldr   r2, =0x01000193                          // odd multiplier (FNV prime)
  mul   r1, r2, r1                               // expected = prime * rotated
  cmp   r0, r1                                   // compare
  bne   .Ghost_Check_Fail                        // mismatch -> fail
  mov   r0, #0                                   // success
  b     .Ghost_Check_Done                        // done
.Ghost_Check_Fail:
  mov   r0, #1                                   // failure
.Ghost_Check_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return
