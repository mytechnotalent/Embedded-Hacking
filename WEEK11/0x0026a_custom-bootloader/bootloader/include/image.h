/**
 * @file image.h
 * @brief Control transfer into a freshly verified Cortex-M application.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

/**
 * @brief Jump to an application whose vector table starts at a base address.
 * @param vector_base Base address of the application vector table.
 * @return void Does not return to the caller.
 */
void image_start(uint32_t vector_base);

#endif
