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
#define ADC_BASE               0x400A0000UL

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
  __IO uint32_t RESERVED0[18]; // Other clock registers                          Address offset: 0x00-0x44
  __IO uint32_t CLK_PERI_CTRL; // Peripheral clock control                       Address offset: 0x48
  __IO uint32_t RESERVED1[8];  // CLK_PERI_DIV..CLK_USB_SELECTED                 Address offset: 0x4C-0x68
  __IO uint32_t CLK_ADC_CTRL;  // ADC clock control                              Address offset: 0x6C
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
  * @brief ADC (Analog-to-Digital Converter)
  */
typedef struct
{
  __IO uint32_t CS;            // Control and status                             Address offset: 0x00
  __IO uint32_t RESULT;        // Conversion result                              Address offset: 0x04
} ADC_TypeDef;

/**
  * @brief Peripheral Definitions
  */
#define XOSC                   ((XOSC_TypeDef *) XOSC_BASE)
#define CLOCKS                 ((CLOCKS_TypeDef *) CLOCKS_BASE)
#define RESETS                 ((RESETS_TypeDef *) RESETS_BASE)
#define IO_BANK0               ((IO_BANK0_TypeDef *) IO_BANK0_BASE)
#define PADS_BANK0             ((PADS_BANK0_TypeDef *) PADS_BANK0_BASE)
#define ADC                    ((ADC_TypeDef *) ADC_BASE)
#define SIO                    ((volatile uint32_t *) SIO_BASE)
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
  * @brief CLOCKS bit definitions
  */
#define CLK_PERI_CTRL_ENABLE_SHIFT   11U
#define CLK_PERI_CTRL_AUXSRC_SHIFT   5U
#define CLK_PERI_CTRL_AUXSRC_MASK   (0x07U << CLK_PERI_CTRL_AUXSRC_SHIFT)
#define CLK_PERI_CTRL_AUXSRC_XOSC   4U

/**
  * @brief CLK_ADC bit definitions
  */
#define CLK_ADC_CTRL_ENABLE_SHIFT    11U
#define CLK_ADC_CTRL_AUXSRC_SHIFT    5U
#define CLK_ADC_CTRL_AUXSRC_MASK    (0x07U << CLK_ADC_CTRL_AUXSRC_SHIFT)
#define CLK_ADC_CTRL_AUXSRC_XOSC     3U

/**
  * @brief RESETS bit definitions
  */
#define RESETS_RESET_IO_BANK0_SHIFT  6U
#define RESETS_RESET_ADC_SHIFT       0U
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
  * @brief GPIO pin definitions
  */
#define ADC_PIN                      26U
#define ADC_CHANNEL                  0U
#define ADC_TEMP_CHANNEL             4U

/**
  * @brief PADS_BANK0 drive strength value
  */
#define PADS_BANK0_DRIVE_4MA         (1U << 4)

/**
  * @brief ADC CS register bit definitions
  */
#define ADC_CS_EN_SHIFT              0U
#define ADC_CS_TS_EN_SHIFT           1U
#define ADC_CS_START_ONCE_SHIFT      2U
#define ADC_CS_READY_SHIFT           8U
#define ADC_CS_AINSEL_SHIFT          12U
#define ADC_CS_AINSEL_MASK           (0x0FU << ADC_CS_AINSEL_SHIFT)

/**
  * @brief ADC conversion constants
  */
#define ADC_VREF_MV                  3300U
#define ADC_FULL_SCALE               4095U
#define ADC_READY_TIMEOUT            1000000U

#endif /* __RP2350_H */
