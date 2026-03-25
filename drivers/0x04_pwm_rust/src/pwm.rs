//! @file pwm.rs
//! @brief Implementation of the generic PWM output driver
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

/// Compute the PWM clock divider that yields the target frequency.
///
/// Derives a floating-point divider from the system clock frequency,
/// the desired output frequency, and the chosen wrap value so that the PWM
/// counter overflows exactly freq_hz times per second.
///
/// # Arguments
///
/// * `sys_hz` - System clock frequency in Hz.
/// * `freq_hz` - Desired PWM output frequency in Hz.
/// * `wrap_val` - Chosen PWM counter wrap value (period - 1).
///
/// # Returns
///
/// Clock divider to program into the PWM slice.
pub fn calc_clk_div(sys_hz: u32, freq_hz: u32, wrap_val: u32) -> f32 {
    sys_hz as f32 / (freq_hz as f32 * (wrap_val + 1) as f32)
}

/// Clamp a duty percentage to the valid 0–100 range.
///
/// Values above 100 are clamped to 100.
///
/// # Arguments
///
/// * `percent` - Raw duty cycle percentage.
///
/// # Returns
///
/// Clamped percentage in the range 0..=100.
pub fn clamp_percent(percent: u8) -> u8 {
    if percent > 100 { 100 } else { percent }
}

/// Map a duty percentage to a PWM channel level.
///
/// Converts a 0–100 percentage to the internal PWM counter range based
/// on the configured wrap value.
///
/// # Arguments
///
/// * `percent` - Duty cycle from 0 (always low) to 100 (always high).
/// * `wrap` - PWM counter wrap value (period - 1).
///
/// # Returns
///
/// Channel level value to write to the PWM slice.
pub fn duty_to_level(percent: u8, wrap: u32) -> u32 {
    let p = clamp_percent(percent);
    (p as u32 * (wrap + 1)) / 100
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    #[test]
    fn calc_clk_div_1khz() {
        let div = calc_clk_div(150_000_000, 1000, 9999);
        assert!((div - 15.0).abs() < 0.01);
    }

    #[test]
    fn calc_clk_div_10khz() {
        let div = calc_clk_div(150_000_000, 10000, 9999);
        assert!((div - 1.5).abs() < 0.01);
    }

    #[test]
    fn clamp_percent_within_range() {
        assert_eq!(clamp_percent(50), 50);
    }

    #[test]
    fn clamp_percent_at_100() {
        assert_eq!(clamp_percent(100), 100);
    }

    #[test]
    fn clamp_percent_above_100() {
        assert_eq!(clamp_percent(255), 100);
    }

    #[test]
    fn clamp_percent_zero() {
        assert_eq!(clamp_percent(0), 0);
    }

    #[test]
    fn duty_to_level_zero() {
        assert_eq!(duty_to_level(0, 9999), 0);
    }

    #[test]
    fn duty_to_level_100() {
        assert_eq!(duty_to_level(100, 9999), 10000);
    }

    #[test]
    fn duty_to_level_50() {
        assert_eq!(duty_to_level(50, 9999), 5000);
    }

    #[test]
    fn duty_to_level_clamped() {
        assert_eq!(duty_to_level(200, 9999), 10000);
    }

    #[test]
    fn duty_to_level_5() {
        assert_eq!(duty_to_level(5, 9999), 500);
    }
}

// End of file
