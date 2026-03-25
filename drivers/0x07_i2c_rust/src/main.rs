//! @file main.rs
//! @brief I2C demonstration: scan all 7-bit addresses and report devices
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
//! Demonstrates I2C bus scanning using the i2c driver (i2c.rs). I2C1
//! is configured at 100 kHz on SDA=GPIO2 / SCL=GPIO3. A formatted hex
//! table of all responding device addresses is printed over UART and
//! repeated every 5 seconds.
//!
//! Wiring:
//!   GPIO2 (SDA) -> I2C device SDA  (4.7 kohm pull-up to 3.3 V recommended)
//!   GPIO3 (SCL) -> I2C device SCL  (4.7 kohm pull-up to 3.3 V recommended)
//!   3.3V        -> I2C device VCC
//!   GND         -> I2C device GND

#![no_std]
#![no_main]

#[allow(dead_code)]
mod i2c;

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
const SCAN_DELAY_MS: u32 = 5_000;

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
fn probe_addr(i2c: &mut impl I2c, addr: u8) -> bool {
    let mut dummy = [0u8; 1];
    i2c.read(addr, &mut dummy).is_ok()
}

/// Application entry point for the I2C bus scanner demo.
///
/// Initializes I2C1 at 100 kHz on SDA=GPIO2 / SCL=GPIO3 and prints
/// a formatted hex table of all responding device addresses over UART,
/// repeating every 5 seconds.
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
    uart.write_full_blocking(b"I2C driver initialized: I2C1 @ 100000 Hz  SDA=GPIO2  SCL=GPIO3\r\n");
    let mut buf = [0u8; 80];
    loop {
        let n = i2c::format_scan_header(&mut buf);
        uart.write_full_blocking(&buf[..n]);
        for addr in 0u8..128 {
            let found = !i2c::is_reserved(addr) && probe_addr(&mut i2c, addr);
            let n = i2c::format_scan_entry(&mut buf, addr, found);
            uart.write_full_blocking(&buf[..n]);
        }
        delay.delay_ms(SCAN_DELAY_MS);
    }
}

#[unsafe(link_section = ".bi_entries")]
#[used]
pub static PICOTOOL_ENTRIES: [hal::binary_info::EntryAddr; 5] = [
    hal::binary_info::rp_cargo_bin_name!(),
    hal::binary_info::rp_cargo_version!(),
    hal::binary_info::rp_program_description!(c"I2C Bus Scanner Demo"),
    hal::binary_info::rp_cargo_homepage_url!(),
    hal::binary_info::rp_program_build_attribute!(),
];

// End of file
