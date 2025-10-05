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

.syntax unified                             // use unified assembly syntax
.cpu cortex-m33                             // target Cortex-M33 core
.thumb                                      // use Thumb instruction set

/**
 * Memory addresses and constants.
 */
.equ IO_BANK0_BASE,   0x40028000            // base address of IO_BANK0
.equ PADS_BANK0_BASE, 0x40038000            // base address of PADS_BANK0
.equ SIO_BASE,        0xD0000000            // base address of SIO block

.equ GPIO16_CTRL,     0x84                  // io[16].ctrl offset
.equ GPIO16_PAD,      0x44                  // pads io[16] offset
.equ GPIO16_BIT,      (1 << 16)             // bit mask for GPIO16

.equ GPIO_OUT_SET,    0x18                  // SIO->GPIO_OUT_SET offset
.equ GPIO_OUT_XOR,    0x28                  // SIO->GPIO_OUT_XOR offset
.equ GPIO_OE_SET,     0x38                  // SIO->GPIO_OE_SET offset

.equ STACK_TOP,       0x20082000            // top of non-secure SRAM
.equ STACK_LIMIT,     0x2007A000            // stack limit (32 KB below top)

/**
 * Vector table section.
 */
.section .vectors, "ax"                     // vector table section
.align 2                                    // align to 4 bytes
.global _vectors                            // export symbol
_vectors:
  .word STACK_TOP                           // initial stack pointer
  .word Reset_Handler + 1                   // reset handler (Thumb bit set)

/**
 * @brief   Reset handler for RP2350.
 * @details Entry point after reset. Performs:
 *          - Stack initialization
 *          - Coprocessor enable
 *          - GPIO16 pad/function configuration
 *          - Infinite blink loop using coprocessor writes
 * @param   None
 * @retval  None
 */
.section .text                              // code section
.align 2                                    // align functions
.global Reset_Handler                       // export Reset_Handler
.type Reset_Handler, %function              // mark as function

Reset_Handler:
  BL    init_stack                          // initialize MSP/PSP and limits
  BL    enable_coprocessor                  // enable CP0 in CPACR for MCRR
  BL    gpio16_config                       // configure pads and FUNCSEL for GPIO16
blink_loop:
  BL    gpio16_set                          // set GPIO16 high
  BL    delay                               // delay ~500 ms
  BL    gpio16_clear                        // set GPIO16 low
  BL    delay                               // delay ~500 ms
  B     blink_loop                          // loop forever

.size Reset_Handler, . - Reset_Handler

/**
 * @brief   Initialize stack pointers.
 * @details Sets Main and Process Stack Pointers (MSP/PSP) and their limits.
 * @param   None
 * @retval  None
 */
.type init_stack, %function
init_stack:
  LDR   R0, =STACK_TOP                      // load stack top
  MSR   PSP, R0                             // set PSP
  LDR   R0, =STACK_LIMIT                    // load stack limit
  MSR   MSPLIM, R0                          // set MSP limit
  MSR   PSPLIM, R0                          // set PSP limit
  LDR   R0, =STACK_TOP                      // reload stack top
  MSR   MSP, R0                             // set MSP
  BX    LR                                  // return

/**
 * @brief   Enable coprocessor access.
 * @details Grants full access to coprocessor 0 (CP0) via CPACR.
 * @param   None
 * @retval  None
 */
.type enable_coprocessor, %function
enable_coprocessor:
  LDR   R0, =0xE000ED88                     // CPACR address
  LDR   R1, [R0]                            // read CPACR
  ORR   R1, R1, #0x3                        // set CP0 full access
  STR   R1, [R0]                            // write CPACR
  DSB                                       // data sync barrier
  ISB                                       // instruction sync barrier
  BX    LR                                  // return

/**
 * @brief   Configure GPIO16 for SIO control.
 * @details Sets pad control (IE, OD, ISO) and FUNCSEL = 5 (SIO). Enables OE.
 * @param   None
 * @retval  None
 */
.type gpio16_config, %function
gpio16_config:
  LDR   R3, =PADS_BANK0_BASE + GPIO16_PAD   // pad control address
  LDR   R2, [R3]                            // read pad config
  BIC   R2, R2, #0x80                       // clear OD
  ORR   R2, R2, #0x40                       // set IE
  BIC   R2, R2, #0x100                      // clear ISO
  STR   R2, [R3]                            // write pad config
  LDR   R3, =IO_BANK0_BASE + GPIO16_CTRL    // IO control address
  LDR   R2, [R3]                            // read IO config
  BIC   R2, R2, #0x1F                       // clear FUNCSEL
  ORR   R2, R2, #5                          // set FUNCSEL = 5
  STR   R2, [R3]                            // write IO config
  MOVS  R4, #16                             // GPIO number
  MOVS  R5, #1                              // enable output
  MCRR  p0, #4, R4, R5, c4                  // gpioc_bit_oe_put(16, 1)
  BX    LR                                  // return

/**
 * @brief   Set GPIO16 high.
 * @details Drives GPIO16 output = 1 via coprocessor MCRR.
 * @param   None
 * @retval  None
 */
.type gpio16_set, %function
gpio16_set:
  MOVS  R4, #16                             // GPIO number
  MOVS  R5, #1                              // logic high
  MCRR  p0, #4, R4, R5, c0                  // gpioc_bit_out_put(16, 1)
  BX    LR                                  // return

/**
 * @brief   Clear GPIO16 (set low).
 * @details Drives GPIO16 output = 0 via coprocessor MCRR.
 * @param   None
 * @retval  None
 */
.type gpio16_clear, %function
gpio16_clear:
  MOVS  R4, #16                             // GPIO number
  MOVS  R5, #0                              // logic low
  MCRR  p0, #4, R4, R5, c0                  // gpioc_bit_out_put(16, 0)
  BX    LR                                  // return

/**
 * @brief   Busy‑wait delay loop.
 * @details Consumes ~2,000,000 cycles to approximate ~500 ms at boot clock.
 * @param   None
 * @retval  None
 */
.type delay, %function
delay:
  LDR   R2, =2000000                        // loop count
1:SUBS  R2, R2, #1                          // decrement
  BNE   1b                                  // loop until zero
  BX    LR                                  // return
