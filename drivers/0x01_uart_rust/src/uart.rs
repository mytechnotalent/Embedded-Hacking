//! @file uart.rs
//! @brief Implementation of the hardware UART0 driver
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
// Non-blocking I/O helper for UART read/write
use nb::block;
// Alias our HAL crate
use rp235x_hal as hal;
// Clock trait for accessing system clock frequency
use hal::Clock;
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
// UART0 peripheral singleton type
use hal::pac::UART0;

/// Type alias for the configured TX pin (GPIO 0, UART function, no pull).
type TxPin = Pin<hal::gpio::bank0::Gpio0, FunctionUart, PullNone>;

/// Type alias for the configured RX pin (GPIO 1, UART function, no pull).
type RxPin = Pin<hal::gpio::bank0::Gpio1, FunctionUart, PullNone>;

/// Type alias for the default TX pin state from `Pins::new()`.
type TxPinDefault = Pin<hal::gpio::bank0::Gpio0, FunctionNull, PullDown>;

/// Type alias for the default RX pin state from `Pins::new()`.
type RxPinDefault = Pin<hal::gpio::bank0::Gpio1, FunctionNull, PullDown>;

/// Type alias for the fully-enabled UART0 peripheral with TX/RX pins.
type EnabledUart = UartPeripheral<Enabled, UART0, (TxPin, RxPin)>;

/// Blocking UART0 driver that owns the enabled peripheral and its pins.
pub struct UartDriver {
    uart: EnabledUart,
}

impl UartDriver {
    /// Initialize hardware UART0 on the specified TX and RX GPIO pins.
    ///
    /// Configures UART0 at the requested baud rate, reconfigures the GPIO
    /// pins for UART function, and enables 8N1 framing.
    ///
    /// # Arguments
    ///
    /// * `uart0` - PAC UART0 peripheral singleton.
    /// * `tx_pin` - GPIO pin to use as UART0 TX (typically GPIO 0).
    /// * `rx_pin` - GPIO pin to use as UART0 RX (typically GPIO 1).
    /// * `baud_rate` - Desired baud rate in bits per second (e.g. 115200).
    /// * `resets` - Mutable reference to the RESETS peripheral.
    /// * `clocks` - Reference to the initialised clock configuration.
    ///
    /// # Panics
    ///
    /// Panics if the HAL cannot achieve the requested baud rate.
    pub fn init(
        uart0: UART0,
        tx_pin: TxPinDefault,
        rx_pin: RxPinDefault,
        baud_rate: u32,
        resets: &mut hal::pac::RESETS,
        clocks: &hal::clocks::ClocksManager,
    ) -> Self {
        let pins = (
            tx_pin.reconfigure::<FunctionUart, PullNone>(),
            rx_pin.reconfigure::<FunctionUart, PullNone>(),
        );
        let cfg = UartConfig::new(baud_rate.Hz(), DataBits::Eight, None, StopBits::One);
        let uart = UartPeripheral::new(uart0, pins, resets)
            .enable(cfg, clocks.peripheral_clock.freq())
            .unwrap();
        Self { uart }
    }

    /// Check whether a received character is waiting in the UART FIFO.
    ///
    /// Returns immediately without blocking.
    ///
    /// # Returns
    ///
    /// `true` if at least one byte is available to read, `false` otherwise.
    pub fn is_readable(&self) -> bool {
        self.uart.uart_is_readable()
    }

    /// Read one character from UART0 (blocking).
    ///
    /// Blocks until a byte arrives in the receive FIFO, then returns it.
    ///
    /// # Returns
    ///
    /// The received byte.
    pub fn getchar(&mut self) -> u8 {
        block!(embedded_hal_nb::serial::Read::read(&mut self.uart)).unwrap()
    }

    /// Transmit one character over UART0 (blocking).
    ///
    /// Waits until the transmit FIFO has space, then places the byte into
    /// the FIFO.
    ///
    /// # Arguments
    ///
    /// * `c` - Byte to transmit.
    pub fn putchar(&mut self, c: u8) {
        block!(embedded_hal_nb::serial::Write::write(&mut self.uart, c)).unwrap();
    }

    /// Transmit a null-terminated string over UART0.
    ///
    /// Sends every byte in the slice, blocking until all bytes are sent.
    ///
    /// # Arguments
    ///
    /// * `s` - Byte slice to transmit.
    pub fn puts(&mut self, s: &[u8]) {
        self.uart.write_full_blocking(s);
    }

    /// Convert a lowercase ASCII character to uppercase.
    ///
    /// Returns the uppercase equivalent if the character is in `b'a'`–`b'z'`;
    /// all other characters are passed through unchanged.
    ///
    /// # Arguments
    ///
    /// * `c` - Input byte.
    ///
    /// # Returns
    ///
    /// Uppercase equivalent, or the original byte.
    pub fn to_upper(c: u8) -> u8 {
        if c >= b'a' && c <= b'z' {
            c - 32
        } else {
            c
        }
    }
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    #[test]
    fn to_upper_lowercase_a() {
        assert_eq!(UartDriver::to_upper(b'a'), b'A');
    }

    #[test]
    fn to_upper_lowercase_z() {
        assert_eq!(UartDriver::to_upper(b'z'), b'Z');
    }

    #[test]
    fn to_upper_lowercase_m() {
        assert_eq!(UartDriver::to_upper(b'm'), b'M');
    }

    #[test]
    fn to_upper_already_uppercase() {
        assert_eq!(UartDriver::to_upper(b'A'), b'A');
    }

    #[test]
    fn to_upper_digit_unchanged() {
        assert_eq!(UartDriver::to_upper(b'5'), b'5');
    }

    #[test]
    fn to_upper_space_unchanged() {
        assert_eq!(UartDriver::to_upper(b' '), b' ');
    }

    #[test]
    fn to_upper_newline_unchanged() {
        assert_eq!(UartDriver::to_upper(b'\n'), b'\n');
    }

    #[test]
    fn to_upper_null_unchanged() {
        assert_eq!(UartDriver::to_upper(0), 0);
    }
}
