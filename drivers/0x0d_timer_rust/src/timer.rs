//! @file timer.rs
//! @brief Implementation of the repeating timer driver
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

/// Default heartbeat message printed by the timer callback.
pub const HEARTBEAT_MSG: &[u8] = b"Timer heartbeat\r\n";

/// Default timer period in milliseconds.
pub const DEFAULT_PERIOD_MS: u32 = 1_000;

/// Timer driver state tracking whether the repeating timer is active.
///
/// Mirrors the C driver's `g_timer_active` / `g_user_callback` globals
/// as a structured state machine testable on the host.
pub struct TimerDriverState {
    /// Whether the repeating timer is currently active.
    active: bool,
    /// Configured period in milliseconds.
    period_ms: u32,
    /// Number of times the callback has fired.
    fire_count: u32,
}

impl TimerDriverState {
    /// Create a new idle timer driver state.
    ///
    /// # Returns
    ///
    /// A `TimerDriverState` in the inactive state with zero fire count.
    pub fn new() -> Self {
        Self {
            active: false,
            period_ms: 0,
            fire_count: 0,
        }
    }

    /// Start the repeating timer with the given period.
    ///
    /// If the timer is already active it is first cancelled, matching
    /// the C driver's `timer_driver_start()` behaviour.
    ///
    /// # Arguments
    ///
    /// * `period_ms` - Interval between callbacks in milliseconds.
    pub fn start(&mut self, period_ms: u32) {
        if self.active {
            self.cancel();
        }
        self.period_ms = period_ms;
        self.active = true;
    }

    /// Cancel the active repeating timer.
    ///
    /// Safe to call even if the timer is already inactive, matching
    /// the C driver's `timer_driver_cancel()` behaviour.
    pub fn cancel(&mut self) {
        self.active = false;
    }

    /// Return whether the timer is currently active.
    ///
    /// # Returns
    ///
    /// `true` if a repeating timer is running, `false` otherwise.
    pub fn is_active(&self) -> bool {
        self.active
    }

    /// Return the configured timer period in milliseconds.
    ///
    /// # Returns
    ///
    /// The period set by the most recent `start()` call.
    pub fn period_ms(&self) -> u32 {
        self.period_ms
    }

    /// Return the total number of times the callback has fired.
    ///
    /// # Returns
    ///
    /// Cumulative fire count since construction.
    pub fn fire_count(&self) -> u32 {
        self.fire_count
    }

    /// Record that the callback has fired once.
    ///
    /// Called by the board-level shim each time the hardware alarm
    /// triggers. Returns `true` to keep the timer repeating (matching
    /// the C `_heartbeat_callback` return value).
    ///
    /// # Returns
    ///
    /// `true` if the timer should continue repeating.
    pub fn on_fire(&mut self) -> bool {
        if !self.active {
            return false;
        }
        self.fire_count += 1;
        true
    }
}

/// Format the heartbeat message into a caller-supplied buffer.
///
/// Writes the fixed `HEARTBEAT_MSG` bytes and returns the number
/// of bytes written.
///
/// # Arguments
///
/// * `buf` - Destination buffer (must be at least `HEARTBEAT_MSG.len()` bytes).
///
/// # Returns
///
/// Number of bytes written to `buf`.
pub fn format_heartbeat(buf: &mut [u8]) -> usize {
    let len = HEARTBEAT_MSG.len();
    buf[..len].copy_from_slice(HEARTBEAT_MSG);
    len
}

/// Format a timer-started banner message.
///
/// Writes "Repeating timer started (XXXX ms)\r\n" into `buf` and
/// returns the number of bytes written.
///
/// # Arguments
///
/// * `buf` - Destination buffer (must be at least 40 bytes).
/// * `period_ms` - Timer period to include in the message.
///
/// # Returns
///
/// Number of bytes written to `buf`.
pub fn format_started(buf: &mut [u8], period_ms: u32) -> usize {
    let prefix = b"Repeating timer started (";
    let mut pos = prefix.len();
    buf[..pos].copy_from_slice(prefix);
    pos += format_u32(&mut buf[pos..], period_ms);
    let suffix = b" ms)\r\n";
    buf[pos..pos + suffix.len()].copy_from_slice(suffix);
    pos + suffix.len()
}

