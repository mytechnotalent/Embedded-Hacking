//! @file watchdog.rs
//! @brief Pure-logic watchdog timer driver (host-testable, no HAL)
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

/// Default watchdog timeout used in the C demo (3000 ms).
pub const DEFAULT_TIMEOUT_MS: u32 = 3_000;

/// Feed interval used in the C demo (1000 ms).
pub const FEED_INTERVAL_MS: u32 = 1_000;

/// Maximum hardware watchdog timeout in milliseconds (8388 ms).
pub const MAX_TIMEOUT_MS: u32 = 8_388;

/// UART message for "Watchdog fed\r\n" matching the C demo output.
pub const WATCHDOG_FED_MSG: &[u8] = b"Watchdog fed\r\n";

/// UART message for watchdog reboot detection.
pub const REBOOT_MSG: &[u8] = b"System rebooted by watchdog timeout\r\n";

/// UART message for normal power-on reset.
pub const NORMAL_RESET_MSG: &[u8] = b"Normal power-on reset\r\n";

/// Pure-logic watchdog driver state.
///
/// Tracks whether the watchdog has been enabled, its configured timeout,
/// and the cumulative number of feeds. The actual hardware watchdog
/// enable / feed / reboot-check calls live in `board.rs`.
pub struct WatchdogDriverState {
    /// Whether the watchdog has been enabled.
    enabled: bool,
    /// Configured timeout in milliseconds.
    timeout_ms: u32,
    /// Number of times the watchdog has been fed.
    feed_count: u32,
}

impl WatchdogDriverState {
    /// Create a new watchdog driver state (disabled, zero timeout).
    ///
    /// # Returns
    ///
    /// A `WatchdogDriverState` with the watchdog disabled.
    pub fn new() -> Self {
        Self {
            enabled: false,
            timeout_ms: 0,
            feed_count: 0,
        }
    }

    /// Enable the watchdog with the given timeout.
    ///
    /// If the watchdog is already enabled, this updates the timeout and
    /// resets the feed count.
    ///
    /// # Arguments
    ///
    /// * `timeout_ms` - Timeout in milliseconds (1–8388).
    pub fn enable(&mut self, timeout_ms: u32) {
        self.enabled = true;
        self.timeout_ms = timeout_ms;
        self.feed_count = 0;
    }

    /// Feed the watchdog, incrementing the feed counter.
    ///
    /// Returns `true` if the watchdog is enabled and the feed was
    /// accepted, `false` if the watchdog is not enabled.
    ///
    /// # Returns
    ///
    /// `true` if the watchdog was fed, `false` otherwise.
    pub fn feed(&mut self) -> bool {
        if self.enabled {
            self.feed_count += 1;
            true
        } else {
            false
        }
    }

    /// Check whether the watchdog is currently enabled.
    ///
    /// # Returns
    ///
    /// `true` if the watchdog has been enabled.
    pub fn is_enabled(&self) -> bool {
        self.enabled
    }

    /// Return the configured timeout in milliseconds.
    ///
    /// # Returns
    ///
    /// The timeout value passed to `enable()`, or 0 if never enabled.
    pub fn timeout_ms(&self) -> u32 {
        self.timeout_ms
    }

    /// Return the total number of times the watchdog has been fed.
    ///
    /// # Returns
    ///
    /// Cumulative feed count since the last `enable()`.
    pub fn feed_count(&self) -> u32 {
        self.feed_count
    }
}

/// Format the "Watchdog fed\r\n" message into `buf`.
///
/// # Arguments
///
/// * `buf` - Output buffer (must be >= 14 bytes).
///
/// # Returns
///
/// Number of bytes written.
pub fn format_fed(buf: &mut [u8]) -> usize {
    let msg = WATCHDOG_FED_MSG;
    let n = msg.len().min(buf.len());
    buf[..n].copy_from_slice(&msg[..n]);
    n
}

/// Format the reset-reason message into `buf`.
///
/// If `caused_reboot` is `true`, writes `REBOOT_MSG`; otherwise writes
/// `NORMAL_RESET_MSG`.
///
/// # Arguments
///
/// * `buf` - Output buffer.
/// * `caused_reboot` - Whether the watchdog triggered the last reset.
///
/// # Returns
///
/// Number of bytes written.
pub fn format_reset_reason(buf: &mut [u8], caused_reboot: bool) -> usize {
    let msg = if caused_reboot { REBOOT_MSG } else { NORMAL_RESET_MSG };
    let n = msg.len().min(buf.len());
    buf[..n].copy_from_slice(&msg[..n]);
    n
}

/// Format the "Watchdog enabled (XXXXs timeout). Feeding every 1s...\r\n"
/// banner message into `buf`.
///
/// # Arguments
///
/// * `buf` - Output buffer (should be >= 64 bytes).
/// * `timeout_ms` - Timeout in milliseconds.
///
/// # Returns
///
/// Number of bytes written.
pub fn format_enabled(buf: &mut [u8], timeout_ms: u32) -> usize {
    let prefix = b"Watchdog enabled (";
    let suffix = b"s timeout). Feeding every 1s...\r\n";
    let mut pos = copy_slice(buf, 0, prefix);
    pos += format_u32(&mut buf[pos..], timeout_ms / 1000);
    pos += copy_slice(buf, pos, suffix);
    pos
}

