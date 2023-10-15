#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void)
{
    stdio_init_all();

    while (true)
    {
        printf("hello, world\r\n");
    }
}
