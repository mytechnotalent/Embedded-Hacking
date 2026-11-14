/**
 * FILE: console.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-3 Console.
 *
 * BRIEF:
 * Line reader, string helpers, and the command dispatcher. The dispatcher
 * walks a table of {name, handler} pairs and calls the matching handler.
 *
 * NOTE:
 * The line reader's bounds check is intentionally off by one (it permits a
 * full-length line plus its terminator), so a maximum-length command writes
 * one byte past cmd_buf into the adjacent cmd_handler word. That is the
 * planted flaw the capstone asks the student to find and exploit.
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
 * Uninitialized global data.
 */
.section .bss                                    // BSS section
.align 2                                         // align to 4-byte boundary
cmd_buf:
  .space 32                                      // command line buffer
cmd_handler:
  .space 4                                       // adjacent handler selector (bug target)

/**
 * Initialize the .text section.
 */
.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Print a NUL-terminated string over UART0.
 *
 * @param   r0 - string address
 * @retval  None
 */
.global Console_PrintStr
.type Console_PrintStr, %function
Console_PrintStr:
.Console_PrintStr_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  mov   r4, r0                                   // r4 = string pointer
.Console_PrintStr_Loop:
  ldrb  r0, [r4]                                 // load next character
  cmp   r0, #0                                   // end of string?
  beq   .Console_PrintStr_Done                   // yes -> finish
  bl    UART0_Out                                // transmit character
  add   r4, r4, #1                               // advance pointer
  b     .Console_PrintStr_Loop                   // loop
.Console_PrintStr_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Read a CR/LF-terminated line into cmd_buf.
 *
 * @details Off-by-one: the length check permits one byte more than the buffer
 *          holds, so a full-length line leaves its terminator in the adjacent
 *          cmd_handler word.
 *
 * @param   None
 * @retval  None
 */
.global Console_ReadLine
.type Console_ReadLine, %function
Console_ReadLine:
.Console_ReadLine_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r4, =cmd_buf                             // r4 = buffer
  mov   r5, #0                                   // r5 = count
.Console_ReadLine_Loop:
  bl    UART0_In                                 // blocking read
  and   r0, r0, #0xff                            // keep low byte
  cmp   r0, #13                                  // carriage return?
  beq   .Console_ReadLine_Done                   // yes -> done
  cmp   r0, #10                                  // line feed?
  beq   .Console_ReadLine_Done                   // yes -> done
  ldr   r6, =32                                  // buffer size
  cmp   r5, r6                                   // count <= size? (off by one)
  bhi   .Console_ReadLine_Done                   // too long -> stop
  strb  r0, [r4, r5]                             // store character
  add   r5, r5, #1                               // increment count
  b     .Console_ReadLine_Loop                   // loop
.Console_ReadLine_Done:
  mov   r0, #0                                   // terminator
  strb  r0, [r4, r5]                             // write terminator (may be 1 past)
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Compare two NUL-terminated strings.
 *
 * @param   r0 - first string
 * @param   r1 - second string
 * @retval  r0 - 0 if equal, non-zero otherwise
 */
.global Console_Strcmp
.type Console_Strcmp, %function
Console_Strcmp:
.Console_Strcmp_Loop:
  ldrb  r2, [r0]                                 // load first byte
  ldrb  r3, [r1]                                 // load second byte
  cmp   r2, r3                                   // equal?
  bne   .Console_Strcmp_NotEqual                 // no -> not equal
  cmp   r2, #0                                   // end of string?
  beq   .Console_Strcmp_Equal                    // yes -> equal
  add   r0, r0, #1                               // advance first
  add   r1, r1, #1                               // advance second
  b     .Console_Strcmp_Loop                     // loop
.Console_Strcmp_Equal:
  mov   r0, #0                                   // equal
  bx    lr                                       // return
.Console_Strcmp_NotEqual:
  mov   r0, #1                                   // not equal
  bx    lr                                       // return

/**
 * @brief   Dispatch the line in cmd_buf.
 *
 * @details Walks the {name, handler} table in .rodata and calls the first
 *          handler whose name matches. Unknown commands print a message.
 *
 * @param   None
 * @retval  None
 */
.global Console_Dispatch
.type Console_Dispatch, %function
Console_Dispatch:
.Console_Dispatch_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r4, =cmd_table                           // r4 = table cursor
.Console_Dispatch_Loop:
  ldr   r5, [r4]                                 // load name pointer
  cmp   r5, #0                                   // end of table?
  beq   .Console_Dispatch_Unknown                // yes -> unknown
  ldr   r0, =cmd_buf                             // command line
  mov   r1, r5                                   // candidate name
  bl    Console_Strcmp                           // compare
  cmp   r0, #0                                   // match?
  beq   .Console_Dispatch_Found                  // yes -> call
  add   r4, r4, #8                               // advance to next entry
  b     .Console_Dispatch_Loop                   // loop
.Console_Dispatch_Found:
  ldr   r5, [r4, #4]                             // load handler address
  blx   r5                                       // call the handler
  b     .Console_Dispatch_Done                   // done
.Console_Dispatch_Unknown:
  ldr   r0, =msg_unknown                         // unknown-command message
  bl    Console_PrintStr                         // print it
.Console_Dispatch_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Console main loop.
 *
 * @param   None
 * @retval  None
 */
.global Console_Loop
.type Console_Loop, %function
Console_Loop:
.Console_Loop_Banner:
  ldr   r0, =msg_banner                          // boot banner
  bl    Console_PrintStr                         // print once
.Console_Loop_Prompt:
  ldr   r0, =msg_prompt                          // prompt string
  bl    Console_PrintStr                         // print prompt
.Console_Loop_Read:
  bl    Console_ReadLine                         // read a line
.Console_Loop_Dispatch:
  bl    Console_Dispatch                         // dispatch it
  b     .Console_Loop_Prompt                     // loop forever
.size Console_Loop, . - Console_Loop
