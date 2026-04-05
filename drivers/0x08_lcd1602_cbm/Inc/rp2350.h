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
#define I2C1_BASE              0x40098000UL

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
  * @brief I2C (Synopsys DW APB I2C Controller)
  */
typedef struct
{
  __IO uint32_t CON;           // Control register                               Address offset: 0x00
  __IO uint32_t TAR;           // Target address                                 Address offset: 0x04
  __IO uint32_t SAR;           // Slave address                                  Address offset: 0x08
  __IO uint32_t RESERVED0;     // Padding                                        Address offset: 0x0C
  __IO uint32_t DATA_CMD;      // Rx/Tx data buffer and command                  Address offset: 0x10
  __IO uint32_t SS_SCL_HCNT;   // Standard speed SCL high count                  Address offset: 0x14
  __IO uint32_t SS_SCL_LCNT;   // Standard speed SCL low count                   Address offset: 0x18
  __IO uint32_t FS_SCL_HCNT;   // Fast mode SCL high count                       Address offset: 0x1C
  __IO uint32_t FS_SCL_LCNT;   // Fast mode SCL low count                        Address offset: 0x20
  __IO uint32_t RESERVED1[2];  // Padding                                        Address offset: 0x24-0x28
  __IO uint32_t INTR_STAT;     // Interrupt status (RO)                          Address offset: 0x2C
  __IO uint32_t INTR_MASK;     // Interrupt mask                                 Address offset: 0x30
  __IO uint32_t RAW_INTR_STAT; // Raw interrupt status (RO)                      Address offset: 0x34
  __IO uint32_t RX_TL;         // Receive FIFO threshold level                   Address offset: 0x38
  __IO uint32_t TX_TL;         // Transmit FIFO threshold level                  Address offset: 0x3C
  __IO uint32_t CLR_INTR;      // Clear combined interrupt (RO)                  Address offset: 0x40
  __IO uint32_t CLR_RX_UNDER;  // Clear RX_UNDER interrupt (RO)                  Address offset: 0x44
  __IO uint32_t CLR_RX_OVER;   // Clear RX_OVER interrupt (RO)                   Address offset: 0x48
  __IO uint32_t CLR_TX_OVER;   // Clear TX_OVER interrupt (RO)                   Address offset: 0x4C
  __IO uint32_t CLR_RD_REQ;    // Clear RD_REQ interrupt (RO)                    Address offset: 0x50
  __IO uint32_t CLR_TX_ABRT;   // Clear TX_ABRT interrupt (RO)                   Address offset: 0x54
  __IO uint32_t CLR_RX_DONE;   // Clear RX_DONE interrupt (RO)                   Address offset: 0x58
  __IO uint32_t CLR_ACTIVITY;  // Clear ACTIVITY interrupt (RO)                  Address offset: 0x5C
  __IO uint32_t CLR_STOP_DET;  // Clear STOP_DET interrupt (RO)                  Address offset: 0x60
  __IO uint32_t CLR_START_DET; // Clear START_DET interrupt (RO)                 Address offset: 0x64
  __IO uint32_t CLR_GEN_CALL;  // Clear GEN_CALL interrupt (RO)                  Address offset: 0x68
  __IO uint32_t ENABLE;        // I2C enable                                     Address offset: 0x6C
  __IO uint32_t STATUS;        // I2C status (RO)                                Address offset: 0x70
  __IO uint32_t TXFLR;         // Transmit FIFO level (RO)                       Address offset: 0x74
  __IO uint32_t RXFLR;         // Receive FIFO level (RO)                        Address offset: 0x78
  __IO uint32_t SDA_HOLD;      // SDA hold time                                  Address offset: 0x7C
  __IO uint32_t TX_ABRT_SRC;   // Transmit abort source (RO)                     Address offset: 0x80
  __IO uint32_t RESERVED2;     // SLV_DATA_NACK_ONLY                             Address offset: 0x84
  __IO uint32_t DMA_CR;        // DMA control                                    Address offset: 0x88
  __IO uint32_t DMA_TDLR;      // DMA transmit data level                        Address offset: 0x8C
  __IO uint32_t DMA_RDLR;      // DMA receive data level                         Address offset: 0x90
  __IO uint32_t SDA_SETUP;     // SDA setup time                                 Address offset: 0x94
  __IO uint32_t ACK_GEN_CALL;  // ACK general call                               Address offset: 0x98
  __IO uint32_t ENABLE_STATUS; // I2C enable status (RO)                         Address offset: 0x9C
  __IO uint32_t FS_SPKLEN;     // Fast mode spike suppression limit              Address offset: 0xA0
} I2C_TypeDef;

