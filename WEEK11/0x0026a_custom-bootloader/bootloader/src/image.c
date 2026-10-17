/**
 * @file image.c
 * @brief Loads the application stack pointer and branches to its reset vector.
 * @author Kevin Thomas
 * @date 2026
 *
 * MIT License
 * Copyright (c) 2026 Kevin Thomas
 */

#include "image.h"

/** @brief Application reset handler signature. */
typedef void (*image_entry_t)(void);

/**
 * @brief Mask interrupts and point VTOR at the incoming image.
 * @param base Vector table base address written to the VTOR register.
 * @return void No return value.
 */
static void halt_irq_and_rebase(uint32_t base) {
    __asm volatile ("cpsid i" ::: "memory");
    *(volatile uint32_t *)0xE000ED08U = base;
}

void image_start(uint32_t vector_base) {
    uint32_t *vectors = (uint32_t *)vector_base;
    image_entry_t entry = (image_entry_t)vectors[1];
    halt_irq_and_rebase(vector_base);
    __asm volatile ("msr msp, %0" :: "r" (vectors[0]) : "memory");
    __asm volatile ("dsb\n\tisb" ::: "memory");
    entry();
}
