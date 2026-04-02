//! @file board.rs
//! @brief Board-level HAL helpers for the watchdog driver
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

// Watchdog driver pure-logic functions and constants
use crate::watchdog;
// Microsecond duration type for watchdog timeout
use fugit::ExtU32;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

// Alias our HAL crate
#[cfg(rp2350)]
use rp235x_hal as hal;
#[cfg(rp2040)]
use rp2040_hal as hal;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

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

/// Check whether the last reset was caused by the watchdog.
///
/// Reads the WATCHDOG REASON register directly from the PAC. Returns
/// `true` if either the timer or force bits are set, matching the
/// C SDK `watchdog_caused_reboot()` behaviour.
///
/// # Returns
///
/// `true` if the watchdog triggered the last reset.
pub(crate) fn watchdog_caused_reboot() -> bool {
    let watchdog = unsafe { &*hal::pac::WATCHDOG::ptr() };
    let reason = watchdog.reason().read();
    reason.timer().bit_is_set() || reason.force().bit_is_set()
}

/// Enable the hardware watchdog with the specified timeout.
///
/// Wraps `hal::Watchdog::start()` converting the timeout from
/// milliseconds to microseconds as required by the HAL.
///
/// # Arguments
///
/// * `watchdog` - Mutable reference to the HAL watchdog.
/// * `timeout_ms` - Timeout in milliseconds (1–8388).
pub(crate) fn watchdog_enable(watchdog: &mut hal::Watchdog, timeout_ms: u32) {
    let timeout_us = timeout_ms * 1_000;
    watchdog.start(timeout_us.micros());
}

/// Feed the hardware watchdog to prevent a reboot.
///
/// Wraps `hal::Watchdog::feed()`.
///
/// # Arguments
///
/// * `watchdog` - Reference to the HAL watchdog.
pub(crate) fn watchdog_feed(watchdog: &hal::Watchdog) {
    watchdog.feed();
}

/// Run the watchdog feed-and-report loop.
///
/// Feeds the watchdog every 1 second and prints `"Watchdog fed\r\n"`
/// over UART, matching the C demo's `_feed_and_report()` function.
/// This function never returns.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `watchdog` - Reference to the HAL watchdog.
/// * `delay` - Mutable reference to the blocking delay provider.
/// * `state` - Mutable reference to the watchdog driver state.
pub(crate) fn feed_loop(
    uart: &EnabledUart,
    watchdog: &hal::Watchdog,
    delay: &mut cortex_m::delay::Delay,
    state: &mut watchdog::WatchdogDriverState,
) -> ! {
    loop {
        watchdog_feed(watchdog);
        state.feed();
        let mut buf = [0u8; 32];
        let n = watchdog::format_fed(&mut buf);
        uart.write_full_blocking(&buf[..n]);
        delay.delay_ms(watchdog::FEED_INTERVAL_MS);
    }
}

/// Initialise all peripherals and run the watchdog feed demo.
///
/// # Arguments
///
/// * `pac` - PAC Peripherals singleton (consumed).
pub(crate) fn run(mut pac: hal::pac::Peripherals) -> ! {
    let mut wd = hal::Watchdog::new(pac.WATCHDOG);
    let clocks = init_clocks(pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS, &mut wd);
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    report_reset_reason(&uart);
    let mut state = start_watchdog(&uart, &mut wd);
    feed_loop(&uart, &wd, &mut delay, &mut state)
}

/// Print whether the last reset was caused by the watchdog.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
fn report_reset_reason(uart: &EnabledUart) {
    let mut buf = [0u8; 64];
    let caused = watchdog_caused_reboot();
    let n = watchdog::format_reset_reason(&mut buf, caused);
    uart.write_full_blocking(&buf[..n]);
}

/// Create the driver state, enable the hardware watchdog, and report.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `wd` - Mutable reference to the HAL watchdog.
///
/// # Returns
///
/// Initialised watchdog driver state.
fn start_watchdog(uart: &EnabledUart, wd: &mut hal::Watchdog) -> watchdog::WatchdogDriverState {
    let mut state = watchdog::WatchdogDriverState::new();
    state.enable(watchdog::DEFAULT_TIMEOUT_MS);
    watchdog_enable(wd, watchdog::DEFAULT_TIMEOUT_MS);
    let mut buf = [0u8; 64];
    let n = watchdog::format_enabled(&mut buf, watchdog::DEFAULT_TIMEOUT_MS);
    uart.write_full_blocking(&buf[..n]);
    state
}

// End of file
