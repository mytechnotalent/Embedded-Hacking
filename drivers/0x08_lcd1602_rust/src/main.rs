//! @file main.rs
//! @brief HD44780 16x2 LCD (PCF8574 I2C backpack) demonstration
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
//!
//! -----------------------------------------------------------------------------
//!
//! Demonstrates 16x2 HD44780 LCD connected via a PCF8574 I2C backpack using
//! the lcd1602 driver (lcd1602.rs). Line 0 shows a static title and line 1
//! displays a live up-counter that increments every second. The counter
//! value is also printed over UART for debugging.
//!
//! Wiring:
//!   GPIO2 (SDA) -> PCF8574 backpack SDA  (4.7 kohm pull-up to 3.3 V)
//!   GPIO3 (SCL) -> PCF8574 backpack SCL  (4.7 kohm pull-up to 3.3 V)
//!   3.3V or 5V  -> PCF8574 backpack VCC
//!   GND         -> PCF8574 backpack GND

#![no_std]
#![no_main]

#[allow(dead_code)]
mod lcd1602;

use defmt_rtt as _;
#[cfg(target_arch = "riscv32")]
use panic_halt as _;
#[cfg(target_arch = "arm")]
use panic_probe as _;

use embedded_hal::i2c::I2c;
use fugit::RateExtU32;
use hal::entry;
use hal::Clock;
use hal::gpio::{FunctionI2C, FunctionNull, FunctionUart, Pin, PullDown, PullNone, PullUp};
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

#[cfg(rp2350)]
use rp235x_hal as hal;

#[cfg(rp2040)]
use rp2040_hal as hal;

#[unsafe(link_section = ".boot2")]
#[used]
#[cfg(rp2040)]
pub static BOOT2: [u8; 256] = rp2040_boot2::BOOT_LOADER_W25Q080;

#[unsafe(link_section = ".start_block")]
#[used]
#[cfg(rp2350)]
pub static IMAGE_DEF: hal::block::ImageDef = hal::block::ImageDef::secure_exe();

const XTAL_FREQ_HZ: u32 = 12_000_000u32;

const UART_BAUD: u32 = 115_200;
const I2C_BAUD: u32 = 100_000;
const LCD_I2C_ADDR: u8 = 0x27;
const NIBBLE_SHIFT: u8 = 4;
const BACKLIGHT_MASK: u8 = 0x08;
const COUNTER_DELAY_MS: u32 = 1_000;

