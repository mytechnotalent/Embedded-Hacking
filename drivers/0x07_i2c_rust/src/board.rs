//! @file board.rs
//! @brief Board-level HAL helpers for the I2C driver
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

// I2C bus trait for device probing
use embedded_hal::i2c::I2c;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionI2C, FunctionNull, FunctionUart, Pin, PullDown, PullNone, PullUp};
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

// Alias our HAL crate
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

/// Delay between scan cycles in milliseconds.
pub(crate) const SCAN_DELAY_MS: u32 = 5_000;

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

/// Probe a 7-bit I2C address by attempting a 1-byte read.
///
/// # Arguments
///
/// * `i2c` - Mutable reference to the I2C bus.
/// * `addr` - 7-bit I2C address to probe.
///
/// # Returns
///
/// `true` if a device acknowledged, `false` otherwise.
pub(crate) fn probe_addr(i2c: &mut impl I2c, addr: u8) -> bool {
    let mut dummy = [0u8; 1];
    i2c.read(addr, &mut dummy).is_ok()
}

/// Initialise all peripherals and run the I2C bus scanner demo.
///
/// # Arguments
///
/// * `pac` - PAC Peripherals singleton (consumed).
pub(crate) fn run(mut pac: hal::pac::Peripherals) -> ! {
    let mut wd = hal::Watchdog::new(pac.WATCHDOG);
    let clocks = init_clocks(pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS, &mut wd);
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    let mut i2c = init_i2c(pac.I2C1, pins.gpio2, pins.gpio3, &mut pac.RESETS, &clocks);
    uart.write_full_blocking(b"I2C driver initialized: I2C1 @ 100000 Hz  SDA=GPIO2  SCL=GPIO3\r\n");
    scan_loop(&uart, &mut i2c, &mut delay)
}

/// Initialise I2C1 on SDA=GPIO2 / SCL=GPIO3.
///
/// # Arguments
///
/// * `i2c1` - PAC I2C1 peripheral singleton.
/// * `sda` - Default GPIO 2 pin (will be reconfigured for I2C).
/// * `scl` - Default GPIO 3 pin (will be reconfigured for I2C).
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `clocks` - Reference to the initialised clock configuration.
///
/// # Returns
///
/// Configured I2C1 bus controller.
fn init_i2c(
    i2c1: hal::pac::I2C1,
    sda: Pin<hal::gpio::bank0::Gpio2, FunctionNull, PullDown>,
    scl: Pin<hal::gpio::bank0::Gpio3, FunctionNull, PullDown>,
    resets: &mut hal::pac::RESETS,
    clocks: &hal::clocks::ClocksManager,
) -> impl I2c {
    let sda = sda.reconfigure::<FunctionI2C, PullUp>();
    let scl = scl.reconfigure::<FunctionI2C, PullUp>();
    hal::I2C::i2c1(i2c1, sda, scl, I2C_BAUD.Hz(), resets, clocks.system_clock.freq())
}

/// Run the I2C address scan loop forever.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `i2c` - Mutable reference to the I2C bus controller.
/// * `delay` - Mutable reference to the blocking delay provider.
fn scan_loop(uart: &EnabledUart, i2c: &mut impl I2c, delay: &mut cortex_m::delay::Delay) -> ! {
    let mut buf = [0u8; 80];
    loop {
        let n = crate::i2c::format_scan_header(&mut buf);
        uart.write_full_blocking(&buf[..n]);
        scan_addresses(uart, i2c, &mut buf);
        delay.delay_ms(SCAN_DELAY_MS);
    }
}

/// Scan all 128 addresses and print the formatted result.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `i2c` - Mutable reference to the I2C bus controller.
/// * `buf` - Scratch buffer for formatting output.
fn scan_addresses(uart: &EnabledUart, i2c: &mut impl I2c, buf: &mut [u8; 80]) {
    for addr in 0u8..128 {
        let found = !crate::i2c::is_reserved(addr) && probe_addr(i2c, addr);
        let n = crate::i2c::format_scan_entry(buf, addr, found);
        uart.write_full_blocking(&buf[..n]);
    }
}

// End of file
