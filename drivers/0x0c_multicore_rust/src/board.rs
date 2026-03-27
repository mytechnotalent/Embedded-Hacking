//! @file board.rs
//! @brief Board-level HAL helpers for the multicore driver
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

// Multicore pure-logic helpers and constants
use crate::multicore;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
// Multicore execution management and stack type for core 1
use hal::multicore::{Multicore, Stack};
// SIO type for inter-core FIFO and GPIO bank ownership
use hal::sio::{Sio, SioFifo};
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

/// Delay between FIFO round-trip messages in milliseconds.
pub(crate) const POLL_MS: u32 = 1_000;

/// Maximum buffer size for formatting a round-trip message.
pub(crate) const MSG_BUF_LEN: usize = 52;

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

// Stack allocation for core 1 (4096 words)
static CORE1_STACK: Stack<4096> = Stack::new();

/// Initialise system clocks and PLLs from the external 12 MHz crystal.
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

/// Unlock the GPIO bank and return the pin set along with the SIO FIFO.
pub(crate) fn init_pins(
    io_bank0: hal::pac::IO_BANK0,
    pads_bank0: hal::pac::PADS_BANK0,
    sio: hal::pac::SIO,
    resets: &mut hal::pac::RESETS,
) -> (hal::gpio::Pins, SioFifo) {
    let sio = Sio::new(sio);
    let pins = hal::gpio::Pins::new(io_bank0, pads_bank0, sio.gpio_bank0, resets);
    (pins, sio.fifo)
}

/// Initialise UART0 for serial output.
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
pub(crate) fn init_delay(clocks: &hal::clocks::ClocksManager) -> cortex_m::delay::Delay {
    let core = cortex_m::Peripherals::take().unwrap();
    cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz())
}

/// Launch core 1 with its FIFO echo task.
pub(crate) fn spawn_core1(
    psm: &mut hal::pac::PSM,
    ppb: &mut hal::pac::PPB,
    fifo: &mut SioFifo,
) {
    let mut mc = Multicore::new(psm, ppb, fifo);
    let cores = mc.cores();
    let core1 = &mut cores[1];
    let _ = core1.spawn(CORE1_STACK.take().unwrap(), move || core1_entry());
}

/// Core 1 entry point: receive values via FIFO, increment, and return.
fn core1_entry() -> ! {
    let pac = unsafe { hal::pac::Peripherals::steal() };
    let sio = Sio::new(pac.SIO);
    let mut fifo = sio.fifo;
    loop {
        let value = fifo.read_blocking();
        fifo.write_blocking(multicore::increment_value(value));
    }
}

/// Send the counter to core 1 via FIFO and print the round-trip result.
pub(crate) fn send_and_print(
    fifo: &mut SioFifo,
    uart: &EnabledUart,
    counter: &mut u32,
    delay: &mut cortex_m::delay::Delay,
) {
    fifo.write_blocking(*counter);
    let response = fifo.read_blocking();
    let mut buf = [0u8; MSG_BUF_LEN];
    let n = multicore::format_round_trip(&mut buf, *counter, response);
    uart.write_full_blocking(&buf[..n]);
    *counter = counter.wrapping_add(1);
    delay.delay_ms(POLL_MS);
}
