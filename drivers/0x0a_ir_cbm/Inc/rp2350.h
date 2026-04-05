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
#define TIMER0_BASE            0x400B0000UL
#define TICKS_BASE             0x40108000UL

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
  * @brief Peripheral Definitions
  */
#define XOSC                   ((XOSC_TypeDef *) XOSC_BASE)
#define CLOCKS                 ((CLOCKS_TypeDef *) CLOCKS_BASE)
#define RESETS                 ((RESETS_TypeDef *) RESETS_BASE)
#define IO_BANK0               ((IO_BANK0_TypeDef *) IO_BANK0_BASE)
#define PADS_BANK0             ((PADS_BANK0_TypeDef *) PADS_BANK0_BASE)
#define SIO                    ((volatile uint32_t *) SIO_BASE)
#define TIMER0                 ((volatile uint32_t *) TIMER0_BASE)
#define TICKS                  ((volatile uint32_t *) TICKS_BASE)
#define CPACR                  ((volatile uint32_t *) (PPB_BASE + 0x0ED88UL))

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
#define RESETS_RESET_TIMER0_SHIFT    23U
#define RESETS_RESET_UART0_SHIFT     26U

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

/**
  * @brief SIO GPIO register offsets (word indices from SIO_BASE)
  */
#define SIO_GPIO_IN_OFFSET           (0x004U / 4U)
#define SIO_GPIO_OUT_SET_OFFSET      (0x018U / 4U)
#define SIO_GPIO_OUT_CLR_OFFSET      (0x020U / 4U)
#define SIO_GPIO_OE_SET_OFFSET       (0x038U / 4U)
#define SIO_GPIO_OE_CLR_OFFSET       (0x040U / 4U)

/**
  * @brief TIMER0 register offsets (word indices from TIMER0_BASE)
  */
#define TIMER_TIMERAWL_OFFSET        (0x028U / 4U)

/**
  * @brief Tick generator register offsets (word indices from TICKS_BASE)
  */
#define TICKS_TIMER0_CTRL_OFFSET     (0x018U / 4U)
#define TICKS_TIMER0_CYCLES_OFFSET   (0x01CU / 4U)
#define TICKS_TIMER0_ENABLE          1U
#define TICKS_TIMER0_CYCLES_12MHZ    12U

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
  * @brief NEC IR receiver GPIO pin definition
  */
#define IR_PIN                       5U

/**
  * @brief NEC protocol timing thresholds (microseconds)
  */
#define NEC_LEADER_WAIT_US           150000U
#define NEC_LEADER_MARK_MIN_US       8000U
#define NEC_LEADER_MARK_MAX_US       10000U
#define NEC_LEADER_MARK_TIMEOUT_US   12000U
#define NEC_LEADER_SPACE_MIN_US      3500U
#define NEC_LEADER_SPACE_MAX_US      5000U
#define NEC_LEADER_SPACE_TIMEOUT_US  7000U
#define NEC_BIT_MARK_TIMEOUT_US      1000U
#define NEC_BIT_SPACE_TIMEOUT_US     2500U
#define NEC_BIT_SPACE_MIN_US         200U
#define NEC_BIT_ONE_THRESHOLD_US     1200U

/**
  * @brief NEC frame data format
  */
#define NEC_DATA_BYTES               4U
#define NEC_DATA_BITS                32U

#endif /* __RP2350_H */