/// Format an unsigned 32-bit integer as decimal ASCII.
///
/// # Arguments
///
/// * `buf` - Destination buffer.
/// * `value` - Value to format.
///
/// # Returns
///
/// Number of ASCII digits written.
fn format_u32(buf: &mut [u8], value: u32) -> usize {
    if value == 0 {
        buf[0] = b'0';
        return 1;
    }
    let mut tmp = [0u8; 10];
    let n = u32_to_digits_reversed(&mut tmp, value);
    reverse_copy(buf, &tmp, n);
    n
}

/// Convert a u32 to reversed decimal digits in a temporary buffer.
fn u32_to_digits_reversed(tmp: &mut [u8; 10], mut value: u32) -> usize {
    let mut n = 0usize;
    while value > 0 {
        tmp[n] = b'0' + (value % 10) as u8;
        value /= 10;
        n += 1;
    }
    n
}

/// Copy digits from a reversed temporary buffer into the output buffer.
fn reverse_copy(buf: &mut [u8], tmp: &[u8], n: usize) {
    for i in 0..n {
        buf[i] = tmp[n - 1 - i];
    }
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// New state is inactive.
    #[test]
    fn new_state_is_inactive() {
        let state = TimerDriverState::new();
        assert!(!state.is_active());
        assert_eq!(state.period_ms(), 0);
        assert_eq!(state.fire_count(), 0);
    }

    /// Start activates timer.
    #[test]
    fn start_activates_timer() {
        let mut state = TimerDriverState::new();
        state.start(1_000);
        assert!(state.is_active());
        assert_eq!(state.period_ms(), 1_000);
    }

    /// Cancel deactivates timer.
    #[test]
    fn cancel_deactivates_timer() {
        let mut state = TimerDriverState::new();
        state.start(1_000);
        state.cancel();
        assert!(!state.is_active());
    }

    /// Cancel when inactive is safe.
    #[test]
    fn cancel_when_inactive_is_safe() {
        let mut state = TimerDriverState::new();
        state.cancel();
        assert!(!state.is_active());
    }

    /// Start while active cancels and restarts.
    #[test]
    fn start_while_active_cancels_and_restarts() {
        let mut state = TimerDriverState::new();
        state.start(500);
        state.start(2_000);
        assert!(state.is_active());
        assert_eq!(state.period_ms(), 2_000);
    }

    /// On fire increments count.
    #[test]
    fn on_fire_increments_count() {
        let mut state = TimerDriverState::new();
        state.start(1_000);
        assert!(state.on_fire());
        assert_eq!(state.fire_count(), 1);
        assert!(state.on_fire());
        assert_eq!(state.fire_count(), 2);
    }

    /// On fire returns false when inactive.
    #[test]
    fn on_fire_returns_false_when_inactive() {
        let mut state = TimerDriverState::new();
        assert!(!state.on_fire());
        assert_eq!(state.fire_count(), 0);
    }

    /// On fire after cancel returns false.
    #[test]
    fn on_fire_after_cancel_returns_false() {
        let mut state = TimerDriverState::new();
        state.start(1_000);
        assert!(state.on_fire());
        state.cancel();
        assert!(!state.on_fire());
        assert_eq!(state.fire_count(), 1);
    }

    /// Format heartbeat matches c output.
    #[test]
    fn format_heartbeat_matches_c_output() {
        let mut buf = [0u8; 32];
        let n = format_heartbeat(&mut buf);
        assert_eq!(&buf[..n], b"Timer heartbeat\r\n");
    }

    /// Format started 1000ms.
    #[test]
    fn format_started_1000ms() {
        let mut buf = [0u8; 48];
        let n = format_started(&mut buf, 1_000);
        assert_eq!(&buf[..n], b"Repeating timer started (1000 ms)\r\n");
    }

    /// Format started single digit.
    #[test]
    fn format_started_single_digit() {
        let mut buf = [0u8; 48];
        let n = format_started(&mut buf, 5);
        assert_eq!(&buf[..n], b"Repeating timer started (5 ms)\r\n");
    }

    /// Format u32 zero.
    #[test]
    fn format_u32_zero() {
        let mut buf = [0u8; 10];
        let n = format_u32(&mut buf, 0);
        assert_eq!(&buf[..n], b"0");
    }

    /// Format u32 large value.
    #[test]
    fn format_u32_large_value() {
        let mut buf = [0u8; 10];
        let n = format_u32(&mut buf, 123456);
        assert_eq!(&buf[..n], b"123456");
    }

    /// Default period matches c demo.
    #[test]
    fn default_period_matches_c_demo() {
        assert_eq!(DEFAULT_PERIOD_MS, 1_000);
    }
}
