//! @file board.rs
//! @brief Board-level HAL helpers for the PWM driver
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

// PWM duty-cycle trait for .set_duty_cycle()
use embedded_hal::pwm::SetDutyCycle;
// Rate extension trait for .Hz() baud rate construction
use fugit::RateExtU32;
// Clock trait for accessing system clock frequency
use hal::Clock;
// GPIO pin types and function selectors
use hal::gpio::{FunctionNull, FunctionUart, Pin, PullDown, PullNone};
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

/// Desired PWM output frequency in Hz.
pub(crate) const PWM_FREQ_HZ: u32 = 1000;

/// PWM counter wrap value (period - 1).
pub(crate) const PWM_WRAP: u32 = 10000 - 1;

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

/// Format a duty percentage into a fixed byte buffer as "Duty: NNN%\r\n".
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 16 bytes).
/// * `duty` - Duty cycle percentage to format.
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub(crate) fn format_duty(buf: &mut [u8], duty: u8) -> usize {
    buf[..6].copy_from_slice(b"Duty: ");
    let mut pos = 6;
    pos += write_duty_digits(&mut buf[pos..], duty);
    buf[pos] = b'%'; pos += 1;
    buf[pos] = b'\r'; pos += 1;
    buf[pos] = b'\n'; pos += 1;
    pos
}

/// Write the 3-character right-justified duty digits into `buf`.
fn write_duty_digits(buf: &mut [u8], duty: u8) -> usize {
    if duty >= 100 {
        buf[0] = b'1'; buf[1] = b'0'; buf[2] = b'0';
    } else if duty >= 10 {
        buf[0] = b' '; buf[1] = b'0' + duty / 10; buf[2] = b'0' + duty % 10;
    } else {
        buf[0] = b' '; buf[1] = b' '; buf[2] = b'0' + duty;
    }
    3
}

/// Sweep the PWM duty cycle from 0% to 100% in steps of 5.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel to set duty on.
/// * `delay` - Delay provider for 50 ms pauses.
/// * `buf` - Scratch buffer for formatting output.
pub(crate) fn sweep_up(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 16],
) {
    let mut duty: u8 = 0;
    while duty <= 100 {
        apply_duty(uart, channel, delay, buf, duty);
        duty += 5;
    }
}

/// Sweep the PWM duty cycle from 100% to 0% in steps of 5.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel to set duty on.
/// * `delay` - Delay provider for 50 ms pauses.
/// * `buf` - Scratch buffer for formatting output.
pub(crate) fn sweep_down(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 16],
) {
    let mut duty: i8 = 100;
    while duty >= 0 {
        apply_duty(uart, channel, delay, buf, duty as u8);
        duty -= 5;
    }
}

/// Apply a single duty step: set PWM level, format, print, and delay.
fn apply_duty(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 16],
    duty: u8,
) {
    let level = crate::pwm::duty_to_level(duty, PWM_WRAP) as u16;
    channel.set_duty_cycle(level).ok();
    let n = format_duty(buf, duty);
    uart.write_full_blocking(&buf[..n]);
    delay.delay_ms(50u32);
}

/// Type alias for PWM slice 4 (onboard LED, GPIO 25).
type PwmSlice4 = hal::pwm::Slice<hal::pwm::Pwm4, hal::pwm::FreeRunning>;

/// Initialise all peripherals and run the PWM breathing demo.
///
/// # Arguments
///
/// * `pac` - PAC Peripherals singleton (consumed).
pub(crate) fn run(mut pac: hal::pac::Peripherals) -> ! {
    let mut wd = hal::Watchdog::new(pac.WATCHDOG);
    let clocks = init_clocks(pac.XOSC, pac.CLOCKS, pac.PLL_SYS, pac.PLL_USB, &mut pac.RESETS, &mut wd);
    let pins = init_pins(pac.IO_BANK0, pac.PADS_BANK0, pac.SIO, &mut pac.RESETS);
    let uart = init_uart(pac.UART0, pins.gpio0, pins.gpio1, &mut pac.RESETS, &clocks);
    let mut delay = init_delay(&clocks);
    let mut pwm = init_pwm(pac.PWM, &mut pac.RESETS, &clocks, pins.gpio25);
    uart.write_full_blocking(b"PWM initialized: GPIO25 @ 1000 Hz\r\n");
    pwm_loop(&uart, &mut pwm, &mut delay)
}

/// Configure PWM slice 4 for 1 kHz output on channel B (GPIO 25).
///
/// # Arguments
///
/// * `pwm_pac` - PAC PWM peripheral singleton.
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `clocks` - Reference to the initialised clock configuration.
/// * `led_pin` - Default GPIO 25 pin to bind to PWM channel B.
///
/// # Returns
///
/// Configured PWM slice 4 in free-running mode.
fn init_pwm(
    pwm_pac: hal::pac::PWM,
    resets: &mut hal::pac::RESETS,
    clocks: &hal::clocks::ClocksManager,
    led_pin: Pin<hal::gpio::bank0::Gpio25, FunctionNull, PullDown>,
) -> PwmSlice4 {
    let slices = hal::pwm::Slices::new(pwm_pac, resets);
    let mut slice = slices.pwm4;
    configure_pwm_div(&mut slice, clocks);
    slice.set_top(PWM_WRAP as u16);
    slice.enable();
    slice.channel_b.output_to(led_pin);
    slice
}

/// Set the clock divider for a PWM slice based on the system clock.
///
/// # Arguments
///
/// * `slice` - Mutable reference to the PWM slice to configure.
/// * `clocks` - Reference to the initialised clock configuration.
fn configure_pwm_div(slice: &mut PwmSlice4, clocks: &hal::clocks::ClocksManager) {
    let sys_hz = clocks.system_clock.freq().to_Hz();
    let div = crate::pwm::calc_clk_div(sys_hz, PWM_FREQ_HZ, PWM_WRAP);
    let div_int = div as u8;
    slice.set_div_int(div_int);
    slice.set_div_frac((((div - div_int as f32) * 16.0) as u8).min(15));
}

/// Run the PWM duty-cycle sweep loop forever.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `pwm` - Mutable reference to the configured PWM slice.
/// * `delay` - Mutable reference to the blocking delay provider.
fn pwm_loop(uart: &EnabledUart, pwm: &mut PwmSlice4, delay: &mut cortex_m::delay::Delay) -> ! {
    let mut buf = [0u8; 16];
    loop {
        sweep_up(uart, &mut pwm.channel_b, delay, &mut buf);
        sweep_down(uart, &mut pwm.channel_b, delay, &mut buf);
    }
}

// End of file
