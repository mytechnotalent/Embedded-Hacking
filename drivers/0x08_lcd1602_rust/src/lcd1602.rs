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
    let prefix = b"Count: ";
    buf[..7].copy_from_slice(prefix);
    let mut pos = 7;
    let digits = [
        ((count / 100000) % 10) as u8,
        ((count / 10000) % 10) as u8,
        ((count / 1000) % 10) as u8,
        ((count / 100) % 10) as u8,
        ((count / 10) % 10) as u8,
        (count % 10) as u8,
    ];
    let mut leading = true;
    for (i, &d) in digits.iter().enumerate() {
        if i == 5 || d != 0 { leading = false; }
        buf[pos] = if leading { b' ' } else { b'0' + d };
        pos += 1;
    }
    pos
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn build_nibble_command_mode() {
        let b = build_nibble(0x03, 4, 0, 0x08);
        assert_eq!(b, 0x38);
    }

    #[test]
    fn build_nibble_data_mode() {
        let b = build_nibble(0x04, 4, 1, 0x08);
        assert_eq!(b, 0x49);
    }

    #[test]
    fn build_nibble_no_backlight() {
        let b = build_nibble(0x0F, 4, 0, 0x00);
        assert_eq!(b, 0xF0);
    }

    #[test]
    fn nibble_with_en_sets_bit() {
        assert_eq!(nibble_with_en(0x38), 0x3C);
    }

    #[test]
    fn nibble_without_en_clears_bit() {
        assert_eq!(nibble_without_en(0x3C), 0x38);
    }

    #[test]
    fn cursor_address_line0_col0() {
        assert_eq!(cursor_address(0, 0), 0x80);
    }

    #[test]
    fn cursor_address_line1_col0() {
        assert_eq!(cursor_address(1, 0), 0xC0);
    }

    #[test]
    fn cursor_address_line0_col5() {
        assert_eq!(cursor_address(0, 5), 0x85);
    }

    #[test]
    fn cursor_address_line1_col15() {
        assert_eq!(cursor_address(1, 15), 0xCF);
    }

    #[test]
    fn cursor_address_clamps_line() {
        assert_eq!(cursor_address(5, 0), 0xC0);
    }

    #[test]
    fn format_counter_zero() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 0);
        assert_eq!(&buf[..n], b"Count:      0");
    }

    #[test]
    fn format_counter_one() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 1);
        assert_eq!(&buf[..n], b"Count:      1");
    }

    #[test]
    fn format_counter_large() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 123456);
        assert_eq!(&buf[..n], b"Count: 123456");
    }

    #[test]
    fn format_counter_six_digits() {
        let mut buf = [0u8; 16];
        let n = format_counter(&mut buf, 999999);
        assert_eq!(&buf[..n], b"Count: 999999");
    }
}
