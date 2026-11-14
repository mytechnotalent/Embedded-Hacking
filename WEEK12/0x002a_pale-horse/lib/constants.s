/**
 * FILE: constants.s
 *
 * DESCRIPTION:
 * RP2350 Memory Addresses and Constants.
 *
 * BRIEF:
 * Defines all memory-mapped register addresses, board pin assignments and
 * PALE HORSE stage boundaries used throughout the driver.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.syntax unified                                  // use unified assembly syntax
.cpu cortex-m33                                  // target Cortex-M33 core
.thumb                                           // use Thumb instruction set

/**
 * Memory addresses and constants.
 */
.equ STACK_TOP,                   0x20082000    // top of stack (grows down)
.equ STACK_LIMIT,                 0x2007a000    // stack guard limit
.equ XOSC_BASE,                   0x40048000    // XOSC peripheral base
.equ XOSC_CTRL,                   XOSC_BASE + 0x00 // XOSC control
.equ XOSC_STATUS,                 XOSC_BASE + 0x04 // XOSC status
.equ XOSC_STARTUP,                XOSC_BASE + 0x0c // XOSC startup delay
.equ PPB_BASE,                    0xe0000000    // private peripheral bus
.equ CPACR,                       PPB_BASE + 0x0ed88 // coprocessor access control
.equ CLOCKS_BASE,                 0x40010000    // clocks peripheral base
.equ CLK_PERI_CTRL,               CLOCKS_BASE + 0x48 // peripheral clock control
.equ RESETS_BASE,                 0x40020000    // reset controller base
.equ RESETS_RESET,                RESETS_BASE + 0x0 // reset assert register
.equ RESETS_RESET_CLEAR,          RESETS_BASE + 0x3000 // reset clear (write-1)
.equ RESETS_RESET_DONE,           RESETS_BASE + 0x8 // reset done status
.equ IO_BANK0_BASE,               0x40028000    // GPIO function select base
.equ PADS_BANK0_BASE,             0x40038000    // GPIO pad control base
.equ SIO_BASE,                    0xd0000000    // single-cycle I/O base
.equ SIO_GPIO_IN,                 0x0004        // GPIO input register
.equ SIO_GPIO_OUT,                0x0010        // GPIO output register
.equ SIO_GPIO_OUT_SET,            0x0014        // GPIO output set (write-1)
.equ SIO_GPIO_OE,                 0x0020        // GPIO output enable register
.equ UART0_BASE,                  0x40070000    // UART0 (PL011) base
.equ TIMER0_BASE,                 0x400b0000    // TIMER0 base (LFSR seed)

/**
 * Board pin assignments (kept identical to Weeks 4-11).
 */
.equ LED1,                        16            // red LED
.equ LED2,                        17            // green LED
.equ LED3,                        18            // yellow LED
.equ BTN,                         15            // push button (pull-up)
.equ SERVO,                       6             // SG90 servo signal (PWM)
.equ I2C_SDA,                     2             // 1602 LCD SDA (I2C1)
.equ I2C_SCL,                     3             // 1602 LCD SCL (I2C1)
.equ DHT11,                       4             // DHT11 data (one-wire)

/**
 * PALE HORSE stage boundaries.
 */
.equ STAGE2_BASE,                 0x10000000    // stage-2 vector table
.equ APP_BASE,                    0x10001000    // stage-3 application
.equ MANIFEST_BASE,               0x1003f000    // manifest sector
.equ GHOST_RAM,                   0x20040000    // decrypted ghost stage (SRAM)
.equ VTOR_RAM,                    0x20000000    // relocated vector table (SRAM)
.equ GHOST_MAGIC,                 0x50414c45    // "PALE"

/**
 * Stage-2 loader constants.
 */
.equ WATCHDOG_BASE,               0x400d8000    // watchdog (scratch registers)
.equ WATCHDOG_SCRATCH0,           WATCHDOG_BASE + 0x0c // boot reason / cookie
.equ WATCHDOG_SCRATCH1,           WATCHDOG_BASE + 0x10 // last stage marker
.equ BOOT_COOKIE_OK,              0x600dbeef    // clean boot
.equ BOOT_COOKIE_TAMPER,          0xbadf00d0    // tamper detected
.equ BOOT_COOKIE_INPROGRESS,      0x1a2b3c4d    // loader running
.equ APP_SIZE,                    0x00004000    // bytes covered by the CRC
.equ APP_CRC_EXPECTED,            0x00000000    // patched after app build
.equ GHOST_BLOB,                  0x1003e000    // encrypted ghost blob in flash
.equ GHOST_SIZE,                  0x00001000    // ghost size in bytes
.equ GHOST_END,                   GHOST_RAM + GHOST_SIZE // end of ghost region
.equ GHOST_KEY,                   0x50414c45    // ghost keystream seed ("PALE")
.equ BRICK_MARKER,                0xdeadb001    // written over the manifest
.equ CRC32_POLY,                  0xedb88320    // reflected CRC-32 polynomial

/**
 * PWM (servo) and I2C (LCD) registers.
 */
.equ PWM_BASE,                    0x400a8000    // PWM peripheral base
.equ PWM_CH_CSR,                  0x00          // channel control/status
.equ PWM_CH_DIV,                  0x04          // channel clock divider
.equ PWM_CH_CTR,                  0x08          // channel counter
.equ PWM_CH_CC,                   0x0c          // channel counter compare
.equ PWM_EN,                      0xa0          // PWM enable register
.equ PWM_SERVO_SLICE,             3             // GP6 -> PWM3 channel A
.equ SERVO_DIV,                   0x00040000    // int div 4, frac 0 (~55 Hz wrap)
.equ SERVO_WRAP,                  0xffff        // 16-bit wrap
.equ I2C1_BASE,                   0x40098000    // I2C1 (LCD) base
.equ I2C_CON,                     0x00          // control
.equ I2C_TAR,                     0x04          // target address
.equ I2C_DATA_CMD,                0x10          // data / command
.equ I2C_ENABLE,                  0x6c          // enable
.equ I2C_STATUS,                  0x70          // status
.equ LCD_PCF_ADDR,                0x27          // PCF8574 backpack address
