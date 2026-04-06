//! @file board.rs
//! @brief Board-level HAL helpers for the IR driver
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

// IR pure-logic functions and timing constants
use crate::ir;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

// Alias our HAL crate
#[cfg(rp2040)]
use rp2040_hal as hal;
#[cfg(rp2350)]
use rp235x_hal as hal;

/// Timer device type for the HAL timer peripheral.
#[cfg(rp2350)]
pub(crate) type HalTimer = hal::Timer<hal::timer::CopyableTimer0>;
/// Timer type alias for RP2040 (non-generic).
#[cfg(rp2040)]
pub(crate) type HalTimer = hal::Timer;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// GPIO pin number connected to the IR receiver output.
pub(crate) const IR_GPIO: u8 = 5;

/// Delay between decode attempts in milliseconds.
pub(crate) const POLL_MS: u32 = 10;

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
pub(crate) fn init_clocks(
    xosc: hal::pac::XOSC,
    clocks: hal::pac::CLOCKS,
    pll_sys: hal::pac::PLL_SYS,
    pll_usb: hal::pac::PLL_USB,
    resets: &mut hal::pac::RESETS,
    watchdog: &mut hal::Watchdog,
) -> hal::clocks::ClocksManager {
    hal::clocks::init_clocks_and_plls(
        XTAL_FREQ_HZ,
        xosc,
        clocks,
        pll_sys,
        pll_usb,
        resets,
        watchdog,
    )
    .unwrap()
}

/// Unlock the GPIO bank and return the pin set.
pub(crate) fn init_pins(
    io_bank0: hal::pac::IO_BANK0,
    pads_bank0: hal::pac::PADS_BANK0,
    sio: hal::pac::SIO,
    resets: &mut hal::pac::RESETS,
) -> hal::gpio::Pins {
    let sio = hal::Sio::new(sio);
    hal::gpio::Pins::new(io_bank0, pads_bank0, sio.gpio_bank0, resets)
}

/// Initialise UART0 for serial output.
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

/// Create a blocking delay timer from the ARM SysTick peripheral.
pub(crate) fn init_delay(clocks: &hal::clocks::ClocksManager) -> cortex_m::delay::Delay {
    let core = cortex_m::Peripherals::take().unwrap();
    cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz())
}

/// Read the free-running microsecond timer (lower 32 bits).
fn time_us_32(timer: &HalTimer) -> u32 {
    timer.get_counter().ticks() as u32
}

/// Read the current logic level of the IR input pin through the SIO block.
fn gpio_read() -> bool {
    unsafe { (*hal::pac::SIO::PTR).gpio_in().read().bits() & (1u32 << IR_GPIO) != 0 }
}

/// Wait for the IR pin to reach the requested level or time out.
fn wait_for_level(timer: &HalTimer, level: bool, timeout_us: u32) -> Option<i64> {
    let start = time_us_32(timer);
    while gpio_read() != level {
        let elapsed = time_us_32(timer).wrapping_sub(start) as i64;
        if elapsed > timeout_us as i64 {
            return None;
        }
    }
    Some(time_us_32(timer).wrapping_sub(start) as i64)
}

/// Wait for the IR receiver to go idle (LOW).
fn wait_for_idle(timer: &HalTimer) -> bool {
    wait_for_level(timer, false, ir::LEADER_START_TIMEOUT_US).is_some()
}

/// Validate the NEC leader mark pulse width.
fn validate_leader_mark(timer: &HalTimer) -> bool {
    let Some(w) = wait_for_level(timer, true, ir::LEADER_MARK_TIMEOUT_US) else {
        return false;
    };
    ir::is_valid_leader_mark(w)
}

/// Validate the NEC leader space width.
fn validate_leader_space(timer: &HalTimer) -> bool {
    let Some(w) = wait_for_level(timer, false, ir::LEADER_SPACE_TIMEOUT_US) else {
        return false;
    };
    ir::is_valid_leader_space(w)
}

/// Wait for the NEC leader burst and space.
fn wait_leader(timer: &HalTimer) -> bool {
    wait_for_idle(timer) && validate_leader_mark(timer) && validate_leader_space(timer)
}

/// Wait for the bit mark and measure the bit space width.
fn measure_bit_space(timer: &HalTimer) -> Option<i64> {
    if wait_for_level(timer, true, ir::BIT_MARK_TIMEOUT_US).is_none() {
        return None;
    }
    let w = wait_for_level(timer, false, ir::BIT_SPACE_TIMEOUT_US)?;
    if !ir::is_valid_bit_space(w) {
        return None;
    }
    Some(w)
}

/// Read one NEC bit and store it in the frame buffer.
fn read_nec_bit(timer: &HalTimer, data: &mut [u8; 4], bit_index: usize) -> bool {
    let Some(space_width) = measure_bit_space(timer) else {
        return false;
    };
    ir::accumulate_nec_bit(data, bit_index, space_width);
    true
}

/// Read a full 32-bit NEC frame.
fn read_32_bits(timer: &HalTimer, data: &mut [u8; 4]) -> bool {
    let mut bit_index = 0usize;
    while bit_index < ir::FRAME_BITS {
        if !read_nec_bit(timer, data, bit_index) {
            return false;
        }
        bit_index += 1;
    }
    true
}

/// Block until an NEC key is decoded or return `None` on failure.
pub(crate) fn ir_getkey(timer: &HalTimer) -> Option<u8> {
    if !wait_leader(timer) {
        return None;
    }
    let mut data = [0u8; 4];
    if !read_32_bits(timer, &mut data) {
        return None;
    }
    ir::validate_nec_frame(&data)
}

/// Poll the decoder and print the key code when a valid frame is received.
pub(crate) fn poll_receiver(
    uart: &EnabledUart,
    timer: &HalTimer,
    delay: &mut cortex_m::delay::Delay,
) {
    let mut buf = [0u8; 26];
    if let Some(command) = ir_getkey(timer) {
        let len = ir::format_command(&mut buf, command);
        uart.write_full_blocking(&buf[..len]);
    }
    delay.delay_ms(POLL_MS);
}

/// Initialise all peripherals and run the NEC IR receiver demo.
///
/// # Arguments
///
/// * `pac` - PAC Peripherals singleton (consumed).
pub(crate) fn run(mut pac: hal::pac::Peripherals) -> ! {
    let mut wd = hal::Watchdog::new(pac.WATCHDOG);
    let clocks = init_clocks(
        pac.XOSC,
        pac.CLOCKS,
        pac.PLL_SYS,
        pac.PLL_USB,
        &mut pac.RESETS,
        &mut wd,
    );
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    #[cfg(rp2350)]
    let timer = hal::Timer::new_timer0(pac.TIMER0, &mut pac.RESETS, &clocks);
    #[cfg(rp2040)]
    let timer = hal::Timer::new(pac.TIMER, &mut pac.RESETS);
    let _ = pins.gpio5.into_pull_up_input();
    announce_ir(&uart);
    loop {
        poll_receiver(&uart, &timer, &mut delay);
    }
}

/// Print the IR driver initialisation banner over UART.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
fn announce_ir(uart: &EnabledUart) {
    uart.write_full_blocking(b"NEC IR driver initialized on GPIO 5\r\n");
    uart.write_full_blocking(b"Press a button on your NEC remote...\r\n");
}

// End of file
