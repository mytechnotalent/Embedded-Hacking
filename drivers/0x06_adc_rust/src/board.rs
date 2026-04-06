//! @file board.rs
//! @brief Board-level HAL helpers for the ADC driver
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

// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// ADC one-shot trait for .read()
use cortex_m::prelude::_embedded_hal_adc_OneShot;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

// Alias our HAL crate
#[cfg(rp2040)]
use rp2040_hal as hal;
#[cfg(rp2350)]
use rp235x_hal as hal;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// Main-loop polling interval in milliseconds.
pub(crate) const POLL_MS: u32 = 500;

/// Type alias for the configured TX pin (GPIO 0, UART function, no pull).
pub(crate) type TxPin = Pin<hal::gpio::bank0::Gpio0, FunctionUart, PullNone>;

/// Type alias for the configured RX pin (GPIO 1, UART function, no pull).
pub(crate) type RxPin = Pin<hal::gpio::bank0::Gpio1, FunctionUart, PullNone>;

/// Type alias for the default TX pin state from `Pins::new()`.
pub(crate) type TxPinDefault = Pin<hal::gpio::bank0::Gpio0, FunctionNull, PullDown>;

/// Type alias for the default RX pin state from `Pins::new()`.
pub(crate) type RxPinDefault = Pin<hal::gpio::bank0::Gpio1, FunctionNull, PullDown>;

