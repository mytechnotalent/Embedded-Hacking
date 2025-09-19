#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    uint8_t age;

    stdio_init_all();

    while (true)
        printf("age: %d\r\n", age);
}
