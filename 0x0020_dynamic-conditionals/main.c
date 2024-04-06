#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "input.h"

#define FAV_NUM 42
#define ONE 0x31
#define TWO 0x32

int main(void) 
{
    stdio_init_all();
    uart0_init();

    uint8_t choice = 0;

    while (true) {
        choice = on_uart_rx();
        if (choice == ONE) {
            printf("1\r\n");
        } else if (choice == TWO) {
            printf("2\r\n");
        } else {
            printf("??\r\n");
        }
        switch (choice) {
            case '1':
                printf("one\r\n");
                break;
            case '2':
                printf("two\r\n");
                break;   
            default:
                printf("??\r\n");  
        }
    }
}
