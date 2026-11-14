/**
 * FILE: reset.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-2 Loader Reset Handler.
 *
 * BRIEF:
 * Entry point of the stage-2 loader. Reads the boot reason from the
 * watchdog scratch registers, scrubs SRAM, arms a tamper cookie, verifies
 * the stage-3 application with a CRC-32, decrypts the ghost stage into
 * SRAM, applies a one-shot patch to the ghost entry word, and branches to
 * the application. Any failure takes the tamper path.
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
 * @brief   Stage-2 loader entry point.
 *
 * @details Boot reason -> SRAM scrub -> tamper cookie -> CRC-32 verify ->
 *          ghost decrypt -> ghost patch -> jump to the application.
 *
 * @param   None
 * @retval  None
 */
.global Stage2_Reset
.type Stage2_Reset, %function
Stage2_Reset:
.Stage2_Reset_Init:
  bl    Init_Stack                               // initialize MSP/PSP and limits
  bl    Enable_Coprocessor                       // enable CP0 coprocessor
.Stage2_Reset_BootReason:
  ldr   r0, =WATCHDOG_SCRATCH0                   // load scratch0 address
  ldr   r1, [r0]                                 // read last boot cookie
  ldr   r2, =BOOT_COOKIE_TAMPER                  // load tamper cookie
  cmp   r1, r2                                   // tamper last boot?
  beq   .Stage2_Reset_Tamper                     // yes -> tamper path
.Stage2_Reset_Arm_Cookie:
  ldr   r1, =BOOT_COOKIE_INPROGRESS              // load in-progress cookie
  str   r1, [r0]                                 // arm the cookie
.Stage2_Reset_Scrub:
  ldr   r4, =GHOST_RAM                           // ghost region start
  ldr   r5, =GHOST_END                           // ghost region end
  mov   r6, #0                                   // fill value
.Stage2_Reset_Scrub_Loop:
  cmp   r4, r5                                   // reached the end?
  bhs   .Stage2_Reset_Verify                     // yes -> verify app
  str   r6, [r4], #4                             // zero word, advance
  b     .Stage2_Reset_Scrub_Loop                 // loop
.Stage2_Reset_Verify:
  ldr   r0, =APP_BASE                            // application base
  ldr   r1, =APP_SIZE                            // bytes covered by CRC
  bl    Stage2_CRC32                             // compute CRC-32
  ldr   r2, =MANIFEST_BASE                       // manifest sector
  ldr   r2, [r2]                                 // expected CRC stored in word 0
  cmp   r0, r2                                   // match?
  bne   .Stage2_Reset_Tamper                     // no -> tamper path
.Stage2_Reset_Decrypt:
  ldr   r0, =GHOST_BLOB                          // encrypted ghost in flash
  ldr   r1, =GHOST_RAM                           // destination in SRAM
  ldr   r2, =GHOST_SIZE                          // ghost length
  ldr   r3, =GHOST_KEY                           // keystream seed
  bl    Stage2_Xor                               // decrypt into SRAM
.Stage2_Reset_Patch:
  ldr   r0, =GHOST_RAM                           // ghost entry address
  ldr   r1, [r0]                                 // load first word
  ldr   r2, =GHOST_MAGIC                         // load patch mask
  eor   r1, r1, r2                               // unmask the entry word
  str   r1, [r0]                                 // write patched word
  dsb                                            // data sync barrier
  isb                                            // instruction sync barrier
.Stage2_Reset_Clear_Cookie:
  ldr   r0, =WATCHDOG_SCRATCH0                   // scratch0 address
  ldr   r1, =BOOT_COOKIE_OK                      // clean-boot cookie
  str   r1, [r0]                                 // clear the in-progress cookie
.Stage2_Reset_Jump:
  ldr   r0, =APP_BASE                            // application vector table
  ldr   r1, [r0, #4]                             // load application reset vector
  bx    r1                                       // branch to the application
.Stage2_Reset_Tamper:
  bl    Stage2_Brick                             // poison and halt
  b     .Stage2_Reset_Tamper                     // never returns
.size Stage2_Reset, . - Stage2_Reset

/**
 * @brief   Tamper response.
 *
 * @details Simulated brick. A production image erases the manifest sector
 *          through the flash controller; this build poisons the ghost region
 *          and halts so the device cannot run.
 *
 * @param   None
 * @retval  None
 */
.global Stage2_Brick
.type Stage2_Brick, %function
Stage2_Brick:
.Stage2_Brick_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.Stage2_Brick_Mark:
  ldr   r0, =GHOST_RAM                           // ghost region
  ldr   r1, =BRICK_MARKER                        // brick marker
  str   r1, [r0]                                 // poison the ghost entry
.Stage2_Brick_Halt:
  b     .Stage2_Brick_Halt                       // halt forever
