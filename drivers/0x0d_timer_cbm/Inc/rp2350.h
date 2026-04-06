/**
  ******************************************************************************
  * @file    rp2350.h
  * @author  Kevin Thomas
  * @brief   RP2350 Device Peripheral Access Layer Header File.
  *
  *          Memory-mapped register structures and peripheral base addresses
  *          for the RP2350 microcontroller (Cortex-M33 dual-core). All
  *          register offsets verified against the RP2350 datasheet
  *          (RP-008373-DS-2).
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Kevin Thomas.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef __RP2350_H
#define __RP2350_H

#include <stdint.h>
#include <stdbool.h>

/*!< Defines 'read / write' permissions */
#define __IO volatile

/*!< Stack addresses */
#define STACK_TOP              0x20082000UL
#define STACK_LIMIT            0x2007A000UL

/*!< Memory map */
#define XIP_BASE               0x10000000UL
#define SRAM_BASE              0x20000000UL
#define SIO_BASE               0xD0000000UL
#define PPB_BASE               0xE0000000UL

/*!< APB peripherals */
#define CLOCKS_BASE            0x40010000UL
#define RESETS_BASE            0x40020000UL
#define IO_BANK0_BASE          0x40028000UL
#define PADS_BANK0_BASE        0x40038000UL
#define XOSC_BASE              0x40048000UL
#define UART0_BASE             0x40070000UL
#define TICKS_BASE             0x40108000UL
#define TIMER0_BASE            0x400B0000UL

/*!< Atomic register alias offsets */
#define ATOMIC_SET_OFFSET      0x2000UL
#define ATOMIC_CLR_OFFSET      0x3000UL

/**
  * @brief XOSC (External Crystal Oscillator)
  */
typedef struct
{
  __IO uint32_t CTRL;          // Control register                               Address offset: 0x00
  __IO uint32_t STATUS;        // Status register                                Address offset: 0x04
  __IO uint32_t DORMANT;       // Dormant mode                                   Address offset: 0x08
  __IO uint32_t STARTUP;       // Startup delay                                  Address offset: 0x0C
  __IO uint32_t COUNT;         // Frequency count                                Address offset: 0x10
} XOSC_TypeDef;

/**
  * @brief CLOCKS
  */
typedef struct
{
  __IO uint32_t RESERVED0[12]; // GPOUT0..GPOUT3 registers                       Address offset: 0x00-0x2C
  __IO uint32_t CLK_REF_CTRL;  // Reference clock control                        Address offset: 0x30
  __IO uint32_t RESERVED1[5];  // CLK_REF_DIV..CLK_SYS_SELECTED                  Address offset: 0x34-0x44
  __IO uint32_t CLK_PERI_CTRL; // Peripheral clock control                       Address offset: 0x48
} CLOCKS_TypeDef;

/**
  * @brief RESETS
  */
typedef struct
{
  __IO uint32_t RESET;         // Reset control                                  Address offset: 0x00
  __IO uint32_t WDSEL;         // Watchdog select                                Address offset: 0x04
  __IO uint32_t RESET_DONE;    // Reset done status                              Address offset: 0x08
} RESETS_TypeDef;

/**
  * @brief IO_BANK0 GPIO Control (one per GPIO)
  */
typedef struct
{
  __IO uint32_t STATUS;        // GPIO status                                    Address offset: 0x00
  __IO uint32_t CTRL;          // GPIO control                                   Address offset: 0x04
} IO_BANK0_GPIO_TypeDef;

/**
  * @brief IO_BANK0
  */
typedef struct
{
  IO_BANK0_GPIO_TypeDef GPIO[30]; // GPIO 0-29 status/ctrl pairs                 Address offset: 0x000-0x0E8
} IO_BANK0_TypeDef;

/**
  * @brief PADS_BANK0
  */
typedef struct
{
  __IO uint32_t VOLTAGE_SELECT; // Voltage select                                Address offset: 0x00
  __IO uint32_t GPIO[30];      // GPIO 0-29 pad control                          Address offset: 0x04-0x78
} PADS_BANK0_TypeDef;

/**
  * @brief TIMER0 (System Timer)
  */
typedef struct
{
  __IO uint32_t TIMEHW;        // Write to bits 63:32 of time (no side effects)  Address offset: 0x00
  __IO uint32_t TIMELW;        // Write to bits 31:0 of time (triggers latch)    Address offset: 0x04
  __IO uint32_t TIMEHR;        // Read bits 63:32 of time (latched on TIMELR)    Address offset: 0x08
  __IO uint32_t TIMELR;        // Read bits 31:0 of time (latches TIMEHR)        Address offset: 0x0C
  __IO uint32_t ALARM0;        // Alarm 0 target (arms on write)                 Address offset: 0x10
  __IO uint32_t ALARM1;        // Alarm 1 target (arms on write)                 Address offset: 0x14
  __IO uint32_t ALARM2;        // Alarm 2 target (arms on write)                 Address offset: 0x18
  __IO uint32_t ALARM3;        // Alarm 3 target (arms on write)                 Address offset: 0x1C
  __IO uint32_t ARMED;         // Armed status / disarm by writing 1             Address offset: 0x20
  __IO uint32_t TIMERAWH;      // Raw read bits 63:32 (no latch)                 Address offset: 0x24
  __IO uint32_t TIMERAWL;      // Raw read bits 31:0 (no latch)                  Address offset: 0x28
  __IO uint32_t DBGPAUSE;      // Debug pause control                            Address offset: 0x2C
  __IO uint32_t PAUSE;         // Pause timer                                    Address offset: 0x30
  __IO uint32_t LOCKED;        // Lock writes to timer                           Address offset: 0x34
  __IO uint32_t SOURCE;        // Clock source select                            Address offset: 0x38
  __IO uint32_t INTR;          // Raw interrupts                                 Address offset: 0x3C
  __IO uint32_t INTE;          // Interrupt enable                               Address offset: 0x40
  __IO uint32_t INTF;          // Interrupt force                                Address offset: 0x44
  __IO uint32_t INTS;          // Interrupt status after masking & forcing        Address offset: 0x48
} TIMER_TypeDef;