type TxPin = Pin<hal::gpio::bank0::Gpio0, FunctionUart, PullNone>;
type RxPin = Pin<hal::gpio::bank0::Gpio1, FunctionUart, PullNone>;
type TxPinDefault = Pin<hal::gpio::bank0::Gpio0, FunctionNull, PullDown>;
type RxPinDefault = Pin<hal::gpio::bank0::Gpio1, FunctionNull, PullDown>;
type EnabledUart = UartPeripheral<Enabled, hal::pac::UART0, (TxPin, RxPin)>;

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
fn init_clocks(
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
fn init_pins(
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
fn init_uart(
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
fn init_delay(clocks: &hal::clocks::ClocksManager) -> cortex_m::delay::Delay {
    let core = cortex_m::Peripherals::take().unwrap();
    cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz())
}

/// Write one raw byte to the PCF8574 expander over I2C.
fn pcf_write_byte(i2c: &mut impl I2c, addr: u8, data: u8) {
    let _ = i2c.write(addr, &[data]);
}

/// Toggle EN to latch a nibble into the LCD controller.
fn pcf_pulse_enable(i2c: &mut impl I2c, addr: u8, data: u8, delay: &mut cortex_m::delay::Delay) {
    pcf_write_byte(i2c, addr, lcd1602::nibble_with_en(data));
    delay.delay_us(1);
    pcf_write_byte(i2c, addr, lcd1602::nibble_without_en(data));
    delay.delay_us(50);
}

/// Write one 4-bit nibble to the LCD.
fn lcd_write4(i2c: &mut impl I2c, addr: u8, nibble: u8, mode: u8, delay: &mut cortex_m::delay::Delay) {
    let data = lcd1602::build_nibble(nibble, NIBBLE_SHIFT, mode, BACKLIGHT_MASK);
    pcf_pulse_enable(i2c, addr, data, delay);
}

/// Send one full 8-bit command/data value as two nibbles.
fn lcd_send(i2c: &mut impl I2c, addr: u8, value: u8, mode: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_write4(i2c, addr, (value >> 4) & 0x0F, mode, delay);
    lcd_write4(i2c, addr, value & 0x0F, mode, delay);
}

/// Execute the HD44780 4-bit mode power-on reset sequence.
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
fn lcd_hd44780_configure(i2c: &mut impl I2c, addr: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_send(i2c, addr, 0x28, 0, delay);
    lcd_send(i2c, addr, 0x0C, 0, delay);
    lcd_send(i2c, addr, 0x01, 0, delay);
    delay.delay_ms(2);
    lcd_send(i2c, addr, 0x06, 0, delay);
}

/// Set the LCD cursor position.
fn lcd_set_cursor(i2c: &mut impl I2c, addr: u8, line: u8, position: u8, delay: &mut cortex_m::delay::Delay) {
    lcd_send(i2c, addr, lcd1602::cursor_address(line, position), 0, delay);
}

/// Write a byte slice as character data to the LCD.
fn lcd_puts(i2c: &mut impl I2c, addr: u8, s: &[u8], delay: &mut cortex_m::delay::Delay) {
    for &ch in s {
        lcd_send(i2c, addr, ch, 1, delay);
    }
}

/// Initialize the LCD, display the title, and log over UART.
fn setup_display(
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
fn update_counter(
    i2c: &mut impl I2c,
    uart: &EnabledUart,
    delay: &mut cortex_m::delay::Delay,
    count: &mut u32,
) {
    let mut buf = [0u8; 16];
    let n = lcd1602::format_counter(&mut buf, *count);
    *count += 1;
    lcd_set_cursor(i2c, LCD_I2C_ADDR, 1, 0, delay);
    lcd_puts(i2c, LCD_I2C_ADDR, &buf[..n], delay);
    uart.write_full_blocking(&buf[..n]);
    uart.write_full_blocking(b"\r\n");
    delay.delay_ms(COUNTER_DELAY_MS);
}

/// Application entry point for the LCD 1602 counter demo.
///
/// Initializes the LCD over I2C with a static title on line 0 and
/// continuously increments a counter on line 1 every second.
///
/// # Returns
///
/// Does not return.
#[entry]
fn main() -> ! {
    let mut pac = hal::pac::Peripherals::take().unwrap();
    let clocks = init_clocks(
        pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS,
        &mut hal::Watchdog::new(pac.WATCHDOG),
    );
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    let sda_pin = pins.gpio2.reconfigure::<FunctionI2C, PullUp>();
    let scl_pin = pins.gpio3.reconfigure::<FunctionI2C, PullUp>();
    let mut i2c = hal::I2C::i2c1(
        pac.I2C1, sda_pin, scl_pin, I2C_BAUD.Hz(),
        &mut pac.RESETS, clocks.system_clock.freq(),
    );
    setup_display(&mut i2c, &uart, &mut delay);
    let mut count: u32 = 0;
    loop {
        update_counter(&mut i2c, &uart, &mut delay, &mut count);
    }
}

#[unsafe(link_section = ".bi_entries")]
#[used]
pub static PICOTOOL_ENTRIES: [hal::binary_info::EntryAddr; 5] = [
    hal::binary_info::rp_cargo_bin_name!(),
    hal::binary_info::rp_cargo_version!(),
    hal::binary_info::rp_program_description!(c"LCD 1602 Counter Demo"),
    hal::binary_info::rp_cargo_homepage_url!(),
    hal::binary_info::rp_program_build_attribute!(),
];

// End of file
