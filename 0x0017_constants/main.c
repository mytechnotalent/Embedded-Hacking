#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define FAV_NUM 42

int main(void)
{
    stdio_init_all();

    while (true)
    {
        const int OTHER_FAV_NUM = 1337;

        printf("FAV_NUM: %d\r\n", FAV_NUM);
        printf("regular_fav_num: %d\r\n", OTHER_FAV_NUM);
    }
}
