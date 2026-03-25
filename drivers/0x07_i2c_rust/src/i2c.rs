/// Lowest valid (non-reserved) 7-bit I2C address.
pub const SCAN_ADDR_MIN: u8 = 0x08;

/// Highest valid (non-reserved) 7-bit I2C address.
pub const SCAN_ADDR_MAX: u8 = 0x77;

/// Return `true` when `addr` falls in a reserved 7-bit I2C range.
pub fn is_reserved(addr: u8) -> bool {
    addr < SCAN_ADDR_MIN || addr > SCAN_ADDR_MAX
}

fn hex_digit(val: u8) -> u8 {
    if val < 10 { b'0' + val } else { b'A' + val - 10 }
}

/// Write the scan-table header into `buf` and return the byte count.
pub fn format_scan_header(buf: &mut [u8]) -> usize {
    let h = b"\r\nI2C bus scan:\r\n     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n";
    buf[..h.len()].copy_from_slice(h);
    h.len()
}

/// Format one cell of the 16-column scan table into `buf`.
///
/// Prepends the row label when `addr` is at a 16-byte boundary and
/// appends `\r\n` when `addr` is the last column of a row.
pub fn format_scan_entry(buf: &mut [u8], addr: u8, found: bool) -> usize {
    let mut pos = 0;
    if addr % 16 == 0 {
        buf[pos] = hex_digit((addr >> 4) & 0x0F); pos += 1;
        buf[pos] = hex_digit(addr & 0x0F); pos += 1;
        buf[pos] = b':'; pos += 1;
        buf[pos] = b' '; pos += 1;
    }
    if is_reserved(addr) {
        buf[pos] = b' '; pos += 1;
        buf[pos] = b' '; pos += 1;
        buf[pos] = b' '; pos += 1;
    } else if found {
        buf[pos] = hex_digit((addr >> 4) & 0x0F); pos += 1;
        buf[pos] = hex_digit(addr & 0x0F); pos += 1;
        buf[pos] = b' '; pos += 1;
    } else {
        buf[pos] = b'-'; pos += 1;
        buf[pos] = b'-'; pos += 1;
        buf[pos] = b' '; pos += 1;
    }
    if addr % 16 == 15 {
        buf[pos] = b'\r'; pos += 1;
        buf[pos] = b'\n'; pos += 1;
    }
    pos
}

#[cfg(test)]
mod tests {
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
