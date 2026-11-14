/**
 * FILE: commands.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-3 Command Handlers.
 *
 * BRIEF:
 * Handlers for the console commands and the {name, handler} dispatch table.
 * Each handler prints a short line over UART0 and returns to the dispatcher.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

.section .rodata                                 // read-only data section
.global msg_prompt                               // exported to console.s
.global msg_unknown                              // exported to console.s
.global cmd_table                                // exported to console.s
.global msg_banner                                // exported to console.s
msg_banner:    .asciz "\r\nFUN SOCIETY AMUSEMENT, LLC\r\nDEAD DROP TERMINAL v0.9\r\nAUTH REQUIRED\r\n"
msg_company:   .asciz "Fun Society Amusement, LLC"
msg_prompt:    .asciz "\r\n> "                   // console prompt
msg_unknown:   .asciz "?\r\n"                    // unknown command
msg_help:      .asciz "help status led unlock\r\n"
msg_status:    .asciz "[stage3] running\r\n"
msg_led:       .asciz "[led] toggled\r\n"
msg_unlock:    .asciz "[lock] challenge issued\r\n"
msg_factory:   .asciz "[factory] FUN SOCIETY AMUSEMENT, LLC\r\nASSET LEDGER PALE-HORSE-7F3A\r\n"
cmd_help:      .asciz "help"
cmd_status:    .asciz "status"
cmd_led:       .asciz "led"
cmd_unlock:    .asciz "unlock"
cmd_factory:   .asciz "factory"
.align 2
cmd_table:
  .word cmd_help,    Cmd_Help
  .word cmd_status,  Cmd_Status
  .word cmd_led,     Cmd_Led
  .word cmd_unlock,  Cmd_Unlock
  .word cmd_factory, Cmd_Factory
  .word 0

.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   `help` handler.
 *
 * @param   None
 * @retval  None
 */
.global Cmd_Help
.type Cmd_Help, %function
Cmd_Help:
.Cmd_Help_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =msg_help                            // help text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   `status` handler.
 *
 * @param   None
 * @retval  None
 */
.global Cmd_Status
.type Cmd_Status, %function
Cmd_Status:
.Cmd_Status_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =msg_status                          // status text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   `led` handler.
 *
 * @details Drives LED1 high through the GPIOC coprocessor.
 *
 * @param   None
 * @retval  None
 */
.global Cmd_Led
.type Cmd_Led, %function
Cmd_Led:
.Cmd_Led_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =LED1                                // LED1 pin
  bl    GPIO_Set                                 // drive it high
  ldr   r0, =msg_led                             // led text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   `unlock` handler.
 *
 * @details Placeholder for the rolling-lock challenge issued by the ghost.
 *
 * @param   None
 * @retval  None
 */
.global Cmd_Unlock
.type Cmd_Unlock, %function
Cmd_Unlock:
.Cmd_Unlock_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =msg_unlock                          // unlock text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   `factory` handler.
 *
 * @details The hidden mode. Prints the manifest. In the final build this is
 *          reached only through the off-by-one or the rolling lock.
 *
 * @param   None
 * @retval  None
 */
.global Cmd_Factory
.type Cmd_Factory, %function
Cmd_Factory:
.Cmd_Factory_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =msg_factory                         // manifest text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return
