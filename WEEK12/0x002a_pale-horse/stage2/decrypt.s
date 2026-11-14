/**
 * FILE: decrypt.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-2 Ghost Decryption.
 *
 * BRIEF:
 * XOR-keystream decryption of the ghost stage blob into SRAM. The keystream
 * advances each byte by rotating the key left five bits and adding 0x9e, so
 * the same routine encrypts and decrypts.
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
 * @brief   XOR-keystream transform of a byte range.
 *
 * @details Byte-for-byte XOR against an advancing keystream. Safe for
 *          in-place operation when source equals destination.
 *
 * @param   r0 - source address
 * @param   r1 - destination address
 * @param   r2 - length in bytes
 * @param   r3 - initial key
 * @retval  None
 */
.global Stage2_Xor
.type Stage2_Xor, %function
Stage2_Xor:
.Stage2_Xor_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.Stage2_Xor_Setup:
  mov   r4, r0                                   // r4 = source
  mov   r5, r1                                   // r5 = destination
  mov   r6, r2                                   // r6 = remaining
  mov   r7, r3                                   // r7 = key
.Stage2_Xor_Loop:
  cmp   r6, #0                                   // done?
  beq   .Stage2_Xor_Done                         // yes -> finish
  ldrb  r0, [r4]                                 // load source byte
  eor   r0, r0, r7                               // XOR with keystream
  strb  r0, [r5]                                 // store destination byte
  ror   r7, r7, #27                              // rotate key left five bits
  ldr   r0, =0x9e                                // keystream step
  add   r7, r7, r0                               // advance key
  add   r4, r4, #1                               // next source
  add   r5, r5, #1                               // next destination
  subs  r6, r6, #1                               // decrement length
  b     .Stage2_Xor_Loop                         // loop
.Stage2_Xor_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return
