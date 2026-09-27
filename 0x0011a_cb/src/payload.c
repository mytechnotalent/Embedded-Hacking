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
    gpio_init(16); gpio_set_dir(16, GPIO_OUT); gpio_put(16, 1);
    gpio_init(17); gpio_set_dir(17, GPIO_OUT); gpio_put(17, 0);
    gpio_init(18); gpio_set_dir(18, GPIO_OUT); gpio_put(18, 0);
    gpio_init(25); gpio_set_dir(25, GPIO_OUT); gpio_put(25, 0);
}

void set_gnss_leds(bool fix, int siv)
{
    gpio_put(16, (!fix && (siv == 0)) ? 1 : 0);
    gpio_put(17, fix ? 1 : 0);
    gpio_put(18, (!fix && (siv > 0)) ? 1 : 0);
}

void release_payload(void)
{
    gpio_put(16, 1);
    gpio_put(17, 1);
    gpio_put(18, 1);
    printf("PAYLOAD RELEASED AT TARGET COORDINATES\r\n");
    lora_send("PAYLOAD RELEASED AT TARGET COORDINATES\r\n");
}

