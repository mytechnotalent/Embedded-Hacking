#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define FAV_NUM 42

void print_me(void);

int main(void)
{
    stdio_init_all();

    while (true)
    {
        print_me();
    }
}

void print_me(void)
{
    printf("FAV_NUM: %d\r\n", FAV_NUM);
}
