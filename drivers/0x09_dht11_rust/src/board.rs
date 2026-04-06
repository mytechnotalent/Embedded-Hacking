//! @file board.rs
//! @brief Board-level HAL helpers for the DHT11 driver
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

// DHT11 pure-logic functions (checksum, parsing, formatting)
use crate::dht11;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
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

/// Timer device type for the HAL timer peripheral.
#[cfg(rp2350)]
pub(crate) type HalTimer = hal::Timer<hal::timer::CopyableTimer0>;
/// Timer type alias for RP2040 (non-generic).
#[cfg(rp2040)]
pub(crate) type HalTimer = hal::Timer;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// GPIO pin number connected to the DHT11 data line.
pub(crate) const DHT11_GPIO: u8 = 4;

/// Polling interval in milliseconds (DHT11 minimum is 2 seconds).
pub(crate) const POLL_MS: u32 = 2_000;

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

/// Set or clear the DHT11 GPIO output level via SIO registers.
fn sio_set_level(sio: &hal::pac::sio::RegisterBlock, high: bool) {
    if high {
        sio.gpio_out_set()
            .write(|w| unsafe { w.bits(1u32 << DHT11_GPIO) });
    } else {
        sio.gpio_out_clr()
            .write(|w| unsafe { w.bits(1u32 << DHT11_GPIO) });
    }
}

/// Drive the DHT11 data pin to a given level (enables output).
///
/// # Arguments
///
/// * `high` - `true` to drive HIGH, `false` to drive LOW.
fn gpio_drive(high: bool) {
    unsafe {
        let sio = &*hal::pac::SIO::PTR;
        sio_set_level(sio, high);
        sio.gpio_oe_set().write(|w| w.bits(1u32 << DHT11_GPIO));
    }
}

/// Release the DHT11 data pin back to input mode (disable output driver).
fn gpio_release() {
    unsafe {
        let sio = &*hal::pac::SIO::PTR;
        sio.gpio_oe_clr().write(|w| w.bits(1u32 << DHT11_GPIO));
    }
}

/// Read the current logic level of the DHT11 data pin.
///
/// # Returns
///
/// `true` if the pin reads HIGH, `false` if LOW.
fn gpio_read() -> bool {
    unsafe { (*hal::pac::SIO::PTR).gpio_in().read().bits() & (1u32 << DHT11_GPIO) != 0 }
}

/// Read the free-running microsecond timer (lower 32 bits).
///
/// # Arguments
///
/// * `timer` - Reference to the HAL timer peripheral.
///
/// # Returns
///
/// Current timer value in microseconds (wrapping at 2^32).
fn time_us_32(timer: &HalTimer) -> u32 {
    timer.get_counter().ticks() as u32
}

/// Send the DHT11 start signal on the data pin.
///
/// Drives the pin LOW for 18 ms then HIGH for 40 us before switching
/// the pin to input mode to listen for the sensor response.
///
/// # Arguments
///
/// * `delay` - Mutable reference to the blocking delay provider.
fn send_start_signal(delay: &mut cortex_m::delay::Delay) {
    gpio_drive(false);
    delay.delay_ms(18);
    gpio_drive(true);
    delay.delay_us(40);
    gpio_release();
}

/// Spin until the pin leaves the given logic level, or time out.
///
/// # Arguments
///
/// * `level` - Logic level to wait through (`true` = HIGH, `false` = LOW).
///
/// # Returns
///
/// `true` once the level changed, `false` on timeout.
fn wait_for_level(level: bool) -> bool {
    let mut timeout: u32 = dht11::LEVEL_WAIT_TIMEOUT;
    while gpio_read() == level {
        timeout -= 1;
        if timeout == 0 {
            return false;
        }
    }
    true
}

/// Wait for the DHT11 response after the start signal.
///
/// The sensor pulls LOW then HIGH then LOW again; each transition
/// is awaited with a timeout.
///
/// # Returns
///
/// `true` if the full response was received, `false` on timeout.
fn wait_response() -> bool {
    wait_for_level(true) && wait_for_level(false) && wait_for_level(true)
}

/// Measure the high-period duration for a single DHT11 bit.
///
/// # Arguments
///
/// * `timer` - Reference to the HAL timer for microsecond measurement.
///
/// # Returns
///
/// Duration in microseconds, or `None` on timeout.
fn measure_bit_duration(timer: &HalTimer) -> Option<u32> {
    if !wait_for_level(false) {
        return None;
    }
    let start = time_us_32(timer);
    if !wait_for_level(true) {
        return None;
    }
    Some(time_us_32(timer).wrapping_sub(start))
}

