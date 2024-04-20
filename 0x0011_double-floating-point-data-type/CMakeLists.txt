cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(0x0011_double-floating-point-data-type C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(0x0011_double-floating-point-data-type
    main.c
)

pico_enable_stdio_usb(0x0011_double-floating-point-data-type 1)
pico_enable_stdio_uart(0x0011_double-floating-point-data-type 1)

pico_add_extra_outputs(0x0011_double-floating-point-data-type)

target_link_libraries(0x0011_double-floating-point-data-type
    pico_stdlib
)
