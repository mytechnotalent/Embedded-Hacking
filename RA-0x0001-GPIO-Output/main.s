/**
 * FILE: main.s
 *
 * DESCRIPTION:
 * RP2350 Bare-Metal GPIO16 Blink, Coprocessor Version.
 * Minimal bare‑metal LED blink on the RP2350 using direct coprocessor
 * (MCRR) instructions to manipulate GPIO control registers. This bypasses
 * SDK abstractions and demonstrates register‑level control in assembler.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: October 5, 2025
 * UPDATE DATE: October 5, 2025
 */

.syntax unified                                       // use unified assembly syntax
.cpu cortex-m33                                       // target Cortex-M33 core
.thumb                                                // use Thumb instruction set

/**
 * Memory addresses and constants.
 */
.equ IO_BANK0_BASE,   0x40028000                      // base address of IO_BANK0
.equ PADS_BANK0_BASE, 0x40038000                      // base address of PADS_BANK0
.equ SIO_BASE,        0xD0000000                      // base address of SIO block
.equ GPIO16_CTRL,     0x84                            // io[16].ctrl offset
.equ GPIO16_PAD,      0x44                            // pads io[16] offset
.equ GPIO16_BIT,      (1<<16)                         // bit mask for GPIO16
.equ GPIO_OUT_SET,    0x18                            // SIO->GPIO_OUT_SET offset
.equ GPIO_OUT_XOR,    0x28                            // SIO->GPIO_OUT_XOR offset
.equ GPIO_OE_SET,     0x38                            // SIO->GPIO_OE_SET offset
.equ STACK_TOP,       0x20082000                      // top of non-secure SRAM
.equ STACK_LIMIT,     0x2007A000                      // stack limit (32 KB below top)

/**
 * Initialize the .vectors section. The .vectors section contains vector
 * table.
 */
.section .vectors, "ax"                               // vector table section
.align 2                                              // align to 4-byte boundary

/**
 * Vector table section.
 */
.global _vectors                                      // export symbol
_vectors:
  .word STACK_TOP                                     // initial stack pointer
  .word Reset_Handler + 1                             // reset handler (Thumb bit set)

/**
 * @brief   Reset handler for RP2350.
 *
 * @details Entry point after reset. Performs:
 *          - Stack initialization
 *          - Coprocessor enable
 *          - GPIO16 pad/function configuration
 *          - Branches to main() which contains the blink loop
 *
 * @param   None
 * @retval  None
 */
.global Reset_Handler                                 // export Reset_Handler
.type Reset_Handler, %function                        // mark as function
Reset_Handler:
  BL    Init_Stack                                    // initialize MSP/PSP and limits
  BL    Enable_Coprocessor                            // enable CP0 in CPACR for MCRR
  B     main                                          // branch to main loop
.size Reset_Handler, . - Reset_Handler

/**
 * @brief   Initialize stack pointers.
 *
 * @details Sets Main and Process Stack Pointers (MSP/PSP) and their limits.
 *
 * @param   None
 * @retval  None
 */
.type Init_Stack, %function
Init_Stack:
  LDR   R0, =STACK_TOP                                // load stack top
  MSR   PSP, R0                                       // set PSP
  LDR   R0, =STACK_LIMIT                              // load stack limit
  MSR   MSPLIM, R0                                    // set MSP limit
  MSR   PSPLIM, R0                                    // set PSP limit
  LDR   R0, =STACK_TOP                                // reload stack top
  MSR   MSP, R0                                       // set MSP
  BX    LR                                            // return

/**
 * @brief   Enable coprocessor access.
 *
 * @details Grants full access to coprocessor 0 (CP0) via CPACR.
 *
 * @param   None
 * @retval  None
 */
.type Enable_Coprocessor , %function
Enable_Coprocessor:
  LDR   R0, =0xE000ED88                               // CPACR address
  LDR   R1, [R0]                                      // read CPACR
  ORR   R1, R1, #0x3                                  // set CP0 full access
  STR   R1, [R0]                                      // write CPACR
  DSB                                                 // data sync barrier
  ISB                                                 // instruction sync barrier
  BX    LR                                            // return

/**
 * Initialize the .text section. 
 * The .text section contains executable code.
 */
.section .text                                        // code section
.align 2                                              // align to 4-byte boundary

/**
 * @brief   Main application entry point.
 *
 * @details Implements the infinite blink loop:
 *          - Set GPIO16 high
 *          - Delay ~500 ms
 *          - Set GPIO16 low
 *          - Delay ~500 ms
 *          - Repeat forever
 *
 * @param   None
 * @retval  None
 */
.global main                                          // export main
.type main, %function                                 // mark as function
main:
.Push_Registers:
  PUSH  {R4-R12, LR}                                  // push registers R4-R12, LR to the stack
.GPIO16_Config:
  BL    GPIO16_Config                                 // configure pads and FUNCSEL for GPIO16
