#include <stdio.h>
#include "pico/stdlib.h"


int main()
{
    stdio_init_all();

    while (true) {
        __asm volatile(
            // Save a low register and the link register.
            "push {r4, lr}\n"
            // Intentionally unordered: the encoded list is still r2, r3, r6.
            "push {r3, r2, r6}\n"
            // Thumb PUSH cannot encode high registers r8-r12.
            "stmdb sp!, {r9, r10}\n"
            // Restore each group in reverse order to return SP to its start.
            "ldmia sp!, {r9, r10}\n"
            "pop {r2, r3, r6}\n"
            "pop {r4, lr}\n"
            ::: "memory");
    }
}
