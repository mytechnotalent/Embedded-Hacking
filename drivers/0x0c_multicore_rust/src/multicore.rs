//! @file multicore.rs
//! @brief Implementation of the multicore FIFO driver helper logic
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

/// Increment a 32-bit value by one (core 1 processing logic).
pub fn increment_value(value: u32) -> u32 {
    value.wrapping_add(1)
}

/// Format a u32 value as decimal ASCII into a buffer.
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

/// Format the round-trip message for UART output.
///
/// Produces: `core0 sent: N, core1 returned: N\r\n`
pub fn format_round_trip(buf: &mut [u8], sent: u32, returned: u32) -> usize {
    let prefix = b"core0 sent: ";
    let middle = b", core1 returned: ";
    let mut pos = 0usize;
    buf[pos..pos + prefix.len()].copy_from_slice(prefix);
    pos += prefix.len();
    pos += format_u32(&mut buf[pos..], sent);
    buf[pos..pos + middle.len()].copy_from_slice(middle);
    pos += middle.len();
    pos += format_u32(&mut buf[pos..], returned);
    buf[pos] = b'\r';
    pos += 1;
    buf[pos] = b'\n';
    pos += 1;
    pos
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    #[test]
    fn increment_value_adds_one() {
        assert_eq!(increment_value(0), 1);
        assert_eq!(increment_value(41), 42);
    }

    #[test]
    fn increment_value_wraps_at_max() {
        assert_eq!(increment_value(u32::MAX), 0);
    }

    #[test]
    fn format_u32_zero() {
        let mut buf = [0u8; 10];
        let n = format_u32(&mut buf, 0);
        assert_eq!(&buf[..n], b"0");
    }

    #[test]
    fn format_u32_single_digit() {
        let mut buf = [0u8; 10];
        let n = format_u32(&mut buf, 7);
        assert_eq!(&buf[..n], b"7");
    }

    #[test]
    fn format_u32_multi_digit() {
        let mut buf = [0u8; 10];
        let n = format_u32(&mut buf, 12345);
        assert_eq!(&buf[..n], b"12345");
    }

    #[test]
    fn format_u32_max() {
        let mut buf = [0u8; 10];
        let n = format_u32(&mut buf, u32::MAX);
        assert_eq!(&buf[..n], b"4294967295");
    }

    #[test]
    fn format_round_trip_small_values() {
        let mut buf = [0u8; 52];
        let n = format_round_trip(&mut buf, 0, 1);
        assert_eq!(&buf[..n], b"core0 sent: 0, core1 returned: 1\r\n");
    }

    #[test]
    fn format_round_trip_larger_values() {
        let mut buf = [0u8; 52];
        let n = format_round_trip(&mut buf, 42, 43);
        assert_eq!(&buf[..n], b"core0 sent: 42, core1 returned: 43\r\n");
    }

    #[test]
    fn format_round_trip_max_values() {
        let mut buf = [0u8; 52];
        let n = format_round_trip(&mut buf, u32::MAX, 0);
        assert_eq!(
            &buf[..n],
            b"core0 sent: 4294967295, core1 returned: 0\r\n"
        );
    }
}
