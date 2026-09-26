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
// File:    payload.c
// Desc:    Implements payload release solenoid latch control on GPIO16.
// Created: 2026

#include "payload.h"
#include "lora.h"
#include "hardware/gpio.h"
#include <stdio.h>

void init_payload(void)
{
    gpio_init(PAYLOAD_PIN);
    gpio_set_dir(PAYLOAD_PIN, GPIO_OUT);
    gpio_put(PAYLOAD_PIN, 0);
}

void release_payload(void)
{
    gpio_put(PAYLOAD_PIN, 1);
    printf("PAYLOAD RELEASED AT TARGET COORDINATES\r\n");
    lora_send("PAYLOAD RELEASED AT TARGET COORDINATES\r\n");
}
