//! @file dht11.rs
//! @brief Implementation of DHT11 temperature and humidity sensor driver
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

/// Bit-duration threshold in microseconds.
///
/// If the high pulse is longer than this value, the bit is a 1; otherwise 0.
pub const BIT_THRESHOLD_US: u32 = 40;

/// Timeout in spin-loop iterations for waiting on a pin level change.
pub const LEVEL_WAIT_TIMEOUT: u32 = 10_000;

/// Accumulate a single received bit into the 5-byte data array.
///
/// Shifts `data[byte_index]` left by one and sets the LSB to 1 if the
/// measured high-pulse `duration_us` exceeds [`BIT_THRESHOLD_US`].
///
/// # Arguments
///
/// * `data` - 5-byte array accumulating the received bits.
/// * `bit_index` - Bit index (0..39).
/// * `duration_us` - Measured duration of the high pulse in microseconds.
pub fn accumulate_bit(data: &mut [u8; 5], bit_index: usize, duration_us: u32) {
    let byte = bit_index / 8;
    data[byte] <<= 1;
    if duration_us > BIT_THRESHOLD_US {
        data[byte] |= 1;
    }
}

/// Verify the DHT11 checksum byte.
///
/// The checksum (byte 4) must equal the lower 8 bits of the sum of
/// bytes 0 through 3.
///
/// # Arguments
///
/// * `data` - 5-byte received data (bytes 0–3 plus checksum in byte 4).
///
/// # Returns
///
/// `true` if the checksum matches, `false` otherwise.
pub fn validate_checksum(data: &[u8; 5]) -> bool {
    data[4] == (data[0].wrapping_add(data[1]).wrapping_add(data[2]).wrapping_add(data[3]))
}

/// Parse humidity from the raw DHT11 data bytes.
///
/// Humidity is encoded as an integer part in byte 0 and a fractional
/// part (tenths) in byte 1.
///
/// # Arguments
///
/// * `data` - 5-byte received data.
///
/// # Returns
///
/// Humidity as a percentage (e.g. 65.3 for 65.3%).
pub fn parse_humidity(data: &[u8; 5]) -> f32 {
    data[0] as f32 + data[1] as f32 * 0.1
}

/// Parse temperature from the raw DHT11 data bytes.
///
/// Temperature is encoded as an integer part in byte 2 and a fractional
/// part (tenths) in byte 3.
///
/// # Arguments
///
/// * `data` - 5-byte received data.
///
/// # Returns
///
/// Temperature in degrees Celsius.
pub fn parse_temperature(data: &[u8; 5]) -> f32 {
    data[2] as f32 + data[3] as f32 * 0.1
}

/// Format a sensor reading as `"Humidity: XX.X%  Temperature: XX.X C"`.
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 40 bytes).
/// * `humidity` - Humidity percentage.
/// * `temperature` - Temperature in degrees Celsius.
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub fn format_reading(buf: &mut [u8], humidity: f32, temperature: f32) -> usize {
    let mut pos = copy_slice(buf, 0, b"Humidity: ");
    pos += format_f32_1(&mut buf[pos..], humidity);
    pos += copy_slice(buf, pos, b"%  Temperature: ");
    pos += format_f32_1(&mut buf[pos..], temperature);
    pos += copy_slice(buf, pos, b" C");
    pos
}

/// Copy a byte slice into `buf` at the given offset, returning bytes written.
fn copy_slice(buf: &mut [u8], offset: usize, src: &[u8]) -> usize {
    buf[offset..offset + src.len()].copy_from_slice(src);
    src.len()
}

/// Format a failed-read error message.
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 42 bytes).
/// * `gpio` - GPIO pin number to include in the message.
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub fn format_error(buf: &mut [u8], gpio: u8) -> usize {
    let mut pos = 0;
    let prefix = b"DHT11 read failed - check wiring on GPIO ";
    buf[pos..pos + prefix.len()].copy_from_slice(prefix);
    pos += prefix.len();
    pos += format_u8(&mut buf[pos..], gpio);
    pos
}

/// Return the number of decimal digits needed for a `u8`.
fn u8_digit_count(val: u8) -> usize {
    if val >= 100 { 3 } else if val >= 10 { 2 } else { 1 }
}

/// Write `count` decimal digits of `val` into `buf`.
fn write_u8_digits(buf: &mut [u8], val: u8, count: usize) {
    if count >= 3 { buf[0] = b'0' + val / 100; }
    if count >= 2 { buf[count - 2] = b'0' + (val / 10) % 10; }
    buf[count - 1] = b'0' + val % 10;
}

/// Format a `u8` as decimal ASCII digits.
///
/// # Arguments
///
/// * `buf` - Output buffer.
/// * `val` - Value to format.
///
/// # Returns
///
/// Number of bytes written.
fn format_u8(buf: &mut [u8], val: u8) -> usize {
    let n = u8_digit_count(val);
    write_u8_digits(buf, val, n);
    n
}

/// Format an `f32` with one decimal place (e.g. `"25.3"`).
///
/// # Arguments
///
/// * `buf` - Output buffer.
/// * `val` - Value to format.
///
/// # Returns
///
/// Number of bytes written.
fn format_f32_1(buf: &mut [u8], val: f32) -> usize {
    let scaled = (val * 10.0) as u32;
    let frac = (scaled % 10) as u8;
    let pos = format_u32_minimal(buf, scaled / 10);
    buf[pos] = b'.';
    buf[pos + 1] = b'0' + frac;
    pos + 2
}

