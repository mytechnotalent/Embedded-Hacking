//! @file i2c.rs
//! @brief Implementation of the I2C bus scanner driver
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

/// Lowest valid (non-reserved) 7-bit I2C address.
pub const SCAN_ADDR_MIN: u8 = 0x08;

/// Highest valid (non-reserved) 7-bit I2C address.
pub const SCAN_ADDR_MAX: u8 = 0x77;

/// Return `true` when `addr` falls in a reserved 7-bit I2C range.
///
/// # Arguments
///
/// * `addr` - 7-bit I2C address to check.
///
/// # Returns
///
/// `true` if the address is in the reserved low or high range.
pub fn is_reserved(addr: u8) -> bool {
    addr < SCAN_ADDR_MIN || addr > SCAN_ADDR_MAX
}

/// Convert a 4-bit value to its uppercase ASCII hex digit.
///
/// # Arguments
///
/// * `val` - Value in the range 0..=15.
///
/// # Returns
///
/// ASCII byte `b'0'`..`b'9'` or `b'A'`..`b'F'`.
fn hex_digit(val: u8) -> u8 {
    if val < 10 { b'0' + val } else { b'A' + val - 10 }
}

/// Write the scan-table header into `buf` and return the byte count.
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 56 bytes).
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub fn format_scan_header(buf: &mut [u8]) -> usize {
    let h = b"\r\nI2C bus scan:\r\n     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n";
    buf[..h.len()].copy_from_slice(h);
    h.len()
}

/// Format one cell of the 16-column scan table into `buf`.
///
/// Prepends the row label when `addr` is at a 16-byte boundary and
/// appends `\r\n` when `addr` is the last column of a row.
///
/// # Arguments
///
/// * `buf` - Mutable byte slice for formatted output.
/// * `addr` - 7-bit I2C address being reported.
/// * `found` - `true` if a device acknowledged at this address.
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub fn format_scan_entry(buf: &mut [u8], addr: u8, found: bool) -> usize {
    let mut pos = format_row_prefix(buf, addr);
    pos += format_cell(&mut buf[pos..], addr, found);
    if addr % 16 == 15 { buf[pos] = b'\r'; pos += 1; buf[pos] = b'\n'; pos += 1; }
    pos
}

/// Write the row prefix ("XX: ") if addr is at a 16-byte boundary.
fn format_row_prefix(buf: &mut [u8], addr: u8) -> usize {
    if addr % 16 != 0 { return 0; }
    buf[0] = hex_digit((addr >> 4) & 0x0F);
    buf[1] = hex_digit(addr & 0x0F);
    buf[2] = b':';
    buf[3] = b' ';
    4
}

/// Write a single cell: "XX " if found, "-- " if not, "   " if reserved.
fn format_cell(buf: &mut [u8], addr: u8, found: bool) -> usize {
    let cell = cell_bytes(addr, found);
    buf[..3].copy_from_slice(&cell);
    3
}

/// Return the 3-byte cell content for an I2C scan address.
fn cell_bytes(addr: u8, found: bool) -> [u8; 3] {
    if is_reserved(addr) { return [b' ', b' ', b' ']; }
    if found { [hex_digit((addr >> 4) & 0x0F), hex_digit(addr & 0x0F), b' '] }
    else { [b'-', b'-', b' '] }
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    #[test]
    fn is_reserved_below_min() {
        assert!(is_reserved(0x00));
        assert!(is_reserved(0x07));
    }

    #[test]
    fn is_reserved_at_min_boundary() {
        assert!(!is_reserved(0x08));
    }

    #[test]
    fn is_reserved_mid_range() {
        assert!(!is_reserved(0x50));
    }

    #[test]
    fn is_reserved_at_max_boundary() {
        assert!(!is_reserved(0x77));
    }

    #[test]
    fn is_reserved_above_max() {
        assert!(is_reserved(0x78));
        assert!(is_reserved(0x7F));
    }

    #[test]
    fn format_scan_header_content() {
        let mut buf = [0u8; 80];
        let n = format_scan_header(&mut buf);
        let s = core::str::from_utf8(&buf[..n]).unwrap();
        assert!(s.starts_with("\r\nI2C bus scan:\r\n"));
        assert!(s.contains("0  1  2  3"));
        assert!(s.contains("D  E  F\r\n"));
    }

    #[test]
    fn format_scan_entry_reserved_row_start() {
        let mut buf = [0u8; 16];
        let n = format_scan_entry(&mut buf, 0x00, false);
        assert_eq!(&buf[..n], b"00:    ");
    }

    #[test]
    fn format_scan_entry_found() {
        let mut buf = [0u8; 16];
        let n = format_scan_entry(&mut buf, 0x51, true);
        assert_eq!(&buf[..n], b"51 ");
    }

    #[test]
    fn format_scan_entry_not_found() {
        let mut buf = [0u8; 16];
        let n = format_scan_entry(&mut buf, 0x51, false);
        assert_eq!(&buf[..n], b"-- ");
    }

    #[test]
    fn format_scan_entry_row_start_valid() {
        let mut buf = [0u8; 16];
        let n = format_scan_entry(&mut buf, 0x10, false);
        assert_eq!(&buf[..n], b"10: -- ");
    }

    #[test]
    fn format_scan_entry_row_end() {
        let mut buf = [0u8; 16];
        let n = format_scan_entry(&mut buf, 0x1F, false);
        assert_eq!(&buf[..n], b"-- \r\n");
    }

    #[test]
    fn hex_digit_values() {
        assert_eq!(hex_digit(0), b'0');
        assert_eq!(hex_digit(9), b'9');
        assert_eq!(hex_digit(10), b'A');
        assert_eq!(hex_digit(15), b'F');
    }
}
