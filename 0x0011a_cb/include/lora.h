// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    lora.h
// Desc:    Declares UART1 interface for the REYAX RYLR998 LoRa transceiver.
// Created: 2026

#ifndef LORA_H
#define LORA_H

#include "hardware/uart.h"

#define LORA_UART     uart1
#define LORA_BAUD     115200
#define LORA_TX_PIN   8
#define LORA_RX_PIN   9

/**
 * @brief Initialize LoRa transceiver over UART1 on GPIO8 and GPIO9
 *
 * @param None.
 * @return None.
 */
void init_lora(void);

/**
 * @brief Transmit string message over LoRa UART1 interface
 *
 * @param msg Null-terminated string buffer to transmit.
 * @return None.
 */
void lora_send(const char *msg);

#endif // LORA_H
