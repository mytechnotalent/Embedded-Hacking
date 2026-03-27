//! @file board.rs
//! @brief Board-level HAL helpers for the SPI driver
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

// SPI pure-logic helpers and constants
use crate::spi;
// Digital output trait for software-controlled chip select
use embedded_hal::digital::OutputPin;
// SPI bus trait for full-duplex transfer
use embedded_hal::spi::SpiBus;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionSio, FunctionSpi, FunctionUart, Pin, PullDown, PullNone};
// SPI peripheral type
use hal::spi::Spi;
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

/// Delay between loopback transfers in milliseconds.
pub(crate) const POLL_MS: u32 = 1_000;

/// Type alias for the configured TX pin (GPIO 0, UART function, no pull).
pub(crate) type TxPin = Pin<hal::gpio::bank0::Gpio0, FunctionUart, PullNone>;

/// Type alias for the configured RX pin (GPIO 1, UART function, no pull).
pub(crate) type RxPin = Pin<hal::gpio::bank0::Gpio1, FunctionUart, PullNone>;

/// Type alias for the default TX pin state from `Pins::new()`.
pub(crate) type TxPinDefault = Pin<hal::gpio::bank0::Gpio0, FunctionNull, PullDown>;

/// Type alias for the default RX pin state from `Pins::new()`.
pub(crate) type RxPinDefault = Pin<hal::gpio::bank0::Gpio1, FunctionNull, PullDown>;

/// Type alias for the configured SPI RX pin (GPIO16).
pub(crate) type MisoPin = Pin<hal::gpio::bank0::Gpio16, FunctionSpi, PullNone>;

/// Type alias for the configured SPI chip-select pin (GPIO17).
pub(crate) type CsPin = Pin<hal::gpio::bank0::Gpio17, FunctionSio<hal::gpio::SioOutput>, PullNone>;

/// Type alias for the configured SPI clock pin (GPIO18).
pub(crate) type SckPin = Pin<hal::gpio::bank0::Gpio18, FunctionSpi, PullNone>;

/// Type alias for the configured SPI TX pin (GPIO19).
pub(crate) type MosiPin = Pin<hal::gpio::bank0::Gpio19, FunctionSpi, PullNone>;

/// Type alias for the fully-enabled UART0 peripheral with TX/RX pins.
pub(crate) type EnabledUart = UartPeripheral<Enabled, hal::pac::UART0, (TxPin, RxPin)>;

/// Type alias for the SPI0 peripheral configured in 8-bit master mode.
pub(crate) type EnabledSpi = Spi<
    hal::spi::Enabled,
    hal::pac::SPI0,
    (MosiPin, MisoPin, SckPin),
    8,
>;

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

/// Unlock the GPIO bank and return the pin set.
pub(crate) fn init_pins(
    io_bank0: hal::pac::IO_BANK0,
    pads_bank0: hal::pac::PADS_BANK0,
    sio: hal::pac::SIO,
    resets: &mut hal::pac::RESETS,
) -> hal::gpio::Pins {
    let sio = hal::Sio::new(sio);
    hal::gpio::Pins::new(io_bank0, pads_bank0, sio.gpio_bank0, resets)
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

/// Configure SPI0 and the software-controlled chip-select pin.
pub(crate) fn init_spi(
    spi0: hal::pac::SPI0,
    miso: hal::gpio::Pin<hal::gpio::bank0::Gpio16, FunctionNull, PullDown>,
    cs: hal::gpio::Pin<hal::gpio::bank0::Gpio17, FunctionNull, PullDown>,
    sck: hal::gpio::Pin<hal::gpio::bank0::Gpio18, FunctionNull, PullDown>,
    mosi: hal::gpio::Pin<hal::gpio::bank0::Gpio19, FunctionNull, PullDown>,
    resets: &mut hal::pac::RESETS,
    clocks: &hal::clocks::ClocksManager,
) -> (EnabledSpi, CsPin) {
    let miso = miso.reconfigure::<FunctionSpi, PullNone>();
    let sck = sck.reconfigure::<FunctionSpi, PullNone>();
    let mosi = mosi.reconfigure::<FunctionSpi, PullNone>();
    let mut cs = cs.reconfigure::<FunctionSio<hal::gpio::SioOutput>, PullNone>();
    let _ = cs.set_high();
    let spi = Spi::<_, _, _, 8>::new(spi0, (mosi, miso, sck)).init(
        resets,
        clocks.peripheral_clock.freq(),
        spi::SPI_BAUD_HZ.Hz(),
        embedded_hal::spi::MODE_0,
    );
    (spi, cs)
}

/// Drive chip select active (low).
fn cs_select(cs: &mut CsPin) {
    let _ = cs.set_low();
}

/// Drive chip select inactive (high).
fn cs_deselect(cs: &mut CsPin) {
    let _ = cs.set_high();
}

/// Perform one SPI loopback transfer and print TX/RX text over UART.
pub(crate) fn loopback_transfer(
    spi_dev: &mut EnabledSpi,
    cs: &mut CsPin,
    uart: &EnabledUart,
    delay: &mut cortex_m::delay::Delay,
) {
    let tx = spi::TX_MESSAGE;
    let mut rx = [0u8; spi::TX_MESSAGE.len()];
    cs_select(cs);
    let _ = spi_dev.transfer(&mut rx, tx);
    cs_deselect(cs);

    let mut line_buf = [0u8; 24];
    let tx_len = spi::format_tx_line(&mut line_buf, tx);
    uart.write_full_blocking(&line_buf[..tx_len]);
    let rx_len = spi::format_rx_line(&mut line_buf, &rx);
    uart.write_full_blocking(&line_buf[..rx_len]);
    spi::clear_rx_buffer(&mut rx);
    delay.delay_ms(POLL_MS);
}