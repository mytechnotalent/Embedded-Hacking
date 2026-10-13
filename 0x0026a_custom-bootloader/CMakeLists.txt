# 0x0026a custom bootloader: builds a secure bootloader plus the signed
# application it launches. Run from this directory:
#
#   cmake -B build -G Ninja -DPICO_BOARD=pico2 \
#         -DPICO_SDK_PATH="$HOME/.pico-sdk/sdk/2.2.0" \
#         -DPICO_TOOLCHAIN_PATH="$HOME/.pico-sdk/toolchain/14_2_Rel1"
#   cmake --build build

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

project(0x0026a_custom_bootloader C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# --- Bootloader: verifies an AES-128-CMAC tag before launching the app --------
add_executable(0x0026a_bootloader
        bootloader/src/main.c
        bootloader/src/boot.c
        bootloader/src/manifest.c
        bootloader/src/aes_cmac.c
        bootloader/src/image.c
        bootloader/src/verify.c)

target_include_directories(0x0026a_bootloader PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/bootloader/include
        ${CMAKE_CURRENT_LIST_DIR}/common/include)

# pico_mbedtls provides the AES-128-CMAC implementation used for verification.
target_link_libraries(0x0026a_bootloader PRIVATE
        pico_stdlib
        pico_mbedtls)

pico_enable_stdio_uart(0x0026a_bootloader 1)
pico_enable_stdio_usb(0x0026a_bootloader 0)
pico_add_extra_outputs(0x0026a_bootloader)

# --- Application: the 0x0026_functions firmware linked at 0x10020000 ----------
add_executable(0x0026a_app
        app/src/app_main.c
        app/src/ir.c
        app/src/led.c
        app/src/secret.c)

target_include_directories(0x0026a_app PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/app/include
        ${CMAKE_CURRENT_LIST_DIR}/common/include)

# pico_mbedtls provides the AES-128-CBC API used by secret.c.
target_link_libraries(0x0026a_app PRIVATE
        pico_stdlib
        pico_mbedtls)

pico_enable_stdio_uart(0x0026a_app 1)
pico_enable_stdio_usb(0x0026a_app 0)

# Relocate the application into flash above the bootloader and manifest.
pico_set_linker_script(0x0026a_app
        ${CMAKE_CURRENT_LIST_DIR}/app/memmap_app.ld)

# The application starts at 0x10020000, so a UF2 (which BOOTSEL always writes to
# 0x10000000) would be wrong. Emit the raw artifacts we flash with OpenOCD.
pico_add_dis_output(0x0026a_app)
pico_add_hex_output(0x0026a_app)
pico_add_bin_output(0x0026a_app)
pico_add_map_output(0x0026a_app)
