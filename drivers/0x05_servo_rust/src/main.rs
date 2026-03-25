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

#[allow(dead_code)]
mod servo;

use defmt_rtt as _;
#[cfg(target_arch = "riscv32")]
use panic_halt as _;
#[cfg(target_arch = "arm")]
use panic_probe as _;

use embedded_hal::pwm::SetDutyCycle;
use fugit::RateExtU32;
use hal::entry;
use hal::Clock;
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
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
const STEP_DEGREES: i32 = 10;
const STEP_DELAY_MS: u32 = 150;

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

/// Format an angle into "Angle: NNN deg\r\n".
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 20 bytes).
/// * `angle` - Angle in degrees (0..180).
///
/// # Returns
///
/// Number of bytes written into the buffer.
fn format_angle(buf: &mut [u8], angle: i32) -> usize {
    let prefix = b"Angle: ";
    buf[..7].copy_from_slice(prefix);
    let mut pos = 7;
    let a = if angle < 0 { 0 } else { angle as u32 };
    if a >= 100 {
        buf[pos] = b'0' + (a / 100) as u8; pos += 1;
        buf[pos] = b'0' + ((a / 10) % 10) as u8; pos += 1;
        buf[pos] = b'0' + (a % 10) as u8; pos += 1;
    } else if a >= 10 {
        buf[pos] = b' '; pos += 1;
        buf[pos] = b'0' + (a / 10) as u8; pos += 1;
        buf[pos] = b'0' + (a % 10) as u8; pos += 1;
    } else {
        buf[pos] = b' '; pos += 1;
        buf[pos] = b' '; pos += 1;
        buf[pos] = b'0' + a as u8; pos += 1;
    }
    let suffix = b" deg\r\n";
    buf[pos..pos + 6].copy_from_slice(suffix);
    pos + 6
}

/// Sweep the servo angle upward from 0 to 180 in STEP_DEGREES increments.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel implementing SetDutyCycle.
/// * `delay` - Delay provider for pause between steps.
/// * `buf` - Scratch buffer for formatting output.
fn sweep_angle_up(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 20],
) {
    let mut angle: i32 = 0;
    while angle <= 180 {
        let pulse = servo::angle_to_pulse_us(angle as f32, servo::SERVO_DEFAULT_MIN_US, servo::SERVO_DEFAULT_MAX_US);
        let level = servo::pulse_us_to_level(pulse as u32, servo::SERVO_WRAP, servo::SERVO_HZ) as u16;
        channel.set_duty_cycle(level).ok();
        let n = format_angle(buf, angle);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(STEP_DELAY_MS);
        angle += STEP_DEGREES;
    }
}

/// Sweep the servo angle downward from 180 to 0 in STEP_DEGREES decrements.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel implementing SetDutyCycle.
/// * `delay` - Delay provider for pause between steps.
/// * `buf` - Scratch buffer for formatting output.
fn sweep_angle_down(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 20],
) {
    let mut angle: i32 = 180;
    while angle >= 0 {
        let pulse = servo::angle_to_pulse_us(angle as f32, servo::SERVO_DEFAULT_MIN_US, servo::SERVO_DEFAULT_MAX_US);
        let level = servo::pulse_us_to_level(pulse as u32, servo::SERVO_WRAP, servo::SERVO_HZ) as u16;
        channel.set_duty_cycle(level).ok();
        let n = format_angle(buf, angle);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(STEP_DELAY_MS);
        angle -= STEP_DEGREES;
    }
}

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
    let clocks = init_clocks(
        pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS,
        &mut hal::Watchdog::new(pac.WATCHDOG),
    );
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
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
        sweep_angle_up(&uart, &mut pwm.channel_a, &mut delay, &mut buf);
        sweep_angle_down(&uart, &mut pwm.channel_a, &mut delay, &mut buf);
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
