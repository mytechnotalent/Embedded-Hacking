cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(0x000b_integer-data-type C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(0x000b_integer-data-type
    main.c
)

pico_enable_stdio_usb(0x000b_integer-data-type 1)
pico_enable_stdio_uart(0x000b_integer-data-type 1)

pico_add_extra_outputs(0x000b_integer-data-type)

target_link_libraries(0x000b_integer-data-type 
    pico_stdlib
)
