#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    uint8_t age = 42;
    
    age = 43;

    stdio_init_all();

    while (true)
        printf("age: %d\r\n", age);
}
