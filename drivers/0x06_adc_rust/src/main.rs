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

#[allow(dead_code)]
mod adc;

use defmt_rtt as _;
#[cfg(target_arch = "riscv32")]
use panic_halt as _;
#[cfg(target_arch = "arm")]
use panic_probe as _;

use cortex_m::prelude::_embedded_hal_adc_OneShot;
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
const POLL_MS: u32 = 500;

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

/// Format a millivolt value into "ADC0: NNNN mV  |  Chip temp: ".
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 48 bytes).
/// * `mv` - Voltage in millivolts.
/// * `temp_int` - Integer part of temperature.
/// * `temp_frac` - Single decimal digit of temperature fraction.
///
/// # Returns
///
/// Number of bytes written into the buffer.
fn format_adc_line(buf: &mut [u8], mv: u32, temp_int: i32, temp_frac: u8) -> usize {
    let prefix = b"ADC0: ";
    buf[..6].copy_from_slice(prefix);
    let mut pos = 6;
    let thousands = ((mv / 1000) % 10) as u8;
    let hundreds = ((mv / 100) % 10) as u8;
    let tens = ((mv / 10) % 10) as u8;
    let ones = (mv % 10) as u8;
    buf[pos] = b'0' + thousands; pos += 1;
    buf[pos] = b'0' + hundreds; pos += 1;
    buf[pos] = b'0' + tens; pos += 1;
    buf[pos] = b'0' + ones; pos += 1;
    let mid = b" mV  |  Chip temp: ";
    buf[pos..pos + 19].copy_from_slice(mid);
    pos += 19;
    let abs_temp = if temp_int < 0 { -temp_int } else { temp_int } as u32;
    if temp_int < 0 {
        buf[pos] = b'-'; pos += 1;
    }
    if abs_temp >= 100 {
        buf[pos] = b'0' + ((abs_temp / 100) % 10) as u8; pos += 1;
    }
    if abs_temp >= 10 {
        buf[pos] = b'0' + ((abs_temp / 10) % 10) as u8; pos += 1;
    }
    buf[pos] = b'0' + (abs_temp % 10) as u8; pos += 1;
    buf[pos] = b'.'; pos += 1;
    buf[pos] = b'0' + temp_frac; pos += 1;
    let suffix = b" C\r\n";
    buf[pos..pos + 4].copy_from_slice(suffix);
    pos += 4;
    pos
}

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
    let clocks = init_clocks(
        pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS,
        &mut hal::Watchdog::new(pac.WATCHDOG),
    );
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
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
        let n = format_adc_line(&mut buf, mv, temp_int, temp_frac);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(POLL_MS);
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
