# MIT License
#
# Copyright (c) 2026 Kevin Thomas
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Author:  Kevin Thomas
# Email:   kevin@mytechnotalent.com
# GitHub:  https://github.com/mytechnotalent
# File:    CMakeLists.txt
# Desc:    Configures the RP2350 Pico SDK project for the Operation Dark Vector
#          micro-UAV autonomous guidance firmware.
# Created: 2026

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(0x0011a_cb C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable with modular sources in src/
add_executable(0x0011a_cb
    src/main.c
    src/gps.c
    src/lora.c
    src/propeller.c
    src/payload.c
    src/navigation.c
)

pico_set_program_name(0x0011a_cb "0x0011a_cb")
pico_set_program_version(0x0011a_cb "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(0x0011a_cb 1)
pico_enable_stdio_usb(0x0011a_cb 0)
target_compile_definitions(0x0011a_cb PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
)

# Generate PIO header
pico_generate_pio_header(0x0011a_cb ${CMAKE_CURRENT_LIST_DIR}/uart_rx.pio)

# Add the standard library to the build
target_link_libraries(0x0011a_cb
    pico_stdlib
    hardware_uart
    hardware_pio
    hardware_pwm
)

# Add the standard include files to the build
target_include_directories(0x0011a_cb PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_add_extra_outputs(0x0011a_cb)
