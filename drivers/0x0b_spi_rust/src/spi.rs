//! @file spi.rs
//! @brief Implementation of the SPI loopback driver helper logic
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

/// SPI port number used by the loopback demo.
pub const SPI_PORT: u8 = 0;

/// SPI clock frequency used by the loopback demo.
pub const SPI_BAUD_HZ: u32 = 1_000_000;

/// GPIO pin number for MISO.
pub const PIN_MISO: u8 = 16;

/// GPIO pin number for chip select.
pub const PIN_CS: u8 = 17;

/// GPIO pin number for SCK.
pub const PIN_SCK: u8 = 18;

/// GPIO pin number for MOSI.
pub const PIN_MOSI: u8 = 19;

/// Fixed transmit message used by the SPI loopback demo.
pub const TX_MESSAGE: &[u8; 16] = b"SPI loopback OK\0";

/// Return true when the selected SPI port is supported by the driver.
pub fn is_valid_port(port: u8) -> bool {
    port <= 1
}

/// Clear a receive buffer to all zeros.
pub fn clear_rx_buffer(rx: &mut [u8]) {
    rx.fill(0);
}

/// Copy `src` into `dst` up to the minimum length of both slices.
pub fn copy_transfer_result(src: &[u8], dst: &mut [u8]) {
    let len = core::cmp::min(src.len(), dst.len());
    dst[..len].copy_from_slice(&src[..len]);
}

/// Format the `TX:` line with trailing CRLF.
pub fn format_tx_line(buf: &mut [u8], tx: &[u8]) -> usize {
    format_line(buf, b"TX: ", tx, false)
}

/// Format the `RX:` line with an extra blank line after it.
pub fn format_rx_line(buf: &mut [u8], rx: &[u8]) -> usize {
    format_line(buf, b"RX: ", rx, true)
}

/// Format a single ASCII line for UART output.
fn format_line(buf: &mut [u8], prefix: &[u8], text: &[u8], extra_blank_line: bool) -> usize {
    let mut pos = 0;
    buf[pos..pos + prefix.len()].copy_from_slice(prefix);
    pos += prefix.len();
    pos += copy_c_string(&mut buf[pos..], text);
    pos += append_crlf(&mut buf[pos..], extra_blank_line);
    pos
}

/// Copy bytes from `text` up to the first NUL into `buf`.
fn copy_c_string(buf: &mut [u8], text: &[u8]) -> usize {
    let len = c_string_len(text);
    buf[..len].copy_from_slice(&text[..len]);
    len
}

/// Append CRLF (and optionally a second blank CRLF) to `buf`.
fn append_crlf(buf: &mut [u8], extra: bool) -> usize {
    buf[0] = b'\r';
    buf[1] = b'\n';
    if extra { buf[2] = b'\r'; buf[3] = b'\n'; 4 } else { 2 }
}

/// Return the length up to the first NUL byte or full slice length.
fn c_string_len(text: &[u8]) -> usize {
    let mut len = 0usize;
    while len < text.len() && text[len] != 0 {
        len += 1;
    }
    len
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// Valid ports are zero and one.
    #[test]
    fn valid_ports_are_zero_and_one() {
        assert!(is_valid_port(0));
        assert!(is_valid_port(1));
        assert!(!is_valid_port(2));
    }

    /// Clear rx buffer zeroes all bytes.
    #[test]
    fn clear_rx_buffer_zeroes_all_bytes() {
        let mut rx = [1u8, 2, 3, 4];
        clear_rx_buffer(&mut rx);
        assert_eq!(rx, [0u8; 4]);
    }

    /// Copy transfer result copies common prefix.
    #[test]
    fn copy_transfer_result_copies_common_prefix() {
        let src = b"abcd";
        let mut dst = [0u8; 3];
        copy_transfer_result(src, &mut dst);
        assert_eq!(&dst, b"abc");
    }

    /// Format tx line omits trailing nul.
    #[test]
    fn format_tx_line_omits_trailing_nul() {
        let mut buf = [0u8; 32];
        let n = format_tx_line(&mut buf, TX_MESSAGE);
        assert_eq!(&buf[..n], b"TX: SPI loopback OK\r\n");
    }

    /// Format rx line adds blank line.
    #[test]
    fn format_rx_line_adds_blank_line() {
        let mut buf = [0u8; 32];
        let n = format_rx_line(&mut buf, TX_MESSAGE);
        assert_eq!(&buf[..n], b"RX: SPI loopback OK\r\n\r\n");
    }

    /// C string len stops at nul.
    #[test]
    fn c_string_len_stops_at_nul() {
        assert_eq!(c_string_len(b"abc\0xyz"), 3);
    }
}