/// Write a conditional digit into `buf` if `val` meets the threshold.
fn write_digit_if(buf: &mut [u8], pos: &mut usize, val: u32, threshold: u32, divisor: u32) {
    if val >= threshold {
        buf[*pos] = b'0' + ((val / divisor) % 10) as u8;
        *pos += 1;
    }
}

/// Format a u32 as minimal decimal digits (no leading zeros).
fn format_u32_minimal(buf: &mut [u8], value: u32) -> usize {
    let mut pos = 0;
    write_digit_if(buf, &mut pos, value, 100, 100);
    write_digit_if(buf, &mut pos, value, 10, 10);
    buf[pos] = b'0' + (value % 10) as u8;
    pos + 1
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// Accumulate bit zero short pulse.
    #[test]
    fn accumulate_bit_zero_short_pulse() {
        let mut data = [0u8; 5];
        accumulate_bit(&mut data, 0, 30);
        assert_eq!(data[0], 0);
    }

    /// Accumulate bit one long pulse.
    #[test]
    fn accumulate_bit_one_long_pulse() {
        let mut data = [0u8; 5];
        accumulate_bit(&mut data, 0, 70);
        assert_eq!(data[0], 1);
    }

    /// Accumulate bit threshold exact.
    #[test]
    fn accumulate_bit_threshold_exact() {
        let mut data = [0u8; 5];
        accumulate_bit(&mut data, 0, BIT_THRESHOLD_US);
        assert_eq!(data[0], 0);
    }

    /// Accumulate bit two bits.
    #[test]
    fn accumulate_bit_two_bits() {
        let mut data = [0u8; 5];
        accumulate_bit(&mut data, 0, 70);
        accumulate_bit(&mut data, 1, 30);
        assert_eq!(data[0], 0b10);
    }

    /// Accumulate bit crosses byte.
    #[test]
    fn accumulate_bit_crosses_byte() {
        let mut data = [0u8; 5];
        accumulate_bit(&mut data, 7, 70);
        accumulate_bit(&mut data, 8, 70);
        assert_eq!(data[0], 1);
        assert_eq!(data[1], 1);
    }

    /// Validate checksum valid.
    #[test]
    fn validate_checksum_valid() {
        let data = [0x28, 0x00, 0x1A, 0x00, 0x42];
        assert!(validate_checksum(&data));
    }

    /// Validate checksum invalid.
    #[test]
    fn validate_checksum_invalid() {
        let data = [0x28, 0x00, 0x1A, 0x00, 0xFF];
        assert!(!validate_checksum(&data));
    }

    /// Validate checksum wraps.
    #[test]
    fn validate_checksum_wraps() {
        let data = [0xFF, 0x01, 0x00, 0x00, 0x00];
        assert!(validate_checksum(&data));
    }

    /// Parse humidity integer only.
    #[test]
    fn parse_humidity_integer_only() {
        let data = [0x41, 0x00, 0x00, 0x00, 0x41];
        let h = parse_humidity(&data);
        assert!((h - 65.0).abs() < 0.01);
    }

    /// Parse humidity with fraction.
    #[test]
    fn parse_humidity_with_fraction() {
        let data = [0x41, 0x03, 0x00, 0x00, 0x44];
        let h = parse_humidity(&data);
        assert!((h - 65.3).abs() < 0.01);
    }

    /// Parse temperature integer only.
    #[test]
    fn parse_temperature_integer_only() {
        let data = [0x00, 0x00, 0x19, 0x00, 0x19];
        let t = parse_temperature(&data);
        assert!((t - 25.0).abs() < 0.01);
    }

    /// Parse temperature with fraction.
    #[test]
    fn parse_temperature_with_fraction() {
        let data = [0x00, 0x00, 0x19, 0x05, 0x1E];
        let t = parse_temperature(&data);
        assert!((t - 25.5).abs() < 0.01);
    }

    /// Format reading typical.
    #[test]
    fn format_reading_typical() {
        let mut buf = [0u8; 48];
        let n = format_reading(&mut buf, 65.0, 25.0);
        assert_eq!(&buf[..n], b"Humidity: 65.0%  Temperature: 25.0 C");
    }

    /// Format reading with fractions.
    #[test]
    fn format_reading_with_fractions() {
        let mut buf = [0u8; 48];
        let n = format_reading(&mut buf, 65.3, 25.5);
        assert_eq!(&buf[..n], b"Humidity: 65.3%  Temperature: 25.5 C");
    }

    /// Format error gpio4.
    #[test]
    fn format_error_gpio4() {
        let mut buf = [0u8; 48];
        let n = format_error(&mut buf, 4);
        assert_eq!(&buf[..n], b"DHT11 read failed - check wiring on GPIO 4");
    }

    /// Format error gpio15.
    #[test]
    fn format_error_gpio15() {
        let mut buf = [0u8; 48];
        let n = format_error(&mut buf, 15);
        assert_eq!(&buf[..n], b"DHT11 read failed - check wiring on GPIO 15");
    }

    /// Format u8 single digit.
    #[test]
    fn format_u8_single_digit() {
        let mut buf = [0u8; 4];
        let n = format_u8(&mut buf, 4);
        assert_eq!(&buf[..n], b"4");
    }

    /// Format u8 two digits.
    #[test]
    fn format_u8_two_digits() {
        let mut buf = [0u8; 4];
        let n = format_u8(&mut buf, 15);
        assert_eq!(&buf[..n], b"15");
    }

    /// Format u8 three digits.
    #[test]
    fn format_u8_three_digits() {
        let mut buf = [0u8; 4];
        let n = format_u8(&mut buf, 255);
        assert_eq!(&buf[..n], b"255");
    }
}
