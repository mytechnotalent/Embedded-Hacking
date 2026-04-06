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

// Embedded HAL digital output traits for GPIO pin control
use embedded_hal::digital::{OutputPin, StatefulOutputPin};

/// GPIO output / LED blink driver that owns a single output pin.
pub struct BlinkDriver<P: OutputPin + StatefulOutputPin> {
    pin: P,
}

impl<P: OutputPin + StatefulOutputPin> BlinkDriver<P> {
    /// Initialize a GPIO pin as a push-pull digital output.
    ///
    /// The initial drive level is low (LED off).
    ///
    /// # Arguments
    ///
    /// * `pin` - GPIO pin configured as a digital output.
    ///
    /// # Returns
    ///
    /// A new BlinkDriver instance owning the configured pin.
    pub fn init(mut pin: P) -> Self {
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

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;
    // Infallible error type for mock pin implementations
    use core::convert::Infallible;
    // Error type trait for mock pin implementations
    use embedded_hal::digital::ErrorType;

    /// Mock GPIO pin for testing.
    struct MockPin {
        state: bool,
    }

    impl MockPin {
        /// New.
        fn new() -> Self {
            Self { state: false }
        }
    }

    /// ErrorType implementation for MockPin.
    impl ErrorType for MockPin {
        type Error = Infallible;
    }

    /// OutputPin implementation for MockPin.
    impl OutputPin for MockPin {
        /// Set low.
        fn set_low(&mut self) -> Result<(), Self::Error> {
            self.state = false;
            Ok(())
        }
        /// Set high.
        fn set_high(&mut self) -> Result<(), Self::Error> {
            self.state = true;
            Ok(())
        }
    }

    /// StatefulOutputPin implementation for MockPin.
    impl StatefulOutputPin for MockPin {
        /// Is set high.
        fn is_set_high(&mut self) -> Result<bool, Self::Error> {
            Ok(self.state)
        }
        /// Is set low.
        fn is_set_low(&mut self) -> Result<bool, Self::Error> {
            Ok(!self.state)
        }
        /// Toggle.
        fn toggle(&mut self) -> Result<(), Self::Error> {
            self.state = !self.state;
            Ok(())
        }
    }

    /// Init starts low.
    #[test]
    fn init_starts_low() {
        let drv = BlinkDriver::init(MockPin::new());
        assert!(!drv.pin.state);
    }

    /// On sets high.
    #[test]
    fn on_sets_high() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.on();
        assert!(drv.pin.state);
    }

    /// Off sets low.
    #[test]
    fn off_sets_low() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.on();
        drv.off();
        assert!(!drv.pin.state);
    }

    /// Toggle from low goes high.
    #[test]
    fn toggle_from_low_goes_high() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.toggle();
        assert!(drv.pin.state);
    }

    /// Toggle from high goes low.
    #[test]
    fn toggle_from_high_goes_low() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.on();
        drv.toggle();
        assert!(!drv.pin.state);
    }

    /// Get state reflects on.
    #[test]
    fn get_state_reflects_on() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.on();
        assert!(drv.get_state());
    }

    /// Get state reflects off.
    #[test]
    fn get_state_reflects_off() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.on();
        drv.off();
        assert!(!drv.get_state());
    }

    /// Double toggle returns to original.
    #[test]
    fn double_toggle_returns_to_original() {
        let mut drv = BlinkDriver::init(MockPin::new());
        drv.toggle();
        drv.toggle();
        assert!(!drv.get_state());
    }
}

// End of file
