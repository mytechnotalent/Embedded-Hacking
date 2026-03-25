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

mod board;
#[allow(dead_code)]
mod adc;

use defmt_rtt as _;
#[cfg(target_arch = "riscv32")]
use panic_halt as _;
#[cfg(target_arch = "arm")]
use panic_probe as _;

use cortex_m::prelude::_embedded_hal_adc_OneShot;
use hal::entry;

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

/// Application entry point for the ADC voltage and temperature demo.
///
/// Initializes the ADC on GPIO26 channel 0 and prints readings
/// every 500 ms over UART.
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
    let mut adc_hw = hal::Adc::new(pac.ADC, &mut pac.RESETS);
    let mut adc_pin = hal::adc::AdcPin::new(pins.gpio26).unwrap();
    let mut temp_sensor = adc_hw.take_temp_sensor().unwrap();
    uart.write_full_blocking(b"ADC driver initialized: GPIO26 (channel 0)\r\n");
    let mut buf = [0u8; 48];
    loop {
        let raw_v: u16 = adc_hw.read(&mut adc_pin).unwrap();
        let mv = adc::raw_to_mv(raw_v);
        let raw_t: u16 = adc_hw.read(&mut temp_sensor).unwrap();
        let temp = adc::raw_to_celsius(raw_t);
        let temp_int = temp as i32;
        let temp_frac = (((temp - temp_int as f32) * 10.0) as u8).min(9);
        let n = board::format_adc_line(&mut buf, mv, temp_int, temp_frac);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(board::POLL_MS);
    }
}

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
