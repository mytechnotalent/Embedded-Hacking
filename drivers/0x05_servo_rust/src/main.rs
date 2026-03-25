//! @file main.rs
//! @brief SG90 servo motor driver demonstration
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
//! Demonstrates SG90 servo control using the servo driver (servo.rs).
//! PWM at 50 Hz on GPIO 6 sweeps the servo from 0 degrees to 180 degrees
//! and back in 10-degree increments, printing each angle over UART.
//!
//! Wiring:
//!   GPIO6  -> Servo signal wire (orange or yellow)
//!   5V     -> Servo power wire  (red) -- use external 5 V supply for load
//!   GND    -> Servo ground wire (brown or black)

#![no_std]
#![no_main]

mod board;
#[allow(dead_code)]
mod servo;

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

/// Application entry point for the servo sweep demo.
///
/// Initializes the servo on GPIO 6 and continuously sweeps 0-180-0
/// degrees in 10-degree increments, reporting each angle over UART.
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
    let pwm_slices = hal::pwm::Slices::new(pac.PWM, &mut pac.RESETS);
    let mut pwm = pwm_slices.pwm3;
    let sys_hz = clocks.system_clock.freq().to_Hz();
    let div = servo::calc_clk_div(sys_hz, servo::SERVO_HZ, servo::SERVO_WRAP);
    let div_int = div as u8;
    pwm.set_div_int(div_int);
    pwm.set_div_frac((((div - div_int as f32) * 16.0) as u8).min(15));
    pwm.set_top(servo::SERVO_WRAP as u16);
    pwm.enable();
    pwm.channel_a.output_to(pins.gpio6);
    uart.write_full_blocking(b"Servo driver initialized on GPIO 6\r\n");
    uart.write_full_blocking(b"Sweeping 0 -> 180 -> 0 degrees in 10-degree steps\r\n");
    let mut buf = [0u8; 20];
    loop {
        board::sweep_angle_up(&uart, &mut pwm.channel_a, &mut delay, &mut buf);
        board::sweep_angle_down(&uart, &mut pwm.channel_a, &mut delay, &mut buf);
    }
}

#[unsafe(link_section = ".bi_entries")]
#[used]
pub static PICOTOOL_ENTRIES: [hal::binary_info::EntryAddr; 5] = [
    hal::binary_info::rp_cargo_bin_name!(),
    hal::binary_info::rp_cargo_version!(),
    hal::binary_info::rp_program_description!(c"SG90 Servo Sweep Demo"),
    hal::binary_info::rp_cargo_homepage_url!(),
    hal::binary_info::rp_program_build_attribute!(),
];

// End of file
