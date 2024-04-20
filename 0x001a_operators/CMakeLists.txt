cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(0x001a_operators C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(0x001a_operators
    main.c
)

pico_enable_stdio_usb(0x001a_operators 1)
pico_enable_stdio_uart(0x001a_operators 1)

pico_add_extra_outputs(0x001a_operators)

target_link_libraries(0x001a_operators 
    pico_stdlib
)
