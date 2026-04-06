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

// HAL entry-point macro
use hal::entry;

// Alias our HAL crate
#[cfg(rp2350)]
use rp235x_hal as hal;
#[cfg(rp2040)]
use rp2040_hal as hal;

/// Second-stage boot loader for RP2040
#[unsafe(link_section = ".boot2")]
#[used]
#[cfg(rp2040)]
pub static BOOT2: [u8; 256] = rp2040_boot2::BOOT_LOADER_W25Q080;

/// Boot metadata for the RP2350 Boot ROM
#[unsafe(link_section = ".start_block")]
#[used]
#[cfg(rp2350)]
pub static IMAGE_DEF: hal::block::ImageDef = hal::block::ImageDef::secure_exe();

/// Application entry point for the I2C bus scanner demo.
#[entry]
fn main() -> ! {
    board::run(hal::pac::Peripherals::take().unwrap())
}

/// Picotool binary info metadata
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