/// Type alias for the fully-enabled UART0 peripheral with TX/RX pins.
pub(crate) type EnabledUart = UartPeripheral<Enabled, hal::pac::UART0, (TxPin, RxPin)>;

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
pub(crate) fn init_clocks(
    xosc: hal::pac::XOSC,
    clocks: hal::pac::CLOCKS,
    pll_sys: hal::pac::PLL_SYS,
    pll_usb: hal::pac::PLL_USB,
    resets: &mut hal::pac::RESETS,
    watchdog: &mut hal::Watchdog,
) -> hal::clocks::ClocksManager {
    hal::clocks::init_clocks_and_plls(
        XTAL_FREQ_HZ,
        xosc,
        clocks,
        pll_sys,
        pll_usb,
        resets,
        watchdog,
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
pub(crate) fn init_pins(
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
pub(crate) fn init_uart(
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
pub(crate) fn init_delay(clocks: &hal::clocks::ClocksManager) -> cortex_m::delay::Delay {
    let core = cortex_m::Peripherals::take().unwrap();
    cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz())
}

/// Write a conditional digit into `buf` if `val` meets the threshold.
fn write_conditional_digit(
    buf: &mut [u8],
    pos: &mut usize,
    val: u32,
    threshold: u32,
    divisor: u32,
) {
    if val >= threshold {
        buf[*pos] = b'0' + ((val / divisor) % 10) as u8;
        *pos += 1;
    }
}

/// Write a u32 with minimum digits (no leading zeros).
fn write_min_digits(buf: &mut [u8], val: u32) -> usize {
    let mut pos = 0;
    write_conditional_digit(buf, &mut pos, val, 100, 100);
    write_conditional_digit(buf, &mut pos, val, 10, 10);
    buf[pos] = b'0' + (val % 10) as u8;
    pos + 1
}

/// Write 4-digit millivolt value into `buf`.
fn write_mv_digits(buf: &mut [u8], mv: u32) -> usize {
    buf[0] = b'0' + ((mv / 1000) % 10) as u8;
    buf[1] = b'0' + ((mv / 100) % 10) as u8;
    buf[2] = b'0' + ((mv / 10) % 10) as u8;
    buf[3] = b'0' + (mv % 10) as u8;
    4
}

/// Write a negative sign if needed and return the absolute temperature value.
fn write_sign(buf: &mut [u8], pos: &mut usize, temp_int: i32) -> u32 {
    if temp_int < 0 {
        buf[*pos] = b'-';
        *pos += 1;
        (-temp_int) as u32
    } else {
        temp_int as u32
    }
}

/// Write temperature as "[-]NN.F" into `buf`.
fn write_temp(buf: &mut [u8], temp_int: i32, temp_frac: u8) -> usize {
    let mut pos = 0;
    let abs_temp = write_sign(buf, &mut pos, temp_int);
    pos += write_min_digits(&mut buf[pos..], abs_temp);
    buf[pos] = b'.';
    buf[pos + 1] = b'0' + temp_frac;
    pos + 2
}

/// Format a millivolt value into "ADC0: NNNN mV  |  Chip temp: NN.N C\r\n".
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
pub(crate) fn format_adc_line(buf: &mut [u8], mv: u32, temp_int: i32, temp_frac: u8) -> usize {
    buf[..6].copy_from_slice(b"ADC0: ");
    let mut pos = 6;
    pos += write_mv_digits(&mut buf[pos..], mv);
    buf[pos..pos + 19].copy_from_slice(b" mV  |  Chip temp: ");
    pos += 19;
    pos += write_temp(&mut buf[pos..], temp_int, temp_frac);
    buf[pos..pos + 4].copy_from_slice(b" C\r\n");
    pos + 4
}

/// Type alias for the ADC input pin on GPIO 26.
type Gpio26Adc = hal::adc::AdcPin<Pin<hal::gpio::bank0::Gpio26, FunctionNull, PullDown>>;

/// Initialise all peripherals and run the ADC demo.
///
/// # Arguments
///
/// * `pac` - PAC Peripherals singleton (consumed).
pub(crate) fn run(mut pac: hal::pac::Peripherals) -> ! {
    let mut wd = hal::Watchdog::new(pac.WATCHDOG);
    let clocks = init_clocks(
        pac.XOSC,
        pac.CLOCKS,
        pac.PLL_SYS,
        pac.PLL_USB,
        &mut pac.RESETS,
        &mut wd,
    );
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    let (mut adc, mut adc_pin, mut temp) = init_adc(pac.ADC, pins.gpio26, &mut pac.RESETS);
    uart.write_full_blocking(b"ADC driver initialized: GPIO26 (channel 0)\r\n");
    adc_loop(&uart, &mut adc, &mut adc_pin, &mut temp, &mut delay)
}

/// Create the ADC peripheral, GPIO 26 input channel, and temperature sensor.
///
/// # Arguments
///
/// * `adc_pac` - PAC ADC peripheral singleton.
/// * `gpio26` - Default GPIO 26 pin to use as ADC input.
/// * `resets` - Mutable reference to the RESETS peripheral.
///
/// # Returns
///
/// Tuple of (ADC driver, ADC pin channel, temperature sensor channel).
fn init_adc(
    adc_pac: hal::pac::ADC,
    gpio26: Pin<hal::gpio::bank0::Gpio26, FunctionNull, PullDown>,
    resets: &mut hal::pac::RESETS,
) -> (hal::Adc, Gpio26Adc, hal::adc::TempSense) {
    let mut adc = hal::Adc::new(adc_pac, resets);
    let pin = hal::adc::AdcPin::new(gpio26).unwrap();
    let temp = adc.take_temp_sensor().unwrap();
    (adc, pin, temp)
}

/// Sample voltage and temperature, format, and print in a loop.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `adc` - Mutable reference to the ADC driver.
/// * `adc_pin` - Mutable reference to the GPIO 26 ADC channel.
/// * `temp` - Mutable reference to the temperature sensor channel.
/// * `delay` - Mutable reference to the blocking delay provider.
fn adc_loop(
    uart: &EnabledUart,
    adc: &mut hal::Adc,
    adc_pin: &mut Gpio26Adc,
    temp: &mut hal::adc::TempSense,
    delay: &mut cortex_m::delay::Delay,
) -> ! {
    let mut buf = [0u8; 48];
    loop {
        let (mv, temp_int, temp_frac) = read_adc(adc, adc_pin, temp);
        let n = format_adc_line(&mut buf, mv, temp_int, temp_frac);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(POLL_MS);
    }
}

/// Read voltage and temperature from the ADC.
///
/// # Arguments
///
/// * `adc` - Mutable reference to the ADC driver.
/// * `adc_pin` - Mutable reference to the GPIO 26 ADC channel.
/// * `temp` - Mutable reference to the temperature sensor channel.
///
/// # Returns
///
/// Tuple of (millivolts, integer temperature, fractional temperature digit).
fn read_adc(
    adc: &mut hal::Adc,
    adc_pin: &mut Gpio26Adc,
    temp: &mut hal::adc::TempSense,
) -> (u32, i32, u8) {
    let raw_v: u16 = adc.read(adc_pin).unwrap();
    let mv = crate::adc::raw_to_mv(raw_v);
    let raw_t: u16 = adc.read(temp).unwrap();
    let celsius = crate::adc::raw_to_celsius(raw_t);
    let temp_int = celsius as i32;
    let temp_frac = (((celsius - temp_int as f32) * 10.0) as u8).min(9);
    (mv, temp_int, temp_frac)
}

// End of file
