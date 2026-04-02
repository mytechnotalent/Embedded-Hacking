//! @file board.rs
//! @brief Board-level HAL helpers for the flash driver
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

// Flash driver pure-logic functions and constants
use crate::flash;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
// ROM data flash functions for low-level flash operations
use hal::rom_data;
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

// Alias our HAL crate
#[cfg(rp2350)]
use rp235x_hal as hal;
#[cfg(rp2040)]
use rp2040_hal as hal;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// Type alias for the configured TX pin (GPIO 0, UART function, no pull).
pub(crate) type TxPin = Pin<hal::gpio::bank0::Gpio0, FunctionUart, PullNone>;

/// Type alias for the configured RX pin (GPIO 1, UART function, no pull).
pub(crate) type RxPin = Pin<hal::gpio::bank0::Gpio1, FunctionUart, PullNone>;

/// Type alias for the default TX pin state from `Pins::new()`.
pub(crate) type TxPinDefault = Pin<hal::gpio::bank0::Gpio0, FunctionNull, PullDown>;

/// Type alias for the default RX pin state from `Pins::new()`.
pub(crate) type RxPinDefault = Pin<hal::gpio::bank0::Gpio1, FunctionNull, PullDown>;

/// Type alias for the fully-enabled UART0 peripheral with TX/RX pins.
pub(crate) type EnabledUart = UartPeripheral<Enabled, hal::pac::UART0, (TxPin, RxPin)>;

/// Initialise system clocks and PLLs from the external 12 MHz crystal.
///
/// # Arguments
///
/// * `xosc` - XOSC peripheral singleton.
/// * `clocks` - CLOCKS peripheral singleton.
/// * `pll_sys` - PLL_SYS peripheral singleton.
/// * `pll_usb` - PLL_USB peripheral singleton.
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `watchdog` - Mutable reference to the watchdog timer.
///
/// # Returns
///
/// Configured clocks manager.
///
/// # Panics
///
/// Panics if clock initialisation fails.
pub(crate) fn init_clocks(
    xosc: hal::pac::XOSC,
    clocks: hal::pac::CLOCKS,
    pll_sys: hal::pac::PLL_SYS,
    pll_usb: hal::pac::PLL_USB,
    resets: &mut hal::pac::RESETS,
    watchdog: &mut hal::Watchdog,
) -> hal::clocks::ClocksManager {
    hal::clocks::init_clocks_and_plls(
        XTAL_FREQ_HZ, xosc, clocks, pll_sys, pll_usb, resets, watchdog,
    )
    .unwrap()
}

/// Unlock the GPIO bank and return the pin set.
///
/// # Arguments
///
/// * `io_bank0` - IO_BANK0 peripheral singleton.
/// * `pads_bank0` - PADS_BANK0 peripheral singleton.
/// * `sio` - SIO peripheral singleton.
/// * `resets` - Mutable reference to the RESETS peripheral.
///
/// # Returns
///
/// GPIO pin set for the entire bank.
pub(crate) fn init_pins(
    io_bank0: hal::pac::IO_BANK0,
    pads_bank0: hal::pac::PADS_BANK0,
    sio: hal::pac::SIO,
    resets: &mut hal::pac::RESETS,
) -> hal::gpio::Pins {
    let sio = hal::Sio::new(sio);
    hal::gpio::Pins::new(io_bank0, pads_bank0, sio.gpio_bank0, resets)
}

