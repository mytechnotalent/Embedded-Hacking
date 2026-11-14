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

/**
 * Test data and constants.
 * The .rodata section is used for constants and static data.
 */
.section .rodata                                 // read-only data section
.global msg_prompt                               // exported to console.s
.global msg_unknown                              // exported to console.s
.global cmd_table                                // exported to console.s
.global msg_banner                               // exported to console.s
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
  .word cmd_help,    CMD_Help
  .word cmd_status,  CMD_Status
  .word cmd_led,     CMD_LED
  .word cmd_unlock,  CMD_Unlock
  .word cmd_factory, CMD_Factory
  .word 0

/**
 * Initialize the .text section.
 * The .text section contains executable code.
 */
.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   `help` handler.
 *
 * @param   None
 * @retval  None
 */
.global CMD_Help
.type CMD_Help, %function
CMD_Help:
.CMD_Help_Push_Registers:
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
.global CMD_Status
.type CMD_Status, %function
CMD_Status:
.CMD_Status_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =msg_status                          // status text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   `led` handler.
 *
 * @details Configures LED1 as an SIO output and toggles it.
 *
 * @param   None
 * @retval  None
 */
.global CMD_LED
.type CMD_LED, %function
CMD_LED:
.CMD_LED_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =LED1                                // LED1 pin
  bl    GPIO_Toggle                              // configure and toggle it
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
.global CMD_Unlock
.type CMD_Unlock, %function
CMD_Unlock:
.CMD_Unlock_Push_Registers:
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
.global CMD_Factory
.type CMD_Factory, %function
CMD_Factory:
.CMD_Factory_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r0, =msg_factory                         // manifest text
  bl    Console_PrintStr                         // print it
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return