/// Copy a byte slice into `buf` at the given offset, returning bytes written.
fn copy_slice(buf: &mut [u8], offset: usize, src: &[u8]) -> usize {
    let n = src.len().min(buf.len().saturating_sub(offset));
    buf[offset..offset + n].copy_from_slice(&src[..n]);
    n
}

/// Format a `u32` as decimal ASCII into `buf`.
///
/// # Arguments
///
/// * `buf` - Output buffer.
/// * `value` - The value to format.
///
/// # Returns
///
/// Number of bytes written.
pub fn format_u32(buf: &mut [u8], value: u32) -> usize {
    if value == 0 {
        if !buf.is_empty() {
            buf[0] = b'0';
            return 1;
        }
        return 0;
    }
    let mut tmp = [0u8; 10];
    let n = u32_to_digits_reversed(&mut tmp, value);
    reverse_copy(buf, &tmp, n);
    n
}

/// Convert a u32 to reversed decimal digits in a temporary buffer.
fn u32_to_digits_reversed(tmp: &mut [u8; 10], mut value: u32) -> usize {
    let mut i = 0usize;
    while value > 0 {
        tmp[i] = b'0' + (value % 10) as u8;
        value /= 10;
        i += 1;
    }
    i
}

/// Copy digits from a reversed temporary buffer into the output buffer.
fn reverse_copy(buf: &mut [u8], tmp: &[u8], n: usize) {
    let count = n.min(buf.len());
    for j in 0..count {
        buf[j] = tmp[n - 1 - j];
    }
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    #[test]
    fn new_state_is_disabled() {
        let state = WatchdogDriverState::new();
        assert!(!state.is_enabled());
        assert_eq!(state.timeout_ms(), 0);
        assert_eq!(state.feed_count(), 0);
    }

    #[test]
    fn enable_activates_watchdog() {
        let mut state = WatchdogDriverState::new();
        state.enable(3000);
        assert!(state.is_enabled());
        assert_eq!(state.timeout_ms(), 3000);
        assert_eq!(state.feed_count(), 0);
    }

    #[test]
    fn feed_increments_count() {
        let mut state = WatchdogDriverState::new();
        state.enable(3000);
        assert!(state.feed());
        assert_eq!(state.feed_count(), 1);
        assert!(state.feed());
        assert_eq!(state.feed_count(), 2);
    }

    #[test]
    fn feed_returns_false_when_disabled() {
        let mut state = WatchdogDriverState::new();
        assert!(!state.feed());
        assert_eq!(state.feed_count(), 0);
    }

    #[test]
    fn enable_resets_feed_count() {
        let mut state = WatchdogDriverState::new();
        state.enable(3000);
        state.feed();
        state.feed();
        assert_eq!(state.feed_count(), 2);
        state.enable(5000);
        assert_eq!(state.feed_count(), 0);
        assert_eq!(state.timeout_ms(), 5000);
    }

    #[test]
    fn default_timeout_matches_c_demo() {
        assert_eq!(DEFAULT_TIMEOUT_MS, 3000);
    }

    #[test]
    fn feed_interval_matches_c_demo() {
        assert_eq!(FEED_INTERVAL_MS, 1000);
    }

    #[test]
    fn max_timeout_is_8388() {
        assert_eq!(MAX_TIMEOUT_MS, 8388);
    }

    #[test]
    fn format_fed_matches_c_output() {
        let mut buf = [0u8; 32];
        let n = format_fed(&mut buf);
        assert_eq!(&buf[..n], b"Watchdog fed\r\n");
    }

    #[test]
    fn format_reset_reason_watchdog() {
        let mut buf = [0u8; 64];
        let n = format_reset_reason(&mut buf, true);
        assert_eq!(&buf[..n], b"System rebooted by watchdog timeout\r\n");
    }

    #[test]
    fn format_reset_reason_normal() {
        let mut buf = [0u8; 64];
        let n = format_reset_reason(&mut buf, false);
        assert_eq!(&buf[..n], b"Normal power-on reset\r\n");
    }

    #[test]
    fn format_enabled_3s() {
        let mut buf = [0u8; 64];
        let n = format_enabled(&mut buf, 3000);
        assert_eq!(
            &buf[..n],
            b"Watchdog enabled (3s timeout). Feeding every 1s...\r\n"
        );
    }

    #[test]
    fn format_enabled_5s() {
        let mut buf = [0u8; 64];
        let n = format_enabled(&mut buf, 5000);
        assert_eq!(
            &buf[..n],
            b"Watchdog enabled (5s timeout). Feeding every 1s...\r\n"
        );
    }

    #[test]
    fn format_u32_zero() {
        let mut buf = [0u8; 16];
        let n = format_u32(&mut buf, 0);
        assert_eq!(&buf[..n], b"0");
    }

    #[test]
    fn format_u32_large_value() {
        let mut buf = [0u8; 16];
        let n = format_u32(&mut buf, 12345);
        assert_eq!(&buf[..n], b"12345");
    }
}

// End of file