/**
  * @brief TICKS (Tick Generator entry, one per source)
  */
typedef struct
{
  __IO uint32_t CTRL;          // Enable tick generator (bit 0)                  Address offset: 0x00
  __IO uint32_t CYCLES;        // Number of CLK_REF cycles per tick (bits 8:0)   Address offset: 0x04
  __IO uint32_t COUNT;         // Running counter of ticks (read-only)           Address offset: 0x08
} TICKS_Entry_TypeDef;

/**
  * @brief Peripheral Definitions
  */
#define XOSC                   ((XOSC_TypeDef *) XOSC_BASE)
#define CLOCKS                 ((CLOCKS_TypeDef *) CLOCKS_BASE)
#define RESETS                 ((RESETS_TypeDef *) RESETS_BASE)
#define IO_BANK0               ((IO_BANK0_TypeDef *) IO_BANK0_BASE)
#define PADS_BANK0             ((PADS_BANK0_TypeDef *) PADS_BANK0_BASE)
#define SIO                    ((volatile uint32_t *) SIO_BASE)
#define CPACR                  ((volatile uint32_t *) (PPB_BASE + 0x0ED88UL))
#define TIMER0                 ((TIMER_TypeDef *) TIMER0_BASE)

/**
  * @brief TICKS entries (indexed from TICKS_BASE, 12-byte stride)
  */
#define TICKS_TIMER0           ((TICKS_Entry_TypeDef *) (TICKS_BASE + 0x18UL))

/**
  * @brief NVIC registers
  */
#define NVIC_ISER0             ((volatile uint32_t *) (PPB_BASE + 0x0E100UL))

/**
  * @brief XOSC bit definitions
  */
#define XOSC_STATUS_STABLE_SHIFT     31U

/**
  * @brief CPACR bit definitions
  */
#define CPACR_CP0_SHIFT              0U
#define CPACR_CP1_SHIFT              1U

/**
  * @brief CLK_REF bit definitions
  */
#define CLK_REF_CTRL_SRC_XOSC       2U

/**
  * @brief CLOCKS bit definitions
  */
#define CLK_PERI_CTRL_ENABLE_SHIFT   11U
#define CLK_PERI_CTRL_AUXSRC_SHIFT   5U
#define CLK_PERI_CTRL_AUXSRC_MASK   (0x07U << CLK_PERI_CTRL_AUXSRC_SHIFT)
#define CLK_PERI_CTRL_AUXSRC_XOSC   4U

/**
  * @brief RESETS bit definitions
  */
#define RESETS_RESET_IO_BANK0_SHIFT  6U
#define RESETS_RESET_PADS_BANK0_SHIFT 9U
#define RESETS_RESET_UART0_SHIFT     26U
#define RESETS_RESET_TIMER0_SHIFT    23U

/**
  * @brief IO_BANK0 bit definitions
  */
#define IO_BANK0_CTRL_FUNCSEL_MASK   0x1FU
#define IO_BANK0_CTRL_FUNCSEL_UART   0x02U
#define IO_BANK0_CTRL_FUNCSEL_SIO    0x05U
#define IO_BANK0_CTRL_FUNCSEL_NULL   0x1FU

/**
  * @brief PADS_BANK0 bit definitions
  */
#define PADS_BANK0_OD_SHIFT          7U
#define PADS_BANK0_IE_SHIFT          6U
#define PADS_BANK0_ISO_SHIFT         8U
#define PADS_BANK0_PUE_SHIFT         3U
#define PADS_BANK0_PDE_SHIFT         2U

/**
  * @brief SIO GPIO register offsets (word indices from SIO_BASE)
  */
#define SIO_GPIO_OUT_SET_OFFSET      (0x018U / 4U)
#define SIO_GPIO_OUT_CLR_OFFSET      (0x020U / 4U)
#define SIO_GPIO_OE_SET_OFFSET       (0x038U / 4U)

/**
  * @brief UART register offsets (word indices from UART0_BASE)
  */
#define UART_DR_OFFSET               (0x000U / 4U)
#define UART_FR_OFFSET               (0x018U / 4U)
#define UART_IBRD_OFFSET             (0x024U / 4U)
#define UART_FBRD_OFFSET             (0x028U / 4U)
#define UART_LCR_H_OFFSET           (0x02CU / 4U)
#define UART_CR_OFFSET               (0x030U / 4U)

/**
  * @brief UART flag register bit definitions
  */
#define UART_FR_TXFF_MASK            32U
#define UART_FR_RXFE_MASK            16U

/**
  * @brief UART line control and enable values
  */
#define UART_LCR_H_8N1_FIFO         0x70U
#define UART_CR_ENABLE               ((3U << 8) | 1U)

/**
  * @brief TIMER0 bit definitions
  */
#define TIMER0_ALARM0_IRQ            0U
#define TIMER_INTE_ALARM0_SHIFT      0U
#define TIMER_INTR_ALARM0_MASK       (1U << 0)

/**
  * @brief TICKS bit definitions
  */
#define TICKS_CTRL_ENABLE_SHIFT      0U
#define TICKS_TIMER0_CYCLES_12MHZ    12U

#endif /* __RP2350_H */