.Loop:
  BL    GPIO16_Set                                    // set GPIO16 high
  BL    Delay_500ms                                   // ~500 ms delay
  BL    GPIO16_Clear                                  // set GPIO16 low
  BL    Delay_500ms                                   // ~500 ms delay
  B     .Loop                                         // loop forever
.Pop_Registers:
  POP   {R4-R12, LR}                                  // pop registers R4-R12, LR from the stack
  BX    LR                                            // return to caller

/**
 * @brief   Configure GPIO16 for SIO control.
 *
 * @details Sets pad control (IE, OD, ISO) and FUNCSEL = 5 (SIO). Enables OE.
 *
 * @param   None
 * @retval  None
 */
.type GPIO16_Config, %function
GPIO16_Config:
.GPIO16_Config_Push_Registers:
  PUSH  {R4-R12, LR}                                  // push registers R4-R12, LR to the stack
.GPIO16_Config_Modify_Pad:
  LDR   R3, =PADS_BANK0_BASE + GPIO16_PAD             // pad control address
  LDR   R2, [R3]                                      // read pad config
  BIC   R2, R2, #0x80                                 // clear OD
  ORR   R2, R2, #0x40                                 // set IE
  BIC   R2, R2, #0x100                                // clear ISO
  STR   R2, [R3]                                      // write pad config
.GPIO16_Config_Modify_IO:
  LDR   R3, =IO_BANK0_BASE + GPIO16_CTRL              // IO control address
  LDR   R2, [R3]                                      // read IO config
  BIC   R2, R2, #0x1F                                 // clear FUNCSEL
  ORR   R2, R2, #5                                    // set FUNCSEL=5
  STR   R2, [R3]                                      // write IO config
.GPIO16_Config_Enable_OE:
  MOVS  R4, #16                                       // GPIO number
  MOVS  R5, #1                                        // enable output
  MCRR  p0, #4, R4, R5, c4                            // gpioc_bit_oe_put(16, 1)
.GPIO16_Config_Pop_Registers:
  POP   {R4-R12, LR}                                  // pop registers R4-R12, LR from the stack
  BX    LR                                            // return

/**
 * @brief   Set GPIO16 high.
 *
 * @details Drives GPIO16 output = 1 via coprocessor MCRR.
 *
 * @param   None
 * @retval  None
 */
.type GPIO16_Set, %function
GPIO16_Set:
.GPIO16_Set_Push_Registers:
  PUSH  {R4-R12, LR}                                  // push registers R4-R12, LR to the stack
.GPIO16_Set_Load_Operands:
  MOVS  R4, #16                                       // GPIO number
  MOVS  R5, #1                                        // logic high
.GPIO16_Set_Execute:
  MCRR  p0, #4, R4, R5, c0                            // gpioc_bit_out_put(16, 1)
.GPIO16_Set_Pop_Registers:
  POP   {R4-R12, LR}                                  // pop registers R4-R12, LR from the stack
  BX    LR                                            // return to caller

/**
 * @brief   Clear GPIO16 (set low).
 *
 * @details Drives GPIO16 output = 0 via coprocessor MCRR.
 *
 * @param   None
 * @retval  None
 */
.type GPIO16_Clear, %function
GPIO16_Clear:
.GPIO16_Clear_Push_Registers:
  PUSH  {R4-R12, LR}                                  // push registers R4-R12, LR to the stack
.GPIO16_Clear_Load_Operands:
  MOVS  R4, #16                                       // GPIO number
  MOVS  R5, #0                                        // logic low
.GPIO16_Clear_Execute:
  MCRR  p0, #4, R4, R5, c0                            // gpioc_bit_out_put(16, 0)
.GPIO16_Clear_Pop_Registers:
  POP   {R4-R12, LR}                                  // pop registers R4-R12, LR from the stack
  BX    LR                                            // return to caller

/**
 * @brief   Busy‑wait Delay_500ms loop.
 *
 * @details Consumes ~2,000,000 cycles to approximate ~500 ms at boot clock.
 *
 * @param   None
 * @retval  None
 */
.type Delay_500ms, %function
Delay_500ms:
.Delay_500ms_Push_Registers:
  PUSH  {R4-R12, LR}                                  // push registers R4-R12, LR to the stack
.Delay_500ms_Setup:
  LDR   R2, =2000000                                  // loop count (~500 ms)
.Delay_500ms_Loop:
  SUBS  R2, R2, #1                                    // decrement counter
  BNE   .Delay_500ms_Loop                             // branch until zero
.Delay_500ms_Pop_Registers:
  POP   {R4-R12, LR}                                  // pop registers R4-R12, LR from the stack
  BX    LR                                            // return to caller

/**
 * Test data and constants.
 * The .rodata section is used for constants and static data.
 */
.section .rodata                                      // read-only data section

/**
 * Initialized global data.
 * The .data section is used for initialized global or static variables.
 */
.section .data                                        // data section

/**
 * Uninitialized global data.
 * The .bss section is used for uninitialized global or static variables.
 */
.section .bss                                         // BSS section
