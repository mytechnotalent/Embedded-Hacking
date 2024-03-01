#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define FAV_NUM 42

int main(void)
{
    stdio_init_all();

    int choice = 1;

    while (true)
    {
        if (choice == 1) {
            printf("1\r\n");
        } else if (choice == 2) {
            printf("2\r\n");
        } else {
            printf("?\r\n");
        }

        switch (choice) {
            case 1:
                printf("one\r\n");
                break;
            case 2:
                printf("two\r\n");
                break;   
            default:
                printf("??\r\n");  
        }
    }
}
