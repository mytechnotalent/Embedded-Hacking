#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void)
{
    stdio_init_all();

    while (true)
    {
        static int static_fav_num = 42;
        int regular_fav_num = 42;

        printf("static_fav_num: %d\r\n", static_fav_num);
        printf("regular_fav_num: %d\r\n", regular_fav_num);

        static_fav_num++;
        regular_fav_num++;
    }
}
