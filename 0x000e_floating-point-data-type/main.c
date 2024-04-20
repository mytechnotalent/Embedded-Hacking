#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    float fav_num = 42.5;
    
    stdio_init_all();

    while (true)
        printf("fav_num: %f\r\n", fav_num);
}
