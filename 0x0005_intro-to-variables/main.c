#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void)
{
    uint8_t age = 42;
    age = 43;

    stdio_init_all();

    while (true)
    {
        printf("age: %d\r\n", age);
    }
}
