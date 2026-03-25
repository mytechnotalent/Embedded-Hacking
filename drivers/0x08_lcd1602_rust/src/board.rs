//! @file board.rs
//! @brief Board-level initialisation and LCD hardware helpers
//! @author Kevin Thomas
//! @date 2025
//!
//! MIT License
//!
//! Copyright (c) 2025 Kevin Thomas
//!
//! Permission is hereby granted, free of charge, to any person obtaining a copy
//! of this software and associated documentation files (the "Software"), to deal
//! in the Software without restriction, including without limitation the rights
//! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//! copies of the Software, and to permit persons to whom the Software is
//! furnished to do so, subject to the following conditions:
//!
//! The above copyright notice and this permission notice shall be included in
//! all copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.

use embedded_hal::i2c::I2c;
use fugit::RateExtU32;
use hal::Clock;
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

#[cfg(rp2350)]
use rp235x_hal as hal;

#[cfg(rp2040)]
use rp2040_hal as hal;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// I2C bus speed in Hz (100 kHz standard mode).
pub(crate) const I2C_BAUD: u32 = 100_000;

/// 7-bit I2C address of the PCF8574 LCD backpack.
pub(crate) const LCD_I2C_ADDR: u8 = 0x27;

/// Number of bit positions to shift a 4-bit nibble.
pub(crate) const NIBBLE_SHIFT: u8 = 4;

/// PCF8574 backlight enable mask.
pub(crate) const BACKLIGHT_MASK: u8 = 0x08;

/// Delay between counter updates in milliseconds.
pub(crate) const COUNTER_DELAY_MS: u32 = 1_000;

/// Type alias for the configured TX pin (GPIO 0, UART function, no pull).
pub(crate) type TxPin = Pin<hal::gpio::bank0::Gpio0, FunctionUart, PullNone>;

/// Type alias for the configured RX pin (GPIO 1, UART function, no pull).
pub(crate) type RxPin = Pin<hal::gpio::bank0::Gpio1, FunctionUart, PullNone>;

/// Type alias for the default TX pin state from `Pins::new()`.
pub(crate) type TxPinDefault = Pin<hal::gpio::bank0::Gpio0, FunctionNull, PullDown>;

/// Type alias for the default RX pin state from `Pins::new()`.
pub(crate) type RxPinDefault = Pin<hal::gpio::bank0::Gpio1, FunctionNull, PullDown>;

/// Type alias for the fully-enabled UART0 peripheral with TX/RX pins.
pub(crate) type EnabledUart = UartPeripheral<Enabled, hal::pac::UART0, (TxPin, RxPin)>;

/// Initialise system clocks and PLLs from the external 12 MHz crystal.
///
/// # Arguments
///
/// * `xosc` - XOSC peripheral singleton.
/// * `clocks` - CLOCKS peripheral singleton.
/// * `pll_sys` - PLL_SYS peripheral singleton.
/// * `pll_usb` - PLL_USB peripheral singleton.
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `watchdog` - Mutable reference to the watchdog timer.
///
/// # Returns
///
/// Configured clocks manager.
///
/// # Panics
///
/// Panics if clock initialisation fails.
pub(crate) fn init_clocks(
    xosc: hal::pac::XOSC,
    clocks: hal::pac::CLOCKS,
    pll_sys: hal::pac::PLL_SYS,
    pll_usb: hal::pac::PLL_USB,
    resets: &mut hal::pac::RESETS,
    watchdog: &mut hal::Watchdog,
) -> hal::clocks::ClocksManager {
    hal::clocks::init_clocks_and_plls(
        XTAL_FREQ_HZ, xosc, clocks, pll_sys, pll_usb, resets, watchdog,
    )
    .unwrap()
}

