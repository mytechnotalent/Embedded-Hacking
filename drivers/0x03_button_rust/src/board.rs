//! @file board.rs
//! @brief Board-level HAL helpers for the button driver
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

// Interior mutability for shared delay access
use core::cell::RefCell;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{
    FunctionNull, FunctionSioInput, FunctionSioOutput, FunctionUart, Pin, PullDown, PullNone,
    PullUp,
};
// UART configuration and peripheral types
use hal::uart::{DataBits, Enabled, StopBits, UartConfig, UartPeripheral};

// Alias our HAL crate
#[cfg(rp2040)]
use rp2040_hal as hal;
#[cfg(rp2350)]
use rp235x_hal as hal;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// Debounce settling time in milliseconds.
pub(crate) const DEBOUNCE_MS: u32 = 20;

/// Main-loop polling interval in milliseconds.
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

/// Create a blocking delay timer from the ARM SysTick peripheral.
///
/// # Arguments
///
/// * `clocks` - Reference to the initialised clock configuration.
///
/// # Returns
///
/// Blocking delay provider.
///
/// # Panics
///
/// Panics if the cortex-m core peripherals have already been taken.
pub(crate) fn init_delay(clocks: &hal::clocks::ClocksManager) -> cortex_m::delay::Delay {
    let core = cortex_m::Peripherals::take().unwrap();
    cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz())
}

/// Type alias for the button input pin (GPIO 15, pull-up).
type BtnPin = Pin<hal::gpio::bank0::Gpio15, FunctionSioInput, PullUp>;

/// Type alias for the LED output pin (GPIO 25, push-pull).
type LedPin = Pin<hal::gpio::bank0::Gpio25, FunctionSioOutput, PullDown>;

/// Initialise all peripherals and run the button debounce demo.
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
    let delay = RefCell::new(init_delay(&clocks));
    uart.write_full_blocking(b"Button driver initialized: button=GPIO15  led=GPIO25\r\n");
    button_demo(&uart, pins.gpio15, pins.gpio25, &delay)
}

/// Create button and LED drivers, then run the polling loop.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `btn_pin` - Default GPIO 15 pin for the button input.
/// * `led_pin` - Default GPIO 25 pin for the LED output.
/// * `delay` - Shared reference to the delay provider.
fn button_demo(
    uart: &EnabledUart,
    btn_pin: Pin<hal::gpio::bank0::Gpio15, FunctionNull, PullDown>,
    led_pin: Pin<hal::gpio::bank0::Gpio25, FunctionNull, PullDown>,
    delay: &RefCell<cortex_m::delay::Delay>,
) -> ! {
    let mut btn =
        crate::button::ButtonDriver::init(btn_pin.into_pull_up_input(), DEBOUNCE_MS, |ms| {
            delay.borrow_mut().delay_ms(ms)
        });
    let mut led = crate::button::ButtonLed::init(led_pin.into_push_pull_output());
    let mut last = false;
    loop {
        poll_button(uart, &mut btn, &mut led, &mut last, delay);
    }
}

/// Poll button state, update LED, and report edge transitions.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `btn` - Mutable reference to the button driver.
/// * `led` - Mutable reference to the LED driver.
/// * `last` - Mutable reference to the previous button state.
/// * `delay` - Shared reference to the delay provider.
fn poll_button<F: FnMut(u32)>(
    uart: &EnabledUart,
    btn: &mut crate::button::ButtonDriver<BtnPin, F>,
    led: &mut crate::button::ButtonLed<LedPin>,
    last: &mut bool,
    delay: &RefCell<cortex_m::delay::Delay>,
) {
    let pressed = btn.is_pressed();
    led.set(pressed);
    if pressed != *last {
        report_edge(uart, pressed);
        *last = pressed;
    }
    delay.borrow_mut().delay_ms(POLL_MS);
}

/// Print button press/release message over UART.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `pressed` - `true` if the button is pressed, `false` if released.
fn report_edge(uart: &EnabledUart, pressed: bool) {
    let msg = if pressed {
        b"Button: PRESSED\r\n" as &[u8]
    } else {
        b"Button: RELEASED\r\n"
    };
    uart.write_full_blocking(msg);
}

// End of file
