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

int __io_putchar(int char_)
{
    // write a single char
    usart2_write_char(char_);

    return char_;
}

static void set_baud_rate(USART_TypeDef *USARTx,  uint32_t periph_clk, uint32_t baud_rate)
{
    // set baud rate register
    USARTx->BRR = ((periph_clk + (baud_rate / 2)) / baud_rate);
}

void usart2_init(void)
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

    // set PA3 to output alternate function mode
    GPIOA->MODER |= GPIO_MODER_MODE3_1;
    GPIOA->MODER &= ~GPIO_MODER_MODE3_0;

    // set PA3 alternate function type to UART_RX (AF07)
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL3_3;
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL3_2;
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL3_1;
    GPIOA->AFR[0] |= GPIO_AFRL_AFRL3_0;

    // enable USART2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // configure baud rate to 9600
    set_baud_rate(USART2, DEFAULT_CLK_SPEED, 9600);

    // configure TX transmitter enable
    USART2->CR1 |= USART_CR1_TE;

    // configure RX receiver enable
    USART2->CR1 |= USART_CR1_RE;

    // enable UART2
    USART2->CR1 |= USART_CR1_UE;
}

void usart2_write_char(uint8_t char_)
{
    // verify transmit data register is empty, will return true if the bit is set
    while(!(USART2->SR & USART_SR_TXE)) {};

    // write a single char to the data register
    USART2->DR = (char_ & 0xFF);
}

char usart2_read_char(void)
{
    // verify receive data register is empty, will return true if the bit is set
    while(!(USART2->SR & USART_SR_RXNE)) {};

    // return a single char from the data register
    return USART2->DR;
}

void input(char *buffer, size_t buffer_size)
{
    char input;
    size_t i = 0;

    while (1)
    {
        input = usart2_read_char();

        // check for newline or carriage return
        if (input == '\n' || input == '\r')  
        {
            // null-terminate the string
            buffer[i] = '\0';  
            break;
        }

        // append new char from usart into buffer
        buffer[i++] = input;

        // ensure we don't overflow the buffer
        if (i >= buffer_size - 1)
        {
            // null-terminate the string
            buffer[i] = '\0';  
            break;
        }
    }
}
