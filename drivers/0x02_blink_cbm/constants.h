/**
 * @file constants.h
 * @brief Memory-mapped register structures and peripheral base addresses
 * @author Kevin Thomas
 * @date 2025
 *
 * MIT License
 *
 * Copyright (c) 2025 Kevin Thomas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

/**
 * Stack addresses.
 */
#define STACK_TOP   0x20082000U
#define STACK_LIMIT 0x2007A000U

/**
 * XOSC (External Crystal Oscillator) Register Structure.
 */
typedef struct {
    volatile uint32_t CTRL;        /**< 0x00: Control register */
    volatile uint32_t STATUS;      /**< 0x04: Status register */
    volatile uint32_t DORMANT;     /**< 0x08: Dormant mode */
    volatile uint32_t STARTUP;     /**< 0x0C: Startup delay */
    volatile uint32_t RESERVED[3]; /**< 0x10-0x18: Reserved */
    volatile uint32_t COUNT;       /**< 0x1C: Frequency count */
} xosc_hw_t;

/**
 * XOSC base address and pointer.
 */
#define XOSC_BASE 0x40048000U
#define XOSC      ((xosc_hw_t *)XOSC_BASE)

/**
 * XOSC status bit definitions.
 */
#define XOSC_STATUS_STABLE_SHIFT 31U

/**
 * CPACR (Coprocessor Access Control Register) in PPB.
 */
#define PPB_BASE 0xE0000000U
#define CPACR    ((volatile uint32_t *)(PPB_BASE + 0x0ED88U))

/**
 * Coprocessor access control bit definitions.
 */
#define CPACR_CP0_SHIFT 0U
#define CPACR_CP1_SHIFT 1U

/**
 * CLOCKS Register Structure.
 */
typedef struct {
    volatile uint32_t RESERVED0[18]; /**< 0x00-0x44: Other clock registers */
    volatile uint32_t CLK_PERI_CTRL; /**< 0x48: Peripheral clock control */
} clocks_hw_t;

/**
 * CLOCKS base address and pointer.
 */
#define CLOCKS_BASE 0x40010000U
#define CLOCKS      ((clocks_hw_t *)CLOCKS_BASE)

/**
 * Clock control bit definitions.
 */
#define CLOCKS_CLK_PERI_CTRL_ENABLE_SHIFT 11U
#define CLOCKS_CLK_PERI_CTRL_AUXSRC_SHIFT 5U
#define CLOCKS_CLK_PERI_CTRL_AUXSRC_XOSC  4U

/**
 * RESETS Register Structure.
 */
typedef struct {
    volatile uint32_t RESET;      /**< 0x00: Reset control */
    volatile uint32_t WDSEL;      /**< 0x04: Watchdog select */
    volatile uint32_t RESET_DONE; /**< 0x08: Reset done status */
} resets_hw_t;

/**
 * RESETS base address and pointer.
 */
#define RESETS_BASE       0x40020000U
#define RESETS             ((resets_hw_t *)RESETS_BASE)
#define RESETS_RESET_CLEAR ((volatile uint32_t *)(RESETS_BASE + 0x3000U))

/**
 * Reset control bit definitions.
 */
#define RESETS_RESET_IO_BANK0_SHIFT 6U
#define RESETS_RESET_UART0_SHIFT    26U

/**
 * IO_BANK0 GPIO Control Register (one per GPIO).
 */
typedef struct {
    volatile uint32_t STATUS; /**< 0x00: GPIO status */
    volatile uint32_t CTRL;   /**< 0x04: GPIO control */
} io_bank0_gpio_ctrl_t;

/**
 * IO_BANK0 Register Structure.
 */
typedef struct {
    io_bank0_gpio_ctrl_t GPIO[30]; /**< 0x000-0x0E8: GPIO 0-29 */
} io_bank0_hw_t;

/**
 * IO_BANK0 base address and pointer.
 */
#define IO_BANK0_BASE 0x40028000U
#define IO_BANK0      ((io_bank0_hw_t *)IO_BANK0_BASE)

/**
 * GPIO control bit definitions.
 */
#define IO_BANK0_CTRL_FUNCSEL_MASK  0x1FU
#define IO_BANK0_CTRL_FUNCSEL_UART  0x02U
#define IO_BANK0_CTRL_FUNCSEL_SIO   0x05U

/**
 * PADS_BANK0 Register Structure.
 */
typedef struct {
    volatile uint32_t VOLTAGE_SELECT; /**< 0x00: Voltage select */
    volatile uint32_t GPIO[30];       /**< 0x04-0x78: GPIO 0-29 pad control */
} pads_bank0_hw_t;

/**
 * PADS_BANK0 base address and pointer.
 */
#define PADS_BANK0_BASE 0x40038000U
#define PADS_BANK0      ((pads_bank0_hw_t *)PADS_BANK0_BASE)

/**
 * Pad control bit definitions.
 */
#define PADS_BANK0_OD_SHIFT  7U
#define PADS_BANK0_IE_SHIFT  6U
#define PADS_BANK0_ISO_SHIFT 8U

/**
 * UART0 base address.
 */
#define UART0_BASE 0x40070000U

/**
 * UART register offsets (as word indices from UART0_BASE).
 */
#define UART_DR_OFFSET      (0x00U / 4U)
#define UART_FR_OFFSET      (0x18U / 4U)
#define UART_IBRD_OFFSET    (0x24U / 4U)
#define UART_FBRD_OFFSET    (0x28U / 4U)
#define UART_LCR_H_OFFSET  (0x2CU / 4U)
#define UART_CR_OFFSET      (0x30U / 4U)

/**
 * UART flag register bit definitions.
 */
#define UART_FR_TXFF_MASK 32U
#define UART_FR_RXFE_MASK 16U

/**
 * UART line control and enable values.
 */
#define UART_LCR_H_8N1_FIFO 0x70U
#define UART_CR_ENABLE       ((3U << 8) | 1U)

/**
 * GPIO pin definitions.
 */
#define LED_PIN 25U

/**
 * SIO (Single-cycle IO) base address.
 */
#define SIO_BASE 0xD0000000U

/**
 * SIO GPIO output register offsets (as word indices).
 */
#define SIO_GPIO_OUT_SET_OFFSET    (0x018U / 4U)
#define SIO_GPIO_OUT_CLR_OFFSET    (0x020U / 4U)
#define SIO_GPIO_OUT_XOR_OFFSET    (0x028U / 4U)
#define SIO_GPIO_OE_SET_OFFSET     (0x038U / 4U)
#define SIO_GPIO_IN_OFFSET         (0x008U / 4U)

#endif // CONSTANTS_H
