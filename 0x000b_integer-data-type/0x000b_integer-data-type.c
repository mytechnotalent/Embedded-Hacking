#include <stdio.h>                          
#include "pico/stdlib.h"                   

int main(void) {                           
    uint8_t age = 43;                       
    int8_t range = -42;                     

    stdio_init_all();                     

    __asm volatile (                        
        "ldr r3, =0x40038000\n"             // address of PADS_BANK0_BASE
        "ldr r2, =0x40028004\n"             // address of IO_BANK0 GPIO0.ctrl
        "movs r0, #16\n"                    // GPIO16 (start pin)

        "init_loop:\n"                      // loop start
            "lsls r1, r0, #2\n"             // pin * 4 (pad offset)
            "adds r4, r3, r1\n"             // PADS base + offset
            "ldr  r5, [r4]\n"               // load current config
            "bic  r5, r5, #0x180\n"         // clear OD+ISO
            "orr  r5, r5, #0x40\n"          // set IE
            "str  r5, [r4]\n"               // store updated config

            "lsls r1, r0, #3\n"             // pin * 8 (ctrl offset)
            "adds r4, r2, r1\n"             // IO_BANK0 base + offset
            "ldr  r5, [r4]\n"               // load current config
            "bic  r5, r5, #0x1f\n"          // clear FUNCSEL bits [4:0]
            "orr  r5, r5, #5\n"             // set FUNCSEL = 5 (SIO)
            "str  r5, [r4]\n"               // store updated config

            "mov  r4, r0\n"                 // pin
            "movs r5, #1\n"                 // bit 1; used for OUT/OE writes
            "mcrr p0, #4, r4, r5, c4\n"     // gpioc_bit_oe_put(pin,1)
            "adds r0, r0, #1\n"             // increment pin
            "cmp  r0, #20\n"                // stop after pin 18
            "blt  init_loop\n"              // loop until r0 == 20
    );                              

    uint8_t pin = 16;                 

    while (1) {                     
        __asm volatile (                
            "mov r4, %0\n"                  // pin
            "movs r5, #0x01\n"              // bit 1; used for OUT/OE writes
            "mcrr p0, #4, r4, r5, c0\n"     // gpioc_bit_out_put(16, 1) 
            :
            : "r"(pin)
            : "r4","r5"
        );
        sleep_ms(500);                 

        __asm volatile (              
            "mov r4, %0\n"                  // pin
            "movs r5, #0\n"                 // bit 0; used for OUT/OE writes
            "mcrr p0, #4, r4, r5, c0\n"     // gpioc_bit_out_put(16, 0)
            :
            : "r"(pin)
            : "r4","r5"
        );
        sleep_ms(500);   

        pin++;                              
        if (pin > 18) pin = 16;            

        printf("age: %d\r\n", age);   
        printf("range: %d\r\n", range);
    }                                 
}
