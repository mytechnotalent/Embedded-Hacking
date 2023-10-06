/**
 * FILE: main.s
 *
 * DESCRIPTION: This file contains the main function for the hello-world program utilizing the STM32F401CC6 microcontroller.
 *
 * AUTHOR: Kevin Thomas
 * DATE: October 6, 2023
 */

#include <stdio.h>

#include "usart.h"

int main(void)
{
    usart2_tx_init();

    while (1)
    {
        printf("hello, world\r\n");
    }
}
