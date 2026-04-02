//! @file ir.rs
//! @brief Implementation of the NEC IR receiver decoder driver
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

/// Leader wait timeout in microseconds.
pub const LEADER_START_TIMEOUT_US: u32 = 150_000;

/// Maximum duration accepted for the NEC leader mark wait.
pub const LEADER_MARK_TIMEOUT_US: u32 = 12_000;

/// Minimum valid NEC leader mark width in microseconds.
pub const LEADER_MARK_MIN_US: i64 = 8_000;

/// Maximum valid NEC leader mark width in microseconds.
pub const LEADER_MARK_MAX_US: i64 = 10_000;

/// Maximum duration accepted for the NEC leader space wait.
pub const LEADER_SPACE_TIMEOUT_US: u32 = 7_000;

/// Minimum valid NEC leader space width in microseconds.
pub const LEADER_SPACE_MIN_US: i64 = 3_500;

/// Maximum valid NEC leader space width in microseconds.
pub const LEADER_SPACE_MAX_US: i64 = 5_000;

/// Maximum duration accepted while waiting for the bit mark to end.
pub const BIT_MARK_TIMEOUT_US: u32 = 1_000;

/// Maximum duration accepted while measuring the data space.
pub const BIT_SPACE_TIMEOUT_US: u32 = 2_500;

/// Minimum valid data space width in microseconds.
pub const BIT_SPACE_MIN_US: i64 = 200;

/// Space width above which a NEC bit is interpreted as logical 1.
pub const BIT_ONE_THRESHOLD_US: i64 = 1_200;

/// Total number of bits in an NEC frame.
pub const FRAME_BITS: usize = 32;

/// Return true if the measured NEC leader mark width is valid.
pub fn is_valid_leader_mark(duration_us: i64) -> bool {
    (LEADER_MARK_MIN_US..=LEADER_MARK_MAX_US).contains(&duration_us)
}

/// Return true if the measured NEC leader space width is valid.
pub fn is_valid_leader_space(duration_us: i64) -> bool {
    (LEADER_SPACE_MIN_US..=LEADER_SPACE_MAX_US).contains(&duration_us)
}

/// Return true if the measured NEC bit space width is valid.
pub fn is_valid_bit_space(duration_us: i64) -> bool {
    duration_us >= BIT_SPACE_MIN_US
}

/// Accumulate a single NEC bit into the 4-byte frame buffer.
///
/// Matches the C implementation exactly: bytes are filled LSB-first.
pub fn accumulate_nec_bit(data: &mut [u8; 4], bit_index: usize, duration_us: i64) {
    let byte_idx = bit_index / 8;
    let bit_idx = bit_index % 8;
    if duration_us > BIT_ONE_THRESHOLD_US {
        data[byte_idx] |= 1u8 << bit_idx;
    }
}

/// Validate an NEC frame and return the command byte on success.
pub fn validate_nec_frame(data: &[u8; 4]) -> Option<u8> {
    if data[0].wrapping_add(data[1]) == 0xFF && data[2].wrapping_add(data[3]) == 0xFF {
        Some(data[2])
    } else {
        None
    }
}

/// Format the decoded command as hexadecimal and decimal followed by CRLF.
pub fn format_command(buf: &mut [u8], command: u8) -> usize {
    let prefix = b"NEC command: 0x";
    let mut pos = copy_slice(buf, 0, prefix);
    pos += format_hex_u8(buf, pos, command);
    pos += copy_slice(buf, pos, b"  (");
    pos += format_u8(buf, pos, command);
    pos += copy_slice(buf, pos, b")\r\n");
    pos
}

/// Copy a byte slice into `buf` at the given offset, returning bytes written.
fn copy_slice(buf: &mut [u8], offset: usize, src: &[u8]) -> usize {
    buf[offset..offset + src.len()].copy_from_slice(src);
    src.len()
}

/// Format an unsigned 8-bit integer at the given buffer offset.
fn format_u8(buf: &mut [u8], pos: usize, value: u8) -> usize {
    let n = u8_digit_count(value);
    write_u8_digits(buf, pos, value, n);
    n
}

/// Return the number of decimal digits in a u8.
fn u8_digit_count(value: u8) -> usize {
    if value >= 100 { 3 } else if value >= 10 { 2 } else { 1 }
}

/// Write the decimal digits of a u8 into `buf` at `pos`.
fn write_u8_digits(buf: &mut [u8], pos: usize, value: u8, n: usize) {
    if n >= 3 { buf[pos] = b'0' + value / 100; }
    if n >= 2 { buf[pos + n - 2] = b'0' + (value / 10) % 10; }
    buf[pos + n - 1] = b'0' + value % 10;
}

/// Format an unsigned 8-bit integer as two uppercase hexadecimal digits.
fn format_hex_u8(buf: &mut [u8], pos: usize, value: u8) -> usize {
    buf[pos] = hex_digit((value >> 4) & 0x0F);
    buf[pos + 1] = hex_digit(value & 0x0F);
    2
}

/// Convert a 4-bit value to its uppercase ASCII hex digit.
fn hex_digit(value: u8) -> u8 {
    if value < 10 {
        b'0' + value
    } else {
        b'A' + (value - 10)
    }
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    #[test]
    fn leader_mark_accepts_lower_bound() {
        assert!(is_valid_leader_mark(8_000));
    }

    #[test]
    fn leader_mark_rejects_below_lower_bound() {
        assert!(!is_valid_leader_mark(7_999));
    }

    #[test]
    fn leader_space_accepts_upper_bound() {
        assert!(is_valid_leader_space(5_000));
    }

    #[test]
    fn leader_space_rejects_above_upper_bound() {
        assert!(!is_valid_leader_space(5_001));
    }

    #[test]
    fn bit_space_rejects_short_pulse() {
        assert!(!is_valid_bit_space(199));
    }

    #[test]
    fn bit_space_accepts_threshold() {
        assert!(is_valid_bit_space(200));
    }

    #[test]
    fn accumulate_zero_bit_leaves_byte_clear() {
        let mut data = [0u8; 4];
        accumulate_nec_bit(&mut data, 0, 800);
        assert_eq!(data[0], 0);
    }

    #[test]
    fn accumulate_one_bit_sets_lsb() {
        let mut data = [0u8; 4];
        accumulate_nec_bit(&mut data, 0, 1_300);
        assert_eq!(data[0], 1);
    }

    #[test]
    fn accumulate_crosses_into_next_byte() {
        let mut data = [0u8; 4];
        accumulate_nec_bit(&mut data, 8, 1_300);
        assert_eq!(data[0], 0);
        assert_eq!(data[1], 1);
    }

    #[test]
    fn validate_frame_returns_command() {
        let data = [0x00, 0xFF, 0x45, 0xBA];
        assert_eq!(validate_nec_frame(&data), Some(0x45));
    }

    #[test]
    fn validate_frame_rejects_bad_inverse() {
        let data = [0x00, 0xFE, 0x45, 0xBA];
        assert_eq!(validate_nec_frame(&data), None);
    }

    #[test]
    fn format_command_single_digit() {
        let mut buf = [0u8; 24];
        let n = format_command(&mut buf, 7);
        assert_eq!(&buf[..n], b"NEC command: 0x07  (7)\r\n");
    }

    #[test]
    fn format_command_three_digits() {
        let mut buf = [0u8; 26];
        let n = format_command(&mut buf, 255);
        assert_eq!(&buf[..n], b"NEC command: 0xFF  (255)\r\n");
    }

    #[test]
    fn format_hex_digit_alpha() {
        assert_eq!(hex_digit(0x0A), b'A');
    }
}