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

#[allow(dead_code)]
mod pwm;

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
const PWM_FREQ_HZ: u32 = 1000;
const PWM_WRAP: u32 = 10000 - 1;

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

/// Format a duty percentage into a fixed byte buffer as "Duty: NNN%\r\n".
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 16 bytes).
/// * `duty` - Duty cycle percentage to format.
///
/// # Returns
///
/// Number of bytes written into the buffer.
fn format_duty(buf: &mut [u8], duty: u8) -> usize {
    let prefix = b"Duty: ";
    buf[..6].copy_from_slice(prefix);
    let mut pos = 6;
    if duty >= 100 {
        buf[pos] = b'1'; pos += 1;
        buf[pos] = b'0'; pos += 1;
        buf[pos] = b'0'; pos += 1;
    } else if duty >= 10 {
        buf[pos] = b' '; pos += 1;
        buf[pos] = b'0' + duty / 10; pos += 1;
        buf[pos] = b'0' + duty % 10; pos += 1;
    } else {
        buf[pos] = b' '; pos += 1;
        buf[pos] = b' '; pos += 1;
        buf[pos] = b'0' + duty; pos += 1;
    }
    buf[pos] = b'%'; pos += 1;
    buf[pos] = b'\r'; pos += 1;
    buf[pos] = b'\n'; pos += 1;
    pos
}

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
    let clocks = init_clocks(
        pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS,
        &mut hal::Watchdog::new(pac.WATCHDOG),
    );
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    uart.write_full_blocking(b"PWM initialized: GPIO25 @ 1000 Hz\r\n");
    let pwm_slices = hal::pwm::Slices::new(pac.PWM, &mut pac.RESETS);
    let mut pwm = pwm_slices.pwm4;
    let sys_hz = clocks.system_clock.freq().to_Hz();
    let div = pwm::calc_clk_div(sys_hz, PWM_FREQ_HZ, PWM_WRAP);
    let div_int = div as u8;
    pwm.set_div_int(div_int);
    pwm.set_div_frac((((div - div_int as f32) * 16.0) as u8).min(15));
    pwm.set_top(PWM_WRAP as u16);
    pwm.enable();
    pwm.channel_b.output_to(pins.gpio25);
    let mut buf = [0u8; 16];
    loop {
        sweep_up(&uart, &mut pwm.channel_b, &mut delay, &mut buf);
        sweep_down(&uart, &mut pwm.channel_b, &mut delay, &mut buf);
    }
}

/// Sweep the PWM duty cycle from 0% to 100% in steps of 5.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel to set duty on.
/// * `delay` - Delay provider for 50 ms pauses.
/// * `buf` - Scratch buffer for formatting output.
fn sweep_up(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 16],
) {
    let mut duty: u8 = 0;
    while duty <= 100 {
        let level = pwm::duty_to_level(duty, PWM_WRAP) as u16;
        channel.set_duty_cycle(level).ok();
        let n = format_duty(buf, duty);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(50u32);
        duty += 5;
    }
}

/// Sweep the PWM duty cycle from 100% to 0% in steps of 5.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel to set duty on.
/// * `delay` - Delay provider for 50 ms pauses.
/// * `buf` - Scratch buffer for formatting output.
fn sweep_down(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 16],
) {
    let mut duty: i8 = 100;
    while duty >= 0 {
        let level = pwm::duty_to_level(duty as u8, PWM_WRAP) as u16;
        channel.set_duty_cycle(level).ok();
        let n = format_duty(buf, duty as u8);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(50u32);
        duty -= 5;
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