/// Read a single bit from the DHT11 data stream.
///
/// Waits for the low-period to end, measures the high-period duration,
/// and accumulates the result into the data array via
/// [`dht11::accumulate_bit`].
///
/// # Arguments
///
/// * `data` - 5-byte array accumulating the received bits.
/// * `i` - Bit index (0–39).
/// * `timer` - Reference to the HAL timer for microsecond measurement.
///
/// # Returns
///
/// `true` on success, `false` on timeout.
fn read_bit(data: &mut [u8; 5], i: usize, timer: &HalTimer) -> bool {
    let Some(duration) = measure_bit_duration(timer) else {
        return false;
    };
    dht11::accumulate_bit(data, i, duration);
    true
}

/// Read all 40 data bits from the DHT11.
///
/// # Arguments
///
/// * `data` - 5-byte array filled with the received data.
/// * `timer` - Reference to the HAL timer for microsecond measurement.
///
/// # Returns
///
/// `true` if all 40 bits were read, `false` on timeout.
fn read_40_bits(data: &mut [u8; 5], timer: &HalTimer) -> bool {
    for i in 0..40 {
        if !read_bit(data, i, timer) {
            return false;
        }
    }
    true
}

/// Run the full DHT11 acquisition sequence (start, response, 40 bits, checksum).
fn acquire_data(timer: &HalTimer, delay: &mut cortex_m::delay::Delay, data: &mut [u8; 5]) -> bool {
    send_start_signal(delay);
    wait_response() && read_40_bits(data, timer) && dht11::validate_checksum(data)
}

/// Execute the full DHT11 read protocol.
///
/// Sends the start signal, waits for the sensor response, reads 40 bits
/// of data, validates the checksum, and parses humidity and temperature.
///
/// # Arguments
///
/// * `timer` - Reference to the HAL timer for microsecond measurement.
/// * `delay` - Mutable reference to the blocking delay provider.
///
/// # Returns
///
/// `Some((humidity, temperature))` on success, `None` on failure.
pub(crate) fn read_sensor(
    timer: &HalTimer,
    delay: &mut cortex_m::delay::Delay,
) -> Option<(f32, f32)> {
    let mut data = [0u8; 5];
    if !acquire_data(timer, delay, &mut data) {
        return None;
    }
    Some((
        dht11::parse_humidity(&data),
        dht11::parse_temperature(&data),
    ))
}

/// Format the sensor result (or error) with CRLF into `buf`.
fn format_sensor_output(buf: &mut [u8], result: Option<(f32, f32)>) -> usize {
    let n = match result {
        Some((h, t)) => dht11::format_reading(buf, h, t),
        None => dht11::format_error(buf, DHT11_GPIO),
    };
    buf[n] = b'\r';
    buf[n + 1] = b'\n';
    n + 2
}

/// Read the sensor, format the result, write it over UART, and wait.
///
/// On a successful read, prints humidity and temperature; on failure,
/// prints a wiring-check message. Always waits [`POLL_MS`] before
/// returning to respect the DHT11 minimum polling interval.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `timer` - Reference to the HAL timer for microsecond measurement.
/// * `delay` - Mutable reference to the blocking delay provider.
pub(crate) fn poll_sensor(
    uart: &EnabledUart,
    timer: &HalTimer,
    delay: &mut cortex_m::delay::Delay,
) {
    let mut buf = [0u8; 64];
    let result = read_sensor(timer, delay);
    let n = format_sensor_output(&mut buf, result);
    uart.write_full_blocking(&buf[..n]);
    delay.delay_ms(POLL_MS);
}

/// Initialise all peripherals and run the DHT11 sensor demo.
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
    #[cfg(rp2350)]
    let timer = hal::Timer::new_timer0(pac.TIMER0, &mut pac.RESETS, &clocks);
    #[cfg(rp2040)]
    let timer = hal::Timer::new(pac.TIMER, &mut pac.RESETS);
    let _ = pins.gpio4.into_pull_up_input();
    uart.write_full_blocking(b"DHT11 driver initialized on GPIO 4\r\n");
    loop {
        poll_sensor(&uart, &timer, &mut delay);
    }
}

// End of file
