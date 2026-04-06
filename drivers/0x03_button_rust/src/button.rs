//! @file button.rs
//! @brief Implementation of the push-button GPIO input driver with debounce
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

// Embedded HAL digital I/O traits for GPIO pin control
use embedded_hal::digital::{InputPin, OutputPin, StatefulOutputPin};

/// Push-button GPIO input driver with debounce.
pub struct ButtonDriver<B: InputPin, D: FnMut(u32)> {
    btn_pin: B,
    debounce_ms: u32,
    delay_fn: D,
}

impl<B: InputPin, D: FnMut(u32)> ButtonDriver<B, D> {
    /// Initialize a GPIO pin as an active-low button input with pull-up.
    ///
    /// The pin reads logic high when the button is open and logic low when
    /// the button connects the pin to GND.
    ///
    /// # Arguments
    ///
    /// * `btn_pin` - GPIO pin configured as input with pull-up.
    /// * `debounce_ms` - Debounce settling time in milliseconds (e.g. 20).
    /// * `delay_fn` - Closure that sleeps for the given number of milliseconds.
    ///
    /// # Returns
    ///
    /// A new ButtonDriver instance.
    pub fn init(btn_pin: B, debounce_ms: u32, delay_fn: D) -> Self {
        Self { btn_pin, debounce_ms, delay_fn }
    }

    /// Confirm a raw active-low pin read by waiting for the debounce period.
    ///
    /// After an initial low reading on the pin, this helper re-samples the pin
    /// after debounce_ms milliseconds to confirm the reading is stable.
    ///
    /// # Returns
    ///
    /// `true` if the pin is still low after the debounce delay.
    fn debounce_confirm(&mut self) -> bool {
        (self.delay_fn)(self.debounce_ms);
        self.btn_pin.is_low().unwrap()
    }

    /// Read the debounced state of the button.
    ///
    /// Returns true only when the pin reads low continuously for the debounce
    /// period configured in init(). This prevents mechanical contact bounce
    /// from registering as multiple rapid presses.
    ///
    /// # Returns
    ///
    /// `true` if the button is firmly pressed, `false` if released.
    pub fn is_pressed(&mut self) -> bool {
        if self.btn_pin.is_low().unwrap() {
            return self.debounce_confirm();
        }
        false
    }
}

/// Indicator LED driver that owns a single output pin.
pub struct ButtonLed<L: OutputPin + StatefulOutputPin> {
    led_pin: L,
}

impl<L: OutputPin + StatefulOutputPin> ButtonLed<L> {
    /// Initialize a GPIO pin as a push-pull digital output for an indicator LED.
    ///
    /// Configures the output and drives it low (LED off).
    ///
    /// # Arguments
    ///
    /// * `led_pin` - GPIO pin configured as a digital output.
    ///
    /// # Returns
    ///
    /// A new ButtonLed instance.
    pub fn init(mut led_pin: L) -> Self {
        led_pin.set_low().unwrap();
        Self { led_pin }
    }

    /// Set the indicator LED state.
    ///
    /// Drives the output pin high (LED on) or low (LED off).
    ///
    /// # Arguments
    ///
    /// * `on` - `true` to turn the LED on, `false` to turn it off.
    pub fn set(&mut self, on: bool) {
        if on {
            self.led_pin.set_high().unwrap();
        } else {
            self.led_pin.set_low().unwrap();
        }
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

    /// Mock input pin for testing.
    struct MockInputPin {
        low: bool,
    }

    impl MockInputPin {
        /// New.
        fn new(low: bool) -> Self {
            Self { low }
        }
    }

    /// ErrorType implementation for MockInputPin.
    impl ErrorType for MockInputPin {
        type Error = Infallible;
    }

    /// InputPin implementation for MockInputPin.
    impl InputPin for MockInputPin {
        /// Is high.
        fn is_high(&mut self) -> Result<bool, Self::Error> {
            Ok(!self.low)
        }
        /// Is low.
        fn is_low(&mut self) -> Result<bool, Self::Error> {
            Ok(self.low)
        }
    }

    /// Mock output pin for testing.
    struct MockOutputPin {
        state: bool,
    }

    impl MockOutputPin {
        /// New.
        fn new() -> Self {
            Self { state: false }
        }
    }

    /// ErrorType implementation for MockOutputPin.
    impl ErrorType for MockOutputPin {
        type Error = Infallible;
    }

    /// OutputPin implementation for MockOutputPin.
    impl OutputPin for MockOutputPin {
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

    /// StatefulOutputPin implementation for MockOutputPin.
    impl StatefulOutputPin for MockOutputPin {
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

    /// Noop delay.
    fn noop_delay(_ms: u32) {}

    /// Is pressed when low and confirmed.
    #[test]
    fn is_pressed_when_low_and_confirmed() {
        let pin = MockInputPin::new(true);
        let mut drv = ButtonDriver::init(pin, 20, noop_delay);
        assert!(drv.is_pressed());
    }

    /// Is not pressed when high.
    #[test]
    fn is_not_pressed_when_high() {
        let pin = MockInputPin::new(false);
        let mut drv = ButtonDriver::init(pin, 20, noop_delay);
        assert!(!drv.is_pressed());
    }

    /// Debounce ms stored.
    #[test]
    fn debounce_ms_stored() {
        let pin = MockInputPin::new(false);
        let drv = ButtonDriver::init(pin, 50, noop_delay);
        assert_eq!(drv.debounce_ms, 50);
    }

    /// Debounce calls delay.
    #[test]
    fn debounce_calls_delay() {
        let mut called_with: u32 = 0;
        let pin = MockInputPin::new(true);
        let mut drv = ButtonDriver::init(pin, 25, |ms| called_with = ms);
        drv.is_pressed();
        assert_eq!(called_with, 25);
    }

    /// Led init starts off.
    #[test]
    fn led_init_starts_off() {
        let led = ButtonLed::init(MockOutputPin::new());
        assert!(!led.led_pin.state);
    }

    /// Led set on.
    #[test]
    fn led_set_on() {
        let mut led = ButtonLed::init(MockOutputPin::new());
        led.set(true);
        assert!(led.led_pin.state);
    }

    /// Led set off.
    #[test]
    fn led_set_off() {
        let mut led = ButtonLed::init(MockOutputPin::new());
        led.set(true);
        led.set(false);
        assert!(!led.led_pin.state);
    }

    /// Led set on then off then on.
    #[test]
    fn led_set_on_then_off_then_on() {
        let mut led = ButtonLed::init(MockOutputPin::new());
        led.set(true);
        led.set(false);
        led.set(true);
        assert!(led.led_pin.state);
    }
}

// End of file
