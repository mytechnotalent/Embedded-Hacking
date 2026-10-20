#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    double fav_num = 42.52525;
    
    stdio_init_all();

    while (true)
        printf("fav_num: %lf\r\n", fav_num);
}
