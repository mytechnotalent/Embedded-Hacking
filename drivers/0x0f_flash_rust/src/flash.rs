//! @file flash.rs
//! @brief Pure-logic flash driver (host-testable, no HAL)
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

/// Total on-chip flash size in bytes (4 MB).
pub const FLASH_SIZE_BYTES: u32 = 4 * 1024 * 1024;

/// Flash sector size in bytes (4096).
pub const FLASH_SECTOR_SIZE: u32 = 4096;

/// Flash page size in bytes (256).
pub const FLASH_PAGE_SIZE: u32 = 256;

/// Flash target offset: last sector of flash.
pub const FLASH_TARGET_OFFSET: u32 = FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;

/// Write buffer length matching the C demo (one page = 256 bytes).
pub const FLASH_WRITE_LEN: usize = FLASH_PAGE_SIZE as usize;

/// XIP base address where flash is memory-mapped.
pub const XIP_BASE: u32 = 0x1000_0000;

/// Demo string written to flash, matching the C demo exactly.
pub const DEMO_MSG: &[u8] = b"Embedded Hacking flash driver demo";

/// Fill the entire buffer with 0xFF (erased flash state).
fn fill_erased(buf: &mut [u8]) {
    for b in buf.iter_mut() {
        *b = 0xFF;
    }
}

/// Copy the demo string and NUL terminator into `buf`, returning meaningful length.
fn copy_demo_msg(buf: &mut [u8]) -> usize {
    let msg_with_nul = DEMO_MSG.len() + 1;
    let n = msg_with_nul.min(buf.len());
    let copy_len = DEMO_MSG.len().min(n);
    buf[..copy_len].copy_from_slice(&DEMO_MSG[..copy_len]);
    if copy_len < n {
        buf[copy_len] = 0x00;
    }
    n
}

/// Prepare a write buffer with 0xFF fill and the demo string at the start.
///
/// Mirrors the C demo's `_prepare_write_buf()` function. The buffer is
/// filled with 0xFF (erased flash state), then the demo string including
/// its NUL terminator is copied to the beginning.
///
/// # Arguments
///
/// * `buf` - Output buffer (should be `FLASH_WRITE_LEN` bytes).
///
/// # Returns
///
/// Number of meaningful bytes (string length + NUL terminator).
pub fn prepare_write_buf(buf: &mut [u8]) -> usize {
    fill_erased(buf);
    copy_demo_msg(buf)
}

/// Format the "Flash readback: <string>\r\n" message into `buf`.
///
/// Reads from `read_data` up to the first NUL byte (or end of slice)
/// and formats the output message matching the C demo's printf.
///
/// # Arguments
///
/// * `buf` - Output buffer (should be >= 64 bytes).
/// * `read_data` - Data read back from flash.
///
/// # Returns
///
/// Number of bytes written to `buf`.
pub fn format_readback(buf: &mut [u8], read_data: &[u8]) -> usize {
    let prefix = b"Flash readback: ";
    let suffix = b"\r\n";
    let mut pos = copy_slice(buf, 0, prefix);
    pos += copy_c_string(&mut buf[pos..], read_data);
    pos += copy_slice(buf, pos, suffix);
    pos
}

/// Copy a byte slice into `buf` at the given offset, returning bytes written.
fn copy_slice(buf: &mut [u8], offset: usize, src: &[u8]) -> usize {
    let n = src.len().min(buf.len().saturating_sub(offset));
    buf[offset..offset + n].copy_from_slice(&src[..n]);
    n
}

/// Copy bytes from `data` up to the first NUL into `buf`.
fn copy_c_string(buf: &mut [u8], data: &[u8]) -> usize {
    let str_len = data.iter().position(|&b| b == 0).unwrap_or(data.len());
    let n = str_len.min(buf.len());
    buf[..n].copy_from_slice(&data[..n]);
    n
}

#[cfg(test)]
mod tests {
    // Import all parent module items
    use super::*;

    /// Flash size is 4mb.
    #[test]
    fn flash_size_is_4mb() {
        assert_eq!(FLASH_SIZE_BYTES, 4 * 1024 * 1024);
    }

    /// Sector size is 4096.
    #[test]
    fn sector_size_is_4096() {
        assert_eq!(FLASH_SECTOR_SIZE, 4096);
    }

    /// Page size is 256.
    #[test]
    fn page_size_is_256() {
        assert_eq!(FLASH_PAGE_SIZE, 256);
    }

    /// Target offset is last sector.
    #[test]
    fn target_offset_is_last_sector() {
        assert_eq!(FLASH_TARGET_OFFSET, FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);
    }

    /// Write len matches page size.
    #[test]
    fn write_len_matches_page_size() {
        assert_eq!(FLASH_WRITE_LEN, 256);
    }

    /// Xip base address.
    #[test]
    fn xip_base_address() {
        assert_eq!(XIP_BASE, 0x1000_0000);
    }

    /// Prepare write buf fills 0xff.
    #[test]
    fn prepare_write_buf_fills_0xff() {
        let mut buf = [0u8; FLASH_WRITE_LEN];
        prepare_write_buf(&mut buf);
        // Bytes after the string + NUL should be 0xFF
        let after = DEMO_MSG.len() + 1;
        for &b in &buf[after..] {
            assert_eq!(b, 0xFF);
        }
    }

    /// Prepare write buf has demo string.
    #[test]
    fn prepare_write_buf_has_demo_string() {
        let mut buf = [0u8; FLASH_WRITE_LEN];
        prepare_write_buf(&mut buf);
        assert_eq!(&buf[..DEMO_MSG.len()], DEMO_MSG);
    }

    /// Prepare write buf has nul terminator.
    #[test]
    fn prepare_write_buf_has_nul_terminator() {
        let mut buf = [0u8; FLASH_WRITE_LEN];
        prepare_write_buf(&mut buf);
        assert_eq!(buf[DEMO_MSG.len()], 0x00);
    }

    /// Format readback matches c output.
    #[test]
    fn format_readback_matches_c_output() {
        let mut read_data = [0xFFu8; FLASH_WRITE_LEN];
        let msg = b"Embedded Hacking flash driver demo";
        read_data[..msg.len()].copy_from_slice(msg);
        read_data[msg.len()] = 0x00;
        let mut buf = [0u8; 128];
        let n = format_readback(&mut buf, &read_data);
        assert_eq!(
            &buf[..n],
            b"Flash readback: Embedded Hacking flash driver demo\r\n"
        );
    }

    /// Format readback empty string.
    #[test]
    fn format_readback_empty_string() {
        let read_data = [0u8; 8];
        let mut buf = [0u8; 64];
        let n = format_readback(&mut buf, &read_data);
        assert_eq!(&buf[..n], b"Flash readback: \r\n");
    }

    /// Format readback no nul.
    #[test]
    fn format_readback_no_nul() {
        let read_data = [b'A'; 8];
        let mut buf = [0u8; 64];
        let n = format_readback(&mut buf, &read_data);
        assert_eq!(&buf[..n], b"Flash readback: AAAAAAAA\r\n");
    }

    /// Demo msg matches c string.
    #[test]
    fn demo_msg_matches_c_string() {
        assert_eq!(DEMO_MSG, b"Embedded Hacking flash driver demo");
    }
}

// End of file
