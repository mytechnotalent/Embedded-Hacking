//! @file servo.rs
//! @brief Implementation of a simple SG90 servo driver (pure-logic helpers)
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

/// Default minimum pulse width in microseconds (0 degrees).
pub const SERVO_DEFAULT_MIN_US: u16 = 1000;

/// Default maximum pulse width in microseconds (180 degrees).
pub const SERVO_DEFAULT_MAX_US: u16 = 2000;

/// Default PWM wrap value for 50 Hz servo (20 000 - 1).
pub const SERVO_WRAP: u32 = 20000 - 1;

/// Default servo frequency in Hz.
pub const SERVO_HZ: f32 = 50.0;

/// Convert a pulse width in microseconds to a PWM counter level.
///
/// Uses the configured PWM wrap and servo frequency to map pulse time
/// into the channel compare value expected by the PWM hardware.
///
/// # Arguments
///
/// * `pulse_us` - Pulse width in microseconds.
/// * `wrap` - PWM counter wrap value.
/// * `hz` - PWM frequency in Hz.
///
/// # Returns
///
/// PWM level suitable for the channel compare register.
pub fn pulse_us_to_level(pulse_us: u32, wrap: u32, hz: f32) -> u32 {
    let period_us = 1_000_000.0f32 / hz;
    let counts_per_us = (wrap + 1) as f32 / period_us;
    (pulse_us as f32 * counts_per_us + 0.5f32) as u32
}

/// Clamp a pulse width to the valid servo range.
///
/// Values below min_us are raised to min_us; values above max_us are
/// lowered to max_us.
///
/// # Arguments
///
/// * `pulse_us` - Raw pulse width in microseconds.
/// * `min_us` - Minimum allowed pulse width.
/// * `max_us` - Maximum allowed pulse width.
///
/// # Returns
///
/// Clamped pulse width.
pub fn clamp_pulse_us(pulse_us: u16, min_us: u16, max_us: u16) -> u16 {
    if pulse_us < min_us {
        min_us
    } else if pulse_us > max_us {
        max_us
    } else {
        pulse_us
    }
}

/// Clamp a floating-point angle to the valid servo range [0.0, 180.0].
fn clamp_degrees(degrees: f32) -> f32 {
    if degrees < 0.0f32 {
        0.0f32
    } else if degrees > 180.0f32 {
        180.0f32
    } else {
        degrees
    }
}

/// Map a servo angle in degrees to a pulse width in microseconds.
///
/// Clamps degrees to [0, 180], then linearly maps to the pulse range.
///
/// # Arguments
///
/// * `degrees` - Angle in degrees (0.0 to 180.0).
/// * `min_us` - Pulse width at 0 degrees.
/// * `max_us` - Pulse width at 180 degrees.
///
/// # Returns
///
/// Pulse width in microseconds corresponding to the given angle.
pub fn angle_to_pulse_us(degrees: f32, min_us: u16, max_us: u16) -> u16 {
    let d = clamp_degrees(degrees);
    let ratio = d / 180.0f32;
    let span = (max_us - min_us) as f32;
    (min_us as f32 + ratio * span + 0.5f32) as u16
}

/// Compute the PWM clock divider for the servo frequency.
///
/// # Arguments
///
/// * `sys_hz` - System clock frequency in Hz.
/// * `servo_hz` - Desired servo PWM frequency in Hz.
/// * `wrap` - PWM counter wrap value.
///
/// # Returns
///
/// Clock divider value.
pub fn calc_clk_div(sys_hz: u32, servo_hz: f32, wrap: u32) -> f32 {
    sys_hz as f32 / (servo_hz * (wrap + 1) as f32)
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// Pulse us to level 1000us.
    #[test]
    fn pulse_us_to_level_1000us() {
        let level = pulse_us_to_level(1000, SERVO_WRAP, SERVO_HZ);
        assert_eq!(level, 1000);
    }

    /// Pulse us to level 2000us.
    #[test]
    fn pulse_us_to_level_2000us() {
        let level = pulse_us_to_level(2000, SERVO_WRAP, SERVO_HZ);
        assert_eq!(level, 2000);
    }

    /// Pulse us to level 1500us.
    #[test]
    fn pulse_us_to_level_1500us() {
        let level = pulse_us_to_level(1500, SERVO_WRAP, SERVO_HZ);
        assert_eq!(level, 1500);
    }

    /// Pulse us to level zero.
    #[test]
    fn pulse_us_to_level_zero() {
        let level = pulse_us_to_level(0, SERVO_WRAP, SERVO_HZ);
        assert_eq!(level, 0);
    }

    /// Clamp pulse us below min.
    #[test]
    fn clamp_pulse_us_below_min() {
        assert_eq!(
            clamp_pulse_us(500, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US),
            1000
        );
    }

    /// Clamp pulse us above max.
    #[test]
    fn clamp_pulse_us_above_max() {
        assert_eq!(
            clamp_pulse_us(3000, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US),
            2000
        );
    }

    /// Clamp pulse us within range.
    #[test]
    fn clamp_pulse_us_within_range() {
        assert_eq!(
            clamp_pulse_us(1500, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US),
            1500
        );
    }

    /// Angle to pulse us zero.
    #[test]
    fn angle_to_pulse_us_zero() {
        let pulse = angle_to_pulse_us(0.0, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
        assert_eq!(pulse, 1000);
    }

    /// Angle to pulse us 180.
    #[test]
    fn angle_to_pulse_us_180() {
        let pulse = angle_to_pulse_us(180.0, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
        assert_eq!(pulse, 2000);
    }

    /// Angle to pulse us 90.
    #[test]
    fn angle_to_pulse_us_90() {
        let pulse = angle_to_pulse_us(90.0, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
        assert_eq!(pulse, 1500);
    }

    /// Angle to pulse us clamped negative.
    #[test]
    fn angle_to_pulse_us_clamped_negative() {
        let pulse = angle_to_pulse_us(-10.0, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
        assert_eq!(pulse, 1000);
    }

    /// Angle to pulse us clamped above.
    #[test]
    fn angle_to_pulse_us_clamped_above() {
        let pulse = angle_to_pulse_us(200.0, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
        assert_eq!(pulse, 2000);
    }

    /// Calc clk div 150mhz.
    #[test]
    fn calc_clk_div_150mhz() {
        let div = calc_clk_div(150_000_000, SERVO_HZ, SERVO_WRAP);
        assert!((div - 150.0).abs() < 0.01);
    }
}

// End of file
