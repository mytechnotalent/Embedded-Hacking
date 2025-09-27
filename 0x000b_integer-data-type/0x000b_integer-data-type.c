#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    uint8_t age = 0;
    int8_t range = 0;
    
    age = 43;
    range = -42;

    stdio_init_all();

    while (true) {
        printf("age: %d\r\n", age);
        printf("range: %d\r\n", range);
    }
}
