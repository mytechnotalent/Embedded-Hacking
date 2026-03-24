//! @file blink.rs
//! @brief Implementation of the GPIO output / LED blink driver
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

use embedded_hal::digital::{OutputPin, StatefulOutputPin};
use rp235x_hal as hal;
use hal::gpio::{FunctionNull, FunctionSio, FunctionSioOutput, Pin, PullDown, SioOutput, ValidFunction};

/// GPIO output / LED blink driver that owns a single output pin.
pub struct BlinkDriver<I: hal::gpio::PinId> {
    pin: Pin<I, FunctionSioOutput, PullDown>,
}

impl<I: hal::gpio::PinId + ValidFunction<FunctionSio<SioOutput>>> BlinkDriver<I> {
    /// Initialize a GPIO pin as a push-pull digital output.
    ///
    /// Calls into_push_pull_output() to configure the pin for output.
    /// The initial drive level is low (LED off).
    ///
    /// # Arguments
    ///
    /// * `pin` - GPIO pin number to configure as a digital output.
    ///
    /// # Returns
    ///
    /// A new BlinkDriver instance owning the configured pin.
    pub fn init(pin: Pin<I, FunctionNull, PullDown>) -> Self {
        let mut pin = pin.into_push_pull_output();
        pin.set_low().unwrap();
        Self { pin }
    }

    /// Drive the output pin high (LED on).
    ///
    /// # Arguments
    ///
    /// * `self` - Mutable reference to the BlinkDriver.
    pub fn on(&mut self) {
        self.pin.set_high().unwrap();
    }

    /// Drive the output pin low (LED off).
    ///
    /// # Arguments
    ///
    /// * `self` - Mutable reference to the BlinkDriver.
    pub fn off(&mut self) {
        self.pin.set_low().unwrap();
    }

    /// Toggle the current state of the output pin.
    ///
    /// Reads the current GPIO output level and inverts it. If the LED was on
    /// it is turned off, and vice versa.
    ///
    /// # Arguments
    ///
    /// * `self` - Mutable reference to the BlinkDriver.
    pub fn toggle(&mut self) {
        self.pin.toggle().unwrap();
    }

    /// Query the current drive state of the output pin.
    ///
    /// # Arguments
    ///
    /// * `self` - Mutable reference to the BlinkDriver.
    ///
    /// # Returns
    ///
    /// `true` if the pin is driven high, `false` if driven low.
    pub fn get_state(&mut self) -> bool {
        self.pin.is_set_high().unwrap()
    }
}

// End of file
