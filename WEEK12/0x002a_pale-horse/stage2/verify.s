/**
 * FILE: verify.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-2 Integrity Verification.
 *
 * BRIEF:
 * Bitwise CRC-32 (reflected, polynomial 0xEDB88320) over a byte range.
 * Used by the stage-2 loader to verify the stage-3 application image
 * before it is allowed to run.
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
 * Initialize the .text section.
 * The .text section contains executable code.
 */
.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Compute a CRC-32 over a memory range.
 *
 * @details Bitwise (non table-driven) reflected CRC-32. For each byte the
 *          accumulator is XORed with the byte, then shifted eight times; when
 *          the low bit is set the accumulator is XORed with the reflected
 *          polynomial. The result is bit-inverted before return.
 *
 * @param   r0 - start address
 * @param   r1 - length in bytes
 * @retval  r0 - CRC-32 value
 */
.global Stage2_CRC32
.type Stage2_CRC32, %function
Stage2_CRC32:
.Stage2_CRC32_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.Stage2_CRC32_Setup:
  mov   r2, #-1                                  // crc = 0xffffffff
  mov   r4, r0                                   // r4 = pointer
  add   r5, r0, r1                               // r5 = end pointer
.Stage2_CRC32_Outer:
  cmp   r4, r5                                   // reached the end?
  bhs   .Stage2_CRC32_Done                       // yes -> finish
  ldrb  r3, [r4]                                 // load next byte
  eor   r2, r2, r3                               // crc ^= byte
  ldr   r6, =8                                   // eight bits per byte
.Stage2_CRC32_Bit:
  tst   r2, #1                                   // test low bit
  beq   .Stage2_CRC32_Shift                      // zero -> just shift
  lsr   r2, r2, #1                               // shift right
  ldr   r7, =CRC32_POLY                          // load reflected polynomial
  eor   r2, r2, r7                               // crc ^= poly
  b     .Stage2_CRC32_Next                       // next bit
.Stage2_CRC32_Shift:
  lsr   r2, r2, #1                               // shift right
.Stage2_CRC32_Next:
  subs  r6, r6, #1                               // decrement bit counter
  bne   .Stage2_CRC32_Bit                        // loop over bits
  add   r4, r4, #1                               // advance to next byte
  b     .Stage2_CRC32_Outer                      // loop over bytes
.Stage2_CRC32_Done:
  mvn   r0, r2                                   // final inversion
.Stage2_CRC32_Pop_Registers:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return