/// Initialise UART0 for serial output (stdio equivalent).
///
/// # Arguments
///
/// * `uart0` - PAC UART0 peripheral singleton.
/// * `tx_pin` - GPIO pin to use as UART0 TX (GPIO 0).
/// * `rx_pin` - GPIO pin to use as UART0 RX (GPIO 1).
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `clocks` - Reference to the initialised clock configuration.
///
/// # Returns
///
/// Enabled UART0 peripheral ready for blocking writes.
///
/// # Panics
///
/// Panics if the HAL cannot achieve the requested baud rate.
pub(crate) fn init_uart(
    uart0: hal::pac::UART0,
    tx_pin: TxPinDefault,
    rx_pin: RxPinDefault,
    resets: &mut hal::pac::RESETS,
    clocks: &hal::clocks::ClocksManager,
) -> EnabledUart {
    let pins = (
        tx_pin.reconfigure::<FunctionUart, PullNone>(),
        rx_pin.reconfigure::<FunctionUart, PullNone>(),
    );
    let cfg = UartConfig::new(UART_BAUD.Hz(), DataBits::Eight, None, StopBits::One);
    UartPeripheral::new(uart0, pins, resets)
        .enable(cfg, clocks.peripheral_clock.freq())
        .unwrap()
}

/// Erase one 4096-byte sector and write data to on-chip flash.
///
/// Disables interrupts, transitions the flash device out of XIP mode,
/// erases the sector at `flash_offset`, programs `data` into it, flushes
/// the cache, re-enters XIP mode, and restores interrupts. This mirrors
/// the C demo's `flash_driver_write()`.
///
/// # Arguments
///
/// * `flash_offset` - Byte offset from the start of flash (must be sector-aligned).
/// * `data` - Data buffer to write (length must be a multiple of 256).
///
/// # Safety
///
/// Caller must ensure no other core or DMA is accessing flash/XIP during
/// this operation.
pub(crate) fn flash_write(flash_offset: u32, data: &[u8]) {
    let len = data.len();
    cortex_m::interrupt::free(|_| unsafe {
        rom_data::connect_internal_flash();
        rom_data::flash_exit_xip();
        rom_data::flash_range_erase(
            flash_offset,
            flash::FLASH_SECTOR_SIZE as usize,
            flash::FLASH_SECTOR_SIZE,
            0x20,
        );
        rom_data::flash_range_program(flash_offset, data.as_ptr(), len);
        rom_data::flash_flush_cache();
        rom_data::flash_enter_cmd_xip();
    });
}

/// Read bytes from on-chip flash via the XIP memory map.
///
/// Flash is memory-mapped starting at XIP_BASE (0x10000000). This
/// function copies `out.len()` bytes from the XIP address corresponding
/// to `flash_offset` into `out`, matching the C demo's
/// `flash_driver_read()`.
///
/// # Arguments
///
/// * `flash_offset` - Byte offset from the start of flash.
/// * `out` - Destination buffer.
pub(crate) fn flash_read(flash_offset: u32, out: &mut [u8]) {
    let addr = (flash::XIP_BASE + flash_offset) as *const u8;
    for (i, byte) in out.iter_mut().enumerate() {
        *byte = unsafe { core::ptr::read_volatile(addr.add(i)) };
    }
}

/// Initialise all peripherals and run the flash demo.
///
/// # Arguments
///
/// * `pac` - PAC Peripherals singleton (consumed).
pub(crate) fn run(mut pac: hal::pac::Peripherals) -> ! {
    let mut wd = hal::Watchdog::new(pac.WATCHDOG);
    let clocks = init_clocks(pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS, &mut wd);
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    flash_demo(&uart);
    loop { cortex_m::asm::wfe(); }
}

/// Execute the flash write / read-back / report sequence.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
fn flash_demo(uart: &EnabledUart) {
    let mut write_buf = [0u8; flash::FLASH_WRITE_LEN];
    flash::prepare_write_buf(&mut write_buf);
    flash_write(flash::FLASH_TARGET_OFFSET, &write_buf);
    let mut read_buf = [0u8; flash::FLASH_WRITE_LEN];
    flash_read(flash::FLASH_TARGET_OFFSET, &mut read_buf);
    report_readback(uart, &read_buf);
}

/// Format and print the flash read-back result over UART.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `read_buf` - Buffer containing the data read back from flash.
fn report_readback(uart: &EnabledUart, read_buf: &[u8]) {
    let mut out = [0u8; 128];
    let n = flash::format_readback(&mut out, read_buf);
    uart.write_full_blocking(&out[..n]);
}

// End of file