/// Unlock the GPIO bank and return the pin set.
///
/// # Arguments
///
/// * `io_bank0` - IO_BANK0 peripheral singleton.
/// * `pads_bank0` - PADS_BANK0 peripheral singleton.
/// * `sio` - SIO peripheral singleton.
/// * `resets` - Mutable reference to the RESETS peripheral.
///
/// # Returns
///
/// GPIO pin set for the entire bank.
pub(crate) fn init_pins(
    io_bank0: hal::pac::IO_BANK0,
    pads_bank0: hal::pac::PADS_BANK0,
    sio: hal::pac::SIO,
    resets: &mut hal::pac::RESETS,
) -> hal::gpio::Pins {
    let sio = hal::Sio::new(sio);
    hal::gpio::Pins::new(io_bank0, pads_bank0, sio.gpio_bank0, resets)
}

/// Initialise UART0 for serial output (stdio equivalent).
///
/// # Arguments
///
/// * `uart0` - PAC UART0 peripheral singleton.
/// * `tx_pin` - GPIO pin to use as UART0 TX (GPIO 0).
/// * `rx_pin` - GPIO pin to use as UART0 RX (GPIO 1).
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `clocks` - Reference to the initialised clock configuration.
///
/// # Returns
///
/// Enabled UART0 peripheral ready for blocking writes.
///
/// # Panics
///
/// Panics if the HAL cannot achieve the requested baud rate.
pub(crate) fn init_uart(
    uart0: hal::pac::UART0,
    tx_pin: TxPinDefault,
    rx_pin: RxPinDefault,
    resets: &mut hal::pac::RESETS,
    clocks: &hal::clocks::ClocksManager,
) -> EnabledUart {
    let pins = (
        tx_pin.reconfigure::<FunctionUart, PullNone>(),
        rx_pin.reconfigure::<FunctionUart, PullNone>(),
    );
    let cfg = UartConfig::new(UART_BAUD.Hz(), DataBits::Eight, None, StopBits::One);
    UartPeripheral::new(uart0, pins, resets)
        .enable(cfg, clocks.peripheral_clock.freq())
        .unwrap()
}

/// Create a blocking delay timer from the ARM SysTick peripheral.
///
/// # Arguments
///
/// * `clocks` - Reference to the initialised clock configuration.
///
/// # Returns
///
/// Blocking delay provider.
///
/// # Panics
///
/// Panics if the cortex-m core peripherals have already been taken.
pub(crate) fn init_delay(clocks: &hal::clocks::ClocksManager) -> cortex_m::delay::Delay {
    let core = cortex_m::Peripherals::take().unwrap();
    cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz())
}

/// Write one raw byte to the PCF8574 expander over I2C.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `data` - Byte to write.
fn pcf_write_byte(i2c: &mut impl I2c, addr: u8, data: u8) {
    let _ = i2c.write(addr, &[data]);
}

/// Toggle EN to latch a nibble into the LCD controller.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `data` - Nibble byte without EN asserted.
/// * `delay` - Delay provider for timing.
fn pcf_pulse_enable(i2c: &mut impl I2c, addr: u8, data: u8, delay: &mut cortex_m::delay::Delay) {
    pcf_write_byte(i2c, addr, crate::lcd1602::nibble_with_en(data));
    delay.delay_us(1);
    pcf_write_byte(i2c, addr, crate::lcd1602::nibble_without_en(data));
    delay.delay_us(50);
}

/// Write one 4-bit nibble to the LCD.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `nibble` - 4-bit value to send.
/// * `mode` - Register select: 0 for command, 1 for data.
/// * `delay` - Delay provider for timing.
fn lcd_write4(i2c: &mut impl I2c, addr: u8, nibble: u8, mode: u8, delay: &mut cortex_m::delay::Delay) {
    let data = crate::lcd1602::build_nibble(nibble, NIBBLE_SHIFT, mode, BACKLIGHT_MASK);
    pcf_pulse_enable(i2c, addr, data, delay);
}