/**
  * @brief Peripheral Definitions
  */
#define XOSC                   ((XOSC_TypeDef *) XOSC_BASE)
#define CLOCKS                 ((CLOCKS_TypeDef *) CLOCKS_BASE)
#define RESETS                 ((RESETS_TypeDef *) RESETS_BASE)
#define IO_BANK0               ((IO_BANK0_TypeDef *) IO_BANK0_BASE)
#define PADS_BANK0             ((PADS_BANK0_TypeDef *) PADS_BANK0_BASE)
#define I2C1                   ((I2C_TypeDef *) I2C1_BASE)
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
#define RESETS_RESET_I2C1_SHIFT      5U
#define RESETS_RESET_UART0_SHIFT     26U

/**
  * @brief IO_BANK0 bit definitions
  */
#define IO_BANK0_CTRL_FUNCSEL_MASK   0x1FU
#define IO_BANK0_CTRL_FUNCSEL_UART   0x02U
#define IO_BANK0_CTRL_FUNCSEL_I2C    0x03U
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
#define I2C_SDA_PIN                  2U
#define I2C_SCL_PIN                  3U

/**
  * @brief I2C CON register bit definitions
  */
#define I2C_CON_MASTER_MODE_SHIFT    0U
#define I2C_CON_SPEED_SHIFT          1U
#define I2C_CON_SPEED_FAST           2U
#define I2C_CON_IC_RESTART_EN_SHIFT  5U
#define I2C_CON_IC_SLAVE_DISABLE_SHIFT 6U
#define I2C_CON_TX_EMPTY_CTRL_SHIFT  8U

/**
  * @brief I2C DATA_CMD register bit definitions
  */
#define I2C_DATA_CMD_CMD_SHIFT       8U
#define I2C_DATA_CMD_STOP_SHIFT      9U
#define I2C_DATA_CMD_RESTART_SHIFT   10U

/**
  * @brief I2C RAW_INTR_STAT register bit masks
  */
#define I2C_RAW_INTR_TX_EMPTY       (1U << 4)
#define I2C_RAW_INTR_TX_ABRT        (1U << 6)
#define I2C_RAW_INTR_STOP_DET       (1U << 9)

/**
  * @brief I2C TX_ABRT_SOURCE bit definitions
  */
#define I2C_TX_ABRT_7B_ADDR_NOACK   (1U << 0)

/**
  * @brief I2C STATUS register bit definitions
  */
#define I2C_STATUS_TFNF_SHIFT       1U
#define I2C_STATUS_RFNE_SHIFT       3U

/**
  * @brief I2C clock timing for 100 kHz at 12 MHz clk_sys
  */
#define I2C_SYS_CLK_HZ              12000000U
#define I2C_BAUD_HZ                 100000U
#define I2C_FS_SCL_HCNT_VAL         48U
#define I2C_FS_SCL_LCNT_VAL         72U
#define I2C_SDA_TX_HOLD_VAL         4U
#define I2C_FS_SPKLEN_VAL           4U

/**
  * @brief I2C timeout
  */
#define I2C_TIMEOUT                  1000000U

/**
  * @brief LCD1602 PCF8574 I2C address
  */
#define LCD_I2C_ADDR                 0x27U

/**
  * @brief PCF8574 -> HD44780 control pin mappings
  */
#define LCD_PIN_RS                   0x01U
#define LCD_PIN_RW                   0x02U
#define LCD_PIN_EN                   0x04U
#define LCD_BACKLIGHT                0x08U
#define LCD_NIBBLE_SHIFT             4U

/**
  * @brief HD44780 command values
  */
#define LCD_CMD_CLEAR                0x01U
#define LCD_CMD_FUNCTION_SET_4BIT    0x28U
#define LCD_CMD_DISPLAY_ON           0x0CU
#define LCD_CMD_ENTRY_MODE           0x06U
#define LCD_CMD_SET_DDRAM            0x80U

/**
  * @brief HD44780 DDRAM row offsets
  */
#define LCD_ROW0_OFFSET              0x00U
#define LCD_ROW1_OFFSET              0x40U

#endif /* __RP2350_H */
