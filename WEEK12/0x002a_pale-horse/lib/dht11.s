/**
 * FILE: dht11.s
 *
 * DESCRIPTION:
 * RP2350 DHT11 Single-Wire Sensor Functions.
 *
 * BRIEF:
 * Bit-banged DHT11 read on GP4. Issues the start pulse, samples the 40-bit
 * response, and verifies the checksum. Timing is derived from CLK_PERI.
 *
 * NOTE:
 * The microsecond loops assume CLK_PERI = 14.5 MHz. Validate against a real
 * sensor; one-wire timing is the first thing to tune if reads fail.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

.include "constants.s"

.section .bss                                    // BSS section
.align 2
dht_buf:
  .space 5                                       // humidity/temp/checksum bytes

.section .text                                   // code section
.align 2                                         // align to 4-byte boundary

/**
 * @brief   Short microsecond busy delay.
 *
 * @param   r0 - microseconds
 * @retval  None
 */
.global DHT11_Delay_US
.type DHT11_Delay_US, %function
DHT11_Delay_US:
.DHT11_Delay_US_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r1, =14                                  // loop iterations per microsecond
  mul   r2, r0, r1                               // total iterations
.DHT11_Delay_US_Loop:
  subs  r2, r2, #1                               // decrement
  bne   .DHT11_Delay_US_Loop                     // loop until zero
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Read one 40-bit frame from the DHT11.
 *
 * @details Drives GP4 low for 18 ms, releases it, samples 40 bits, and
 *          checks the checksum. Results are stored in dht_buf.
 *
 * @param   None
 * @retval  r0 - 0 on success, 1 on checksum failure
 */
.global DHT11_Read
.type DHT11_Read, %function
DHT11_Read:
.DHT11_Read_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
.DHT11_Read_Start:
  ldr   r0, =DHT11                               // data pin
  ldr   r4, =1                                   // enable output
  mcrr  p0, #4, r0, r4, c4                       // gpioc_bit_oe_put(pin, 1)
  ldr   r4, =0                                   // drive low
  mcrr  p0, #4, r0, r4, c0                       // gpioc_bit_out_put(pin, 0)
  ldr   r0, =18000                               // 18 ms start pulse
  bl    DHT11_Delay_US                           // hold low
  ldr   r0, =DHT11                               // data pin
  ldr   r4, =1                                   // drive high (release)
  mcrr  p0, #4, r0, r4, c0                       // gpioc_bit_out_put(pin, 1)
  ldr   r4, =0                                   // disable output -> input
  mcrr  p0, #4, r0, r4, c4                       // gpioc_bit_oe_put(pin, 0)
  ldr   r0, =30                                  // 30 us release
  bl    DHT11_Delay_US                           // wait
.DHT11_Read_Bits:
  ldr   r4, =dht_buf                             // frame buffer
  mov   r5, #5                                   // five bytes
  mov   r6, #0                                   // byte accumulator
.DHT11_Read_Byte_Loop:
  ldr   r7, =8                                   // eight bits per byte
.DHT11_Read_Bit_Loop:
  bl    DHT11_Wait_Rise                          // wait for the low period to end
  add   r6, r6, r6                               // shift accumulator left
  bl    DHT11_Sample_High                        // measure the high period
  orr   r6, r6, r0                               // append the bit
  subs  r7, r7, #1                               // decrement bit count
  bne   .DHT11_Read_Bit_Loop                     // next bit
  strb  r6, [r4], #1                             // store byte
  mov   r6, #0                                   // reset accumulator
  subs  r5, r5, #1                               // decrement byte count
  bne   .DHT11_Read_Byte_Loop                    // next byte
.DHT11_Read_Checksum:
  ldr   r0, =dht_buf                             // frame buffer
  ldrb  r1, [r0, #0]                             // humidity integer
  ldrb  r2, [r0, #1]                             // humidity decimal
  ldrb  r3, [r0, #2]                             // temperature integer
  ldrb  r4, [r0, #3]                             // temperature decimal
  add   r1, r1, r2                               // sum
  add   r1, r1, r3                               // sum
  add   r1, r1, r4                               // sum
  and   r1, r1, #0xff                            // low byte
  ldrb  r2, [r0, #4]                             // checksum byte
  cmp   r1, r2                                   // match?
  bne   .DHT11_Read_Fail                         // no -> fail
  mov   r0, #0                                   // success
  b     .DHT11_Read_Done                         // done
.DHT11_Read_Fail:
  mov   r0, #1                                   // checksum failure
.DHT11_Read_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return

/**
 * @brief   Wait for the data line to go high.
 *
 * @retval  None
 */
.global DHT11_Wait_Rise
.type DHT11_Wait_Rise, %function
DHT11_Wait_Rise:
.DHT11_Wait_Rise_Loop:
  ldr   r0, =SIO_BASE                            // SIO base
  ldr   r1, [r0, #SIO_GPIO_IN]                   // read all pin inputs
  tst   r1, #(1<<4)                              // GP4 high yet?
  beq   .DHT11_Wait_Rise_Loop                    // no -> keep waiting
  bx    lr                                       // return

/**
 * @brief   Sample the high period of a bit.
 *
 * @details Short high -> 0, long high -> 1.
 *
 * @retval  r0 - 0 or 1
 */
.global DHT11_Sample_High
.type DHT11_Sample_High, %function
DHT11_Sample_High:
.DHT11_Sample_High_Push_Registers:
  push  {r4-r12, lr}                             // push registers r4-r12, lr to the stack
  ldr   r4, =40                                  // ~40 us sample point
.DHT11_Sample_High_Loop:
  subs  r4, r4, #1                               // decrement
  beq   .DHT11_Sample_High_One                   // still high -> 1
  ldr   r0, =SIO_BASE                            // SIO base
  ldr   r1, [r0, #SIO_GPIO_IN]                   // read inputs
  tst   r1, #(1<<4)                              // GP4 still high?
  bne   .DHT11_Sample_High_Loop                  // yes -> keep sampling
  mov   r0, #0                                   // went low -> bit 0
  b     .DHT11_Sample_High_Done                  // done
.DHT11_Sample_High_One:
  mov   r0, #1                                   // stayed high -> bit 1
.DHT11_Sample_High_Done:
  pop   {r4-r12, lr}                             // pop registers r4-r12, lr from the stack
  bx    lr                                       // return
