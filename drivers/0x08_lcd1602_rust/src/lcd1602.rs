//! @file lcd1602.rs
//! @brief Implementation of the HD44780 16x2 LCD (PCF8574 backpack) driver
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

/// PCF8574 -> LCD control pin: Register Select.
pub const PIN_RS: u8 = 0x01;

/// PCF8574 -> LCD control pin: Read/Write.
pub const PIN_RW: u8 = 0x02;

/// PCF8574 -> LCD control pin: Enable.
pub const PIN_EN: u8 = 0x04;

/// Build a PCF8574 output byte for a 4-bit LCD nibble.
///
/// # Arguments
///
/// * `nibble` - 4-bit data value (0x00..0x0F).
/// * `nibble_shift` - Number of bits to shift the nibble left.
/// * `mode` - Register select mode (0 = command, non-zero = data).
/// * `backlight_mask` - Bitmask to enable the backlight LED.
///
/// # Returns
///
/// Assembled PCF8574 output byte.
pub fn build_nibble(nibble: u8, nibble_shift: u8, mode: u8, backlight_mask: u8) -> u8 {
    let mut data = (nibble & 0x0F) << nibble_shift;
    if mode != 0 { data |= PIN_RS; }
    data |= backlight_mask;
    data
}

/// Build the PCF8574 byte with EN asserted.
///
/// # Arguments
///
/// * `nibble_byte` - PCF8574 output byte before EN assertion.
///
/// # Returns
///
/// Byte with the EN bit set.
pub fn nibble_with_en(nibble_byte: u8) -> u8 {
    nibble_byte | PIN_EN
}

/// Build the PCF8574 byte with EN de-asserted.
///
/// # Arguments
///
/// * `nibble_byte` - PCF8574 output byte before EN de-assertion.
///
/// # Returns
///
/// Byte with the EN bit cleared.
pub fn nibble_without_en(nibble_byte: u8) -> u8 {
    nibble_byte & !PIN_EN
}

/// HD44780 row-offset lookup.
const ROW_OFFSETS: [u8; 2] = [0x00, 0x40];

/// Compute the DDRAM address byte for `lcd_set_cursor`.
///
/// # Arguments
///
/// * `line` - Display row (0 or 1; values > 1 are clamped to 1).
/// * `position` - Column offset within the row.
///
/// # Returns
///
/// HD44780 set-DDRAM-address command byte (0x80 | offset).
pub fn cursor_address(line: u8, position: u8) -> u8 {
    let row = if line > 1 { 1 } else { line as usize };
    0x80 | (position + ROW_OFFSETS[row])
}

/// Extract the six decimal digits of a counter value into an array.
fn fill_digit_array(count: u32) -> [u8; 6] {
    [
        ((count / 100000) % 10) as u8,
        ((count / 10000) % 10) as u8,
        ((count / 1000) % 10) as u8,
        ((count / 100) % 10) as u8,
        ((count / 10) % 10) as u8,
        (count % 10) as u8,
    ]
}

/// Write six counter digits into `buf` starting at `start`, suppressing leading zeros.
fn write_counter_digits(buf: &mut [u8], start: usize, digits: &[u8; 6]) -> usize {
    let mut pos = start;
    let mut leading = true;
    for (i, &d) in digits.iter().enumerate() {
        if i == 5 || d != 0 { leading = false; }
        buf[pos] = if leading { b' ' } else { b'0' + d };
        pos += 1;
    }
    pos
}

/// Format a counter value as `"Count: NNNNNN"` (right-justified, 6 digits).
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 13 bytes).
/// * `count` - Counter value to format (0..999999).
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub fn format_counter(buf: &mut [u8], count: u32) -> usize {
    buf[..7].copy_from_slice(b"Count: ");
    let digits = fill_digit_array(count);
    write_counter_digits(buf, 7, &digits)
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// Build nibble command mode.
    #[test]
    fn build_nibble_command_mode() {
        let b = build_nibble(0x03, 4, 0, 0x08);
        assert_eq!(b, 0x38);
    }

    /// Build nibble data mode.
    #[test]
    fn build_nibble_data_mode() {
        let b = build_nibble(0x04, 4, 1, 0x08);
        assert_eq!(b, 0x49);
    }

    /// Build nibble no backlight.
    #[test]
    fn build_nibble_no_backlight() {
        let b = build_nibble(0x0F, 4, 0, 0x00);
        assert_eq!(b, 0xF0);
    }

    /// Nibble with en sets bit.
    #[test]
    fn nibble_with_en_sets_bit() {
        assert_eq!(nibble_with_en(0x38), 0x3C);
    }

    /// Nibble without en clears bit.
    #[test]
    fn nibble_without_en_clears_bit() {
        assert_eq!(nibble_without_en(0x3C), 0x38);
    }

    /// Cursor address line0 col0.
    #[test]
    fn cursor_address_line0_col0() {
        assert_eq!(cursor_address(0, 0), 0x80);
    }

    /// Cursor address line1 col0.
    #[test]
    fn cursor_address_line1_col0() {
        assert_eq!(cursor_address(1, 0), 0xC0);
    }

    /// Cursor address line0 col5.
    #[test]
    fn cursor_address_line0_col5() {
        assert_eq!(cursor_address(0, 5), 0x85);
    }

    /// Cursor address line1 col15.
    #[test]
    fn cursor_address_line1_col15() {
        assert_eq!(cursor_address(1, 15), 0xCF);
    }

    /// Cursor address clamps line.
    #[test]
    fn cursor_address_clamps_line() {
        assert_eq!(cursor_address(5, 0), 0xC0);
    }

    /// Format counter zero.
    #[test]
    fn format_counter_zero() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 0);
        assert_eq!(&buf[..n], b"Count:      0");
    }

    /// Format counter one.
    #[test]
    fn format_counter_one() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 1);
        assert_eq!(&buf[..n], b"Count:      1");
    }

    /// Format counter large.
    #[test]
    fn format_counter_large() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 123456);
        assert_eq!(&buf[..n], b"Count: 123456");
    }

    /// Format counter six digits.
    #[test]
    fn format_counter_six_digits() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 999999);
        assert_eq!(&buf[..n], b"Count: 999999");
    }
}
