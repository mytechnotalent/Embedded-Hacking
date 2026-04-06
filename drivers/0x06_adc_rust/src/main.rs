//! @file main.rs
//! @brief ADC demonstration: potentiometer voltage + on-chip temperature
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
//! Demonstrates 12-bit ADC using the adc driver (adc.rs). Reads ADC
//! channel 0 (GPIO 26) and reports the voltage in millivolts alongside
//! the on-chip temperature sensor reading every 500 ms over UART.
//!
//! Wiring:
//!   GPIO26 -> Wiper of a 10 kohm potentiometer
//!   3.3V   -> One end of the potentiometer
//!   GND    -> Other end of the potentiometer

#![no_std]
#![no_main]

// Board-level helpers: constants, type aliases, and init functions
mod board;
// ADC driver module — suppress warnings for unused public API functions
#[allow(dead_code)]
mod adc;

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
#[cfg(rp2040)]
use rp2040_hal as hal;
#[cfg(rp2350)]
use rp235x_hal as hal;

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

/// Application entry point for the ADC voltage and temperature demo.
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
    hal::binary_info::rp_program_description!(c"ADC Voltage and Temperature Demo"),
    hal::binary_info::rp_cargo_homepage_url!(),
    hal::binary_info::rp_program_build_attribute!(),
];

// End of file
