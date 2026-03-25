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

// Board-level helpers: constants, type aliases, and init functions
mod board;
// I2C driver module — suppress warnings for unused public API functions
#[allow(dead_code)]
mod i2c;

// Debugging output over RTT
use defmt_rtt as _;
// Panic handler for RISC-V targets
#[cfg(target_arch = "riscv32")]
use panic_halt as _;
// Panic handler for ARM targets
#[cfg(target_arch = "arm")]
use panic_probe as _;

// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// HAL entry-point macro
use hal::entry;
// GPIO traits for I2C pin reconfiguration
use hal::gpio::{FunctionI2C, PullUp};

// Alias our HAL crate
#[cfg(rp2350)]
use rp235x_hal as hal;
#[cfg(rp2040)]
use rp2040_hal as hal;

// Second-stage boot loader for RP2040
#[unsafe(link_section = ".boot2")]
#[used]
#[cfg(rp2040)]
pub static BOOT2: [u8; 256] = rp2040_boot2::BOOT_LOADER_W25Q080;

// Boot metadata for the RP2350 Boot ROM
#[unsafe(link_section = ".start_block")]
#[used]
#[cfg(rp2350)]
pub static IMAGE_DEF: hal::block::ImageDef = hal::block::ImageDef::secure_exe();

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
    let clocks = board::init_clocks(
        pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS,
        &mut hal::Watchdog::new(pac.WATCHDOG),
    );
    let pins = board::init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = board::init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = board::init_delay(&clocks);
    let sda_pin = pins.gpio2.reconfigure::<FunctionI2C, PullUp>();
    let scl_pin = pins.gpio3.reconfigure::<FunctionI2C, PullUp>();
    let mut i2c = hal::I2C::i2c1(
        pac.I2C1, sda_pin, scl_pin, board::I2C_BAUD.Hz(),
        &mut pac.RESETS, clocks.system_clock.freq(),
    );
    uart.write_full_blocking(b"I2C driver initialized: I2C1 @ 100000 Hz  SDA=GPIO2  SCL=GPIO3\r\n");
    let mut buf = [0u8; 80];
    loop {
        let n = i2c::format_scan_header(&mut buf);
        uart.write_full_blocking(&buf[..n]);
        for addr in 0u8..128 {
            let found = !i2c::is_reserved(addr) && board::probe_addr(&mut i2c, addr);
            let n = i2c::format_scan_entry(&mut buf, addr, found);
            uart.write_full_blocking(&buf[..n]);
        }
        delay.delay_ms(board::SCAN_DELAY_MS);
    }
}

// Picotool binary info metadata
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
