<img src="https://github.com/mytechnotalent/RP2350_Blink_Driver/blob/main/RP2350_Blink_Driver.png?raw=true">

## FREE Reverse Engineering Self-Study Course [HERE](https://github.com/mytechnotalent/Reverse-Engineering-Tutorial)

<br>

# RP2350 Blink Driver
An RP2350 blink driver written entirely in Assembler.

# Code
```assembler
/**
 * FILE: main.s
 *
 * DESCRIPTION:
 * RP2350 Bare-Metal GPIO16 Blink, Coprocessor Version.
 * 
 * BRIEF:
 * Minimal bare‑metal LED blink on the RP2350 using the direct coprocessor
 * (MCRR) instructions to manipulate GPIO control registers. This bypasses
 * SDK abstractions and demonstrates register‑level control in assembler.
 * Clocks the external crystal oscillator (XOSC) at 14.5MHz.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: October 24, 2025
 * UPDATE DATE: October 25, 2025
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

/**
 * Memory addresses and constants.
 */
.equ STACK_TOP,                   0x20082000               
.equ STACK_LIMIT,                 0x2007A000             
.equ XOSC_BASE,                   0x40048000          
.equ XOSC_CTRL,                   XOSC_BASE + 0x00       
.equ XOSC_STATUS,                 XOSC_BASE + 0x04       
.equ XOSC_STARTUP,                XOSC_BASE + 0x0C        
.equ PPB_BASE,                    0xE0000000               
.equ CPACR,                       PPB_BASE + 0x0ED88       
.equ CLOCKS_BASE,                 0x40010000              
.equ CLK_PERI_CTRL,               CLOCKS_BASE + 0x48       
.equ RESETS_BASE,                 0x40020000               
.equ RESETS_RESET,                RESETS_BASE + 0x0        
.equ RESETS_RESET_CLEAR,          RESETS_BASE + 0x3000     
.equ RESETS_RESET_DONE,           RESETS_BASE + 0x8        
.equ IO_BANK0_BASE,               0x40028000               
.equ IO_BANK0_GPIO16_CTRL_OFFSET, 0x84                   
.equ PADS_BANK0_BASE,             0x40038000               
.equ PADS_BANK0_GPIO16_OFFSET,    0x44                    

/**
 * Initialize the .vectors section. The .vectors section contains vector
 * table and Reset_Handler.
 */
.section .vectors, "ax"                          // vector table section
.align 2                                         // align to 4-byte boundary

/**
 * Vector table section.
 */
.global _vectors                                 // export _vectors symbol
_vectors:
  .word STACK_TOP                                // initial stack pointer
  .word Reset_Handler + 1                        // reset handler (Thumb bit set)

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
.global Reset_Handler                            // export Reset_Handler symbol
.type Reset_Handler, %function                        
Reset_Handler:
  BL    Init_Stack                               // initialize MSP/PSP and limits
  BL    Init_XOSC                                // initialize external crystal oscillator
  BL    Enable_XOSC_Peri_Clock                   // enable XOSC peripheral clock
  BL    Init_Subsystem                           // initialize subsystems
  BL    Enable_Coprocessor                       // enable CP0 coprocessor
  B     main                                     // branch to main loop
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
  LDR   R0, =STACK_TOP                           // load stack top
  MSR   PSP, R0                                  // set PSP
  LDR   R0, =STACK_LIMIT                         // load stack limit
  MSR   MSPLIM, R0                               // set MSP limit
  MSR   PSPLIM, R0                               // set PSP limit
  LDR   R0, =STACK_TOP                           // reload stack top
  MSR   MSP, R0                                  // set MSP
  BX    LR                                       // return

/**
 * @brief   Init XOSC and wait until it is ready.
 *
 * @details Configures and initializes the external crystal oscillator (XOSC).
 *          Waits for the XOSC to become stable before returning.
 *
 * @param   None
 * @retval  None
 */
.type Init_XOSC, %function
Init_XOSC:
  LDR   R0, =XOSC_STARTUP                        // load XOSC_STARTUP address
  LDR   R1, =0x00C4                              // set delay 50,000 cycles
  STR   R1, [R0]                                 // store value into XOSC_STARTUP
  LDR   R0, =XOSC_CTRL                           // load XOSC_CTRL address
  LDR   R1, =0x00FABAA0                          // set 1_15MHz, freq range, actual 14.5MHz
  STR   R1, [R0]                                 // store value into XOSC_CTRL
.Init_XOSC_Wait:
  LDR   R0, =XOSC_STATUS                         // load XOSC_STATUS address
  LDR   R1, [R0]                                 // read XOSC_STATUS value
  TST   R1, #(1<<31)                             // test STABLE bit
  BEQ   .Init_XOSC_Wait                          // wait until stable bit is set
  BX    LR                                       // return

/**
 * @brief   Enable XOSC peripheral clock.
 *
 * @details Sets the peripheral clock to use XOSC as its AUXSRC.
 *
 * @param   None
 * @retval  None
 */
.type Enable_XOSC_Peri_Clock, %function
Enable_XOSC_Peri_Clock:
  LDR   R0, =CLK_PERI_CTRL                       // load CLK_PERI_CTRL address
  LDR   R1, [R0]                                 // read CLK_PERI_CTRL value
  ORR   R1, R1, #(1<<11)                         // set ENABLE bit
  ORR   R1, R1, #(4<<5)                          // set AUXSRC: XOSC_CLKSRC bit
  STR   R1, [R0]                                 // store value into CLK_PERI_CTRL
  BX    LR                                       // return

/**
 * @brief   Init subsystem.
 *
 * @details Initiates the various subsystems by clearing their reset bits.
 *
 * @param   None
 * @retval  None
 */
.type Init_Subsystem, %function
Init_Subsystem:
.GPIO_Subsystem_Reset:
  LDR   R0, =RESETS_RESET                        // load RESETS->RESET address
  LDR   R1, [R0]                                 // read RESETS->RESET value
  BIC   R1, R1, #(1<<6)                          // clear IO_BANK0 bit
  STR   R1, [R0]                                 // store value into RESETS->RESET address
.GPIO_Subsystem_Reset_Wait:
  LDR   R0, =RESETS_RESET_DONE                   // load RESETS->RESET_DONE address
  LDR   R1, [R0]                                 // read RESETS->RESET_DONE value
  TST   R1, #(1<<6)                              // test IO_BANK0 reset done
  BEQ   .GPIO_Subsystem_Reset_Wait               // wait until done
  BX    LR                                       // return

/**
 * @brief   Enable coprocessor access.
 *
 * @details Grants full access to coprocessor 0 via CPACR.
 *
 * @param   None
 * @retval  None
 */
.type Enable_Coprocessor , %function
Enable_Coprocessor:
  LDR   R0, =CPACR                               // load CPACR address
  LDR   R1, [R0]                                 // read CPACR value
  ORR   R1, R1, #(1<<1)                          // set CP0: Ctrl access priv coproc 0 bit
  ORR   R1, R1, #(1<<0)                          // set CP0: Ctrl access priv coproc 0 bit
  STR   R1, [R0]                                 // store value into CPACR
  DSB                                            // data sync barrier
  ISB                                            // instruction sync barrier
  BX    LR                                       // return

/**
 * Initialize the .text section. 
 * The .text section contains executable code.
 */
.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Main application entry point.
 *
 * @details Implements the infinite blink loop.
 *
 * @param   None
 * @retval  None
 */
.global main                                     // export main
.type main, %function                            // mark as function
main:
.Push_Registers:
  PUSH  {R4-R12, LR}                             // push registers R4-R12, LR to the stack
.GPIO16_Config:
  LDR   R0, =PADS_BANK0_GPIO16_OFFSET            // load PADS_BANK0_GPIO16_OFFSET
  LDR   R1, =IO_BANK0_GPIO16_CTRL_OFFSET         // load IO_BANK0_GPIO16_CTRL_OFFSET
  LDR   R2, =16                                  // load GPIO number
  BL    GPIO_Config                              // call GPIO_Config
.Loop:
  LDR   R0, =16                                  // load GPIO number
  BL    GPIO_Set                                 // call GPIO_Set
  LDR   R0, =500                                 // 500ms
  BL    Delay_MS                                 // call Delay_MS
  LDR   R0, =16                                  // load GPIO number
  BL    GPIO_Clear                               // call GPIO_Clear
  LDR   R0, =500                                 // 500ms
  BL    Delay_MS                                 // call Delay_MS
  B     .Loop                                    // loop forever
.Pop_Registers:
  POP   {R4-R12, LR}                             // pop registers R4-R12, LR from the stack
  BX    LR                                       // return to caller

/**
 * @brief   Configure GPIO.
 *
 * @details Configures a GPIO pin's pad control and function select.
 *
 * @param   R0 - PAD_OFFSET
 * @param   R1 - CTRL_OFFSET
 * @param   R2 - GPIO
 * @retval  None
 */
.type GPIO_Config, %function
GPIO_Config:
.GPIO_Config_Push_Registers:
  PUSH  {R4-R12, LR}                             // push registers R4-R12, LR to the stack
.GPIO_Config_Modify_Pad:
  LDR   R4, =PADS_BANK0_BASE                     // load PADS_BANK0_BASE address
  ADD   R4, R4, R0                               // PADS_BANK0_BASE + PAD_OFFSET
  LDR   R5, [R4]                                 // read PAD_OFFSET value
  BIC   R5, R5, #(1<<7)                          // clear OD bit
  ORR   R5, R5, #(1<<6)                          // set IE bit
  BIC   R5, R5, #(1<<8)                          // clear ISO bit
  STR   R5, [R4]                                 // store value into PAD_OFFSET
.GPIO_Config_Modify_CTRL:
  LDR   R4, =IO_BANK0_BASE                       // load IO_BANK0 base
  ADD   R4, R4, R1                               // IO_BANK0_BASE + CTRL_OFFSET
  LDR   R5, [R4]                                 // read CTRL_OFFSET value
  BIC   R5, R5, #0x1F                            // clear FUNCSEL
  ORR   R5, R5, #0x05                            // set FUNCSEL 0x05->SIO_0
  STR   R5, [R4]                                 // store value into CTRL_OFFSET
.GPIO_Config_Enable_OE:
  LDR   R4, =1                                   // enable output
  MCRR  P0, #4, R2, R4, C4                       // gpioc_bit_oe_put(GPIO, 1)
.GPIO_Config_Pop_Registers:
  POP   {R4-R12, LR}                             // pop registers R4-R12, LR from the stack
  BX    LR                                       // return

/**
 * @brief   GPIO set.
 *
 * @details Drives GPIO output high via coprocessor.
 *
 * @param   R0 - GPIO
 * @retval  None
 */
.type GPIO_Set, %function
GPIO_Set:
.GPIO_Set_Push_Registers:
  PUSH  {R4-R12, LR}                             // push registers R4-R12, LR to the stack
.GPIO_Set_Execute:
  LDR   R4, =1                                   // enable output
  MCRR  P0, #4, R0, R4, C0                       // gpioc_bit_out_put(GPIO, 1)
.GPIO_Set_Pop_Registers:
  POP   {R4-R12, LR}                             // pop registers R4-R12, LR from the stack
  BX    LR                                       // return

/**
 * @brief   GPIO clear.
 *
 * @details Drives GPIO output high via coprocessor.
 *
 * @param   R0 - GPIO
 * @retval  None
 */
.type GPIO_Clear, %function
GPIO_Clear:
.GPIO_Clear_Push_Registers:
  PUSH  {R4-R12, LR}                             // push registers R4-R12, LR to the stack
.GPIO_Clear_Execute:
  LDR   R4, =0                                   // disable output
  MCRR  P0, #4, R0, R4, C0                       // gpioc_bit_out_put(GPIO, 1)
.GPIO_Clear_Pop_Registers:
  POP   {R4-R12, LR}                             // pop registers R4-R12, LR from the stack
  BX    LR                                       // return

/**
 * @brief   Delay_MS.
 *
 * @details Delays for R0 milliseconds. Conversion: loop_count = ms * 3600
 *          based on a 14.5MHz clock.
 *
 * @param   R0 - milliseconds
 * @retval  None
 */
.type Delay_MS, %function
Delay_MS:
.Delay_MS_Push_Registers:
  PUSH  {R4-R12, LR}                             // push registers R4-R12, LR to the stack
.Delay_MS_Check:
  CMP   R0, #0                                   // if MS is not valid, return
  BLE   .Delay_MS_Done                           // branch if less or equal to 0 
.Delay_MS_Setup:
  LDR   R4, =3600                                // loops per MS based on 14.5MHz clock
  MUL   R5, R0, R4                               // MS * 3600
.Delay_MS_Loop:
  SUBS  R5, R5, #1                               // decrement counter
  BNE   .Delay_MS_Loop                           // branch until zero
.Delay_MS_Done:
  POP   {R4-R12, LR}                             // pop registers R4-R12, LR from the stack
  BX    LR                                       // return

/**
 * Test data and constants.
 * The .rodata section is used for constants and static data.
 */
.section .rodata                                 // read-only data section

/**
 * Initialized global data.
 * The .data section is used for initialized global or static variables.
 */
.section .data                                   // data section

/**
 * Uninitialized global data.
 * The .bss section is used for uninitialized global or static variables.
 */
.section .bss                                    // BSS section
```

<br>

## License
[Apache License 2.0](https://github.com/mytechnotalent/RP2350_Blink_Driver/blob/main/LICENSE)
