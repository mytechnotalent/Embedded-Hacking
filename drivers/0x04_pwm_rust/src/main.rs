//! @file main.rs
//! @brief PWM demonstration: LED breathing effect via duty-cycle sweep
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
//! Demonstrates PWM output using the pwm driver (pwm.rs). A 1 kHz
//! signal on GPIO 25 (onboard LED) sweeps its duty cycle from 0% to 100%
//! and back to produce a smooth breathing effect. The current duty is
//! reported over UART at 115200 baud.
//!
//! Wiring:
//!   GPIO0  -> UART TX (USB-to-UART adapter RX)
//!   GPIO1  -> UART RX (USB-to-UART adapter TX)
//!   GPIO25 -> Onboard LED (no external wiring needed)

#![no_std]
#![no_main]

mod board;
#[allow(dead_code)]
mod pwm;

use defmt_rtt as _;
#[cfg(target_arch = "riscv32")]
use panic_halt as _;
#[cfg(target_arch = "arm")]
use panic_probe as _;

use hal::Clock;
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

/// Application entry point for the PWM LED breathing demo.
///
/// Initializes PWM at 1 kHz on the onboard LED and enters an infinite
/// loop that sweeps the duty cycle up and down to produce a smooth
/// breathing effect, reporting each step over UART.
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
    uart.write_full_blocking(b"PWM initialized: GPIO25 @ 1000 Hz\r\n");
    let pwm_slices = hal::pwm::Slices::new(pac.PWM, &mut pac.RESETS);
    let mut pwm_slice = pwm_slices.pwm4;
    let sys_hz = clocks.system_clock.freq().to_Hz();
    let div = pwm::calc_clk_div(sys_hz, board::PWM_FREQ_HZ, board::PWM_WRAP);
    let div_int = div as u8;
    pwm_slice.set_div_int(div_int);
    pwm_slice.set_div_frac((((div - div_int as f32) * 16.0) as u8).min(15));
    pwm_slice.set_top(board::PWM_WRAP as u16);
    pwm_slice.enable();
    pwm_slice.channel_b.output_to(pins.gpio25);
    let mut buf = [0u8; 16];
    loop {
        board::sweep_up(&uart, &mut pwm_slice.channel_b, &mut delay, &mut buf);
        board::sweep_down(&uart, &mut pwm_slice.channel_b, &mut delay, &mut buf);
    }
}

#[unsafe(link_section = ".bi_entries")]
#[used]
pub static PICOTOOL_ENTRIES: [hal::binary_info::EntryAddr; 5] = [
    hal::binary_info::rp_cargo_bin_name!(),
    hal::binary_info::rp_cargo_version!(),
    hal::binary_info::rp_program_description!(c"PWM LED Breathing Demo"),
    hal::binary_info::rp_cargo_homepage_url!(),
    hal::binary_info::rp_program_build_attribute!(),
];

// End of file
