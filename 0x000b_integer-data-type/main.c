#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void)
{
    uint8_t age = 0;
    int8_t range = 0;
    
    age = 43;
    range = -42;

    stdio_init_all();

    while (true)
    {
        printf("age: %d\r\n", age);
        printf("range: %d\r\n", range);
    }
}
