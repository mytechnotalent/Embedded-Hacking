//! @file adc.rs
//! @brief Implementation of the 12-bit ADC driver (pure-logic helpers)
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

/// ADC reference voltage in millivolts.
pub const ADC_VREF_MV: u32 = 3300;

/// ADC full-scale value for 12-bit resolution.
pub const ADC_FULL_SCALE: u32 = 4095;

/// Convert a raw 12-bit ADC value to millivolts.
///
/// Scales the raw value linearly against the 3.3 V reference.
///
/// # Arguments
///
/// * `raw` - 12-bit ADC conversion result (0–4095).
///
/// # Returns
///
/// Equivalent voltage in millivolts (0–3300).
pub fn raw_to_mv(raw: u16) -> u32 {
    raw as u32 * ADC_VREF_MV / ADC_FULL_SCALE
}

/// Convert a raw temperature-sensor ADC value to degrees Celsius.
///
/// Applies the RP2350 datasheet formula:
///   T = 27 - (V - 0.706) / 0.001721
///
/// # Arguments
///
/// * `raw` - 12-bit ADC result from the internal temperature sensor (channel 4).
///
/// # Returns
///
/// Die temperature in degrees Celsius.
pub fn raw_to_celsius(raw: u16) -> f32 {
    let voltage = raw as f32 * 3.3f32 / ADC_FULL_SCALE as f32;
    27.0f32 - (voltage - 0.706f32) / 0.001721f32
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// Raw to mv zero.
    #[test]
    fn raw_to_mv_zero() {
        assert_eq!(raw_to_mv(0), 0);
    }

    /// Raw to mv full scale.
    #[test]
    fn raw_to_mv_full_scale() {
        assert_eq!(raw_to_mv(4095), 3300);
    }

    /// Raw to mv half.
    #[test]
    fn raw_to_mv_half() {
        let mv = raw_to_mv(2048);
        assert!(mv >= 1649 && mv <= 1651);
    }

    /// Raw to mv quarter.
    #[test]
    fn raw_to_mv_quarter() {
        let mv = raw_to_mv(1024);
        assert!(mv >= 824 && mv <= 826);
    }

    /// Raw to celsius room temp.
    #[test]
    fn raw_to_celsius_room_temp() {
        let temp = raw_to_celsius(876);
        assert!(temp > 20.0 && temp < 35.0);
    }

    /// Raw to celsius known voltage.
    #[test]
    fn raw_to_celsius_known_voltage() {
        let raw = (0.706f32 / 3.3f32 * ADC_FULL_SCALE as f32 + 0.5f32) as u16;
        let temp = raw_to_celsius(raw);
        assert!((temp - 27.0).abs() < 1.0);
    }

    /// Raw to celsius higher voltage.
    #[test]
    fn raw_to_celsius_higher_voltage() {
        let temp_low = raw_to_celsius(1000);
        let temp_high = raw_to_celsius(800);
        assert!(temp_high > temp_low);
    }

    /// Raw to mv one count.
    #[test]
    fn raw_to_mv_one_count() {
        assert_eq!(raw_to_mv(1), 0);
    }

    /// Raw to mv ten counts.
    #[test]
    fn raw_to_mv_ten_counts() {
        assert_eq!(raw_to_mv(10), 8);
    }
}

// End of file
