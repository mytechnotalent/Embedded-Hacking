cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(0x0014_static-variables C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(0x0014_static-variables
    main.c
)

pico_enable_stdio_usb(0x0014_static-variables 1)
pico_enable_stdio_uart(0x0014_static-variables 1)

pico_add_extra_outputs(0x0014_static-variables)

target_link_libraries(0x0014_static-variables 
    pico_stdlib
)
