/**
 * @file 0x001a_operators.c
 * @brief Operators: demonstrate C operators with DHT11 temperature sensor
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * -----------------------------------------------------------------------------
 *
 * Demonstrates arithmetic, increment, relational, logical, bitwise, and
 * assignment operators in C. Also reads humidity and temperature from a
 * DHT11 sensor on GPIO4.
 *
 * Wiring:
 *   GPIO4 -> DHT11 data pin (with 10k pull-up to 3V3)
 *   3V3   -> DHT11 VCC
 *   GND   -> DHT11 GND
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "dht11.h"

/**
 * @brief Print pre-computed operator results
 *
 * @details Prints all six operator demonstration values over UART.
 *
 * @param arith   arithmetic operator result
 * @param inc     increment operator result
 * @param rel     relational operator result
 * @param logic   logical operator result
 * @param bitw    bitwise operator result
 * @param assign  assignment operator result
 * @retval None
 */
static void print_operator_results(int arith, int inc, bool rel,
                                    bool logic, int bitw, int assign) {
    printf("arithmetic_operator: %d\r\n", arith);
    printf("increment_operator: %d\r\n", inc);
    printf("relational_operator: %d\r\n", rel);
    printf("logical_operator: %d\r\n", logic);
    printf("bitwise_operator: %d\r\n", bitw);
    printf("assignment_operator: %d\r\n", assign);
}

/**
 * @brief Compute arithmetic and increment operator results
 *
 * @param arith pointer to store multiplication result
 * @param incr  pointer to store post-increment result
 * @param x     left operand
 * @param y     right operand
 * @retval None
 */
static void compute_arithmetic_ops(int *arith, int *incr, int x, int y) {
    *arith = (x * y);
    *incr = x++;
}

/**
 * @brief Compute all operator values and print results
 *
 * @details Performs arithmetic, relational, logical, bitwise,
 *          and assignment operations, then prints each result.
 *
 * @retval None
 */
static void compute_operators(void) {
    int x = 5, y = 10;
    int arith, incr;
    compute_arithmetic_ops(&arith, &incr, x, y);
    bool rel = (x > y);
    bool logic = (x > y) && (y > x);
    int bits = (x << 1);
    int assign = (x += 5);
    print_operator_results(arith, incr, rel, logic, bits, assign);
}

/**
 * @brief Read and print DHT11 humidity and temperature
 *
 * @details Attempts a DHT11 read; on success prints humidity and
 *          temperature, on failure prints an error message.
 *
 * @retval None
 */
static void print_dht11_reading(void) {
    float hum, temp;
    if (dht11_read(&hum, &temp)) {
        printf("Humidity: %.1f%%, Temperature: %.1f°C\r\n", hum, temp);
    } else {
        printf("DHT11 read failed\r\n");
    }
}

int main(void) {
    stdio_init_all();
    dht11_init(4);
    while (true) {
        compute_operators();
        print_dht11_reading();
        sleep_ms(2000);
    }
}