/// Send one full 8-bit command/data value as two nibbles.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `value` - 8-bit value to send.
/// * `mode` - Register select: 0 for command, 1 for data.
/// * `delay` - Delay provider for timing.
fn lcd_send(i2c: &mut impl I2c, addr: u8, value: u8, mode: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_write4(i2c, addr, (value >> 4) & 0x0F, mode, delay);
    lcd_write4(i2c, addr, value & 0x0F, mode, delay);
}

/// Execute the HD44780 4-bit mode power-on reset sequence.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `delay` - Delay provider for timing.
fn lcd_hd44780_reset(i2c: &mut impl I2c, addr: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_write4(i2c, addr, 0x03, 0, delay);
    delay.delay_ms(5);
    lcd_write4(i2c, addr, 0x03, 0, delay);
    delay.delay_us(150);
    lcd_write4(i2c, addr, 0x03, 0, delay);
    delay.delay_us(150);
    lcd_write4(i2c, addr, 0x02, 0, delay);
    delay.delay_us(150);
}

/// Send post-reset configuration commands to the HD44780.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `delay` - Delay provider for timing.
fn lcd_hd44780_configure(i2c: &mut impl I2c, addr: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_send(i2c, addr, 0x28, 0, delay);
    lcd_send(i2c, addr, 0x0C, 0, delay);
    lcd_send(i2c, addr, 0x01, 0, delay);
    delay.delay_ms(2);
    lcd_send(i2c, addr, 0x06, 0, delay);
}

/// Set the LCD cursor position.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `line` - Display row (0 or 1).
/// * `position` - Column offset.
/// * `delay` - Delay provider for timing.
fn lcd_set_cursor(i2c: &mut impl I2c, addr: u8, line: u8, position: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_send(i2c, addr, crate::lcd1602::cursor_address(line, position), 0, delay);
}

/// Write a byte slice as character data to the LCD.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address of the PCF8574.
/// * `s` - Byte slice of ASCII characters to display.
/// * `delay` - Delay provider for timing.
fn lcd_puts(i2c: &mut impl I2c, addr: u8, s: &[u8], delay: &mut cortex_m::delay::Delay) {
    for &ch in s {
        lcd_send(i2c, addr, ch, 1, delay);
    }
}

/// Initialize the LCD, display the title, and log over UART.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `uart` - UART peripheral for serial log output.
/// * `delay` - Delay provider for timing.
pub(crate) fn setup_display(
    i2c: &mut impl I2c,
    uart: &EnabledUart,
    delay: &mut cortex_m::delay::Delay,
) {
    lcd_hd44780_reset(i2c, LCD_I2C_ADDR, delay);
    lcd_hd44780_configure(i2c, LCD_I2C_ADDR, delay);
    lcd_set_cursor(i2c, LCD_I2C_ADDR, 0, 0, delay);
    lcd_puts(i2c, LCD_I2C_ADDR, b"Reverse Eng.", delay);
    uart.write_full_blocking(b"LCD 1602 driver initialized at I2C addr 0x27\r\n");
}

/// Format and display the next counter value on LCD line 1.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `uart` - UART peripheral for serial log output.
/// * `delay` - Delay provider for timing.
/// * `count` - Mutable reference to the counter state.
pub(crate) fn update_counter(
    i2c: &mut impl I2c,
    uart: &EnabledUart,
    delay: &mut cortex_m::delay::Delay,
    count: &mut u32,
) {
    let mut buf = [0u8; 16];
    let n = crate::lcd1602::format_counter(&mut buf, *count);
    *count += 1;
    lcd_set_cursor(i2c, LCD_I2C_ADDR, 1, 0, delay);
    lcd_puts(i2c, LCD_I2C_ADDR, &buf[..n], delay);
    uart.write_full_blocking(&buf[..n]);
    uart.write_full_blocking(b"\r\n");
    delay.delay_ms(COUNTER_DELAY_MS);
}

// End of file
