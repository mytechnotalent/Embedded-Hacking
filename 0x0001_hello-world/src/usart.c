/**
 * FILE: usart.s
 *
 * DESCRIPTION: This file contains the usart functions utilizing the STM32F401CC6 microcontroller.
 *
 * AUTHOR: Kevin Thomas
 * DATE: October 6, 2023
 */

#include "usart.h"

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void)file;
    int data_idx;

    for (data_idx = 0; data_idx < len; data_idx++)
    {
        __io_putchar(*ptr++);
    }

    return len;
}

int __io_putchar(int ch)
{
    // write a single char
    usart2_write(ch);

    return ch;
}

static void set_baud_rate(USART_TypeDef *USARTx,  uint32_t periph_clk, uint32_t baud_rate)
{
    // set baud rate register
    USARTx->BRR = ((periph_clk + (baud_rate / 2)) / baud_rate);
}

void usart2_tx_init(void)
{
    // enable GPIOA clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // set PA2 to output alternate function mode
    GPIOA->MODER |= GPIO_MODER_MODE2_1;
    GPIOA->MODER &= ~GPIO_MODER_MODE2_0;

    // set PA2 alternate function type to UART_TX (AF07)
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL2_3;
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL2_2;
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL2_1;
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL2_0;

    // enable USART2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // configure baud rate to 9600
    set_baud_rate(USART2, DEFAULT_CLK_SPEED, 9600);

    // configure TX transmitter enable
    USART2->CR1 |= USART_CR1_TE;

    // enable UART2
    USART2->CR1 |= USART_CR1_UE;
}

void usart2_write(uint8_t ch)
{
    // verify transmit data register is empty, will return true if the bit is set
    while(!(USART2->SR & USART_SR_TXE)) {};

    // write a single char to the transmit data register
    USART2->DR = (ch & 0xFF);
}
