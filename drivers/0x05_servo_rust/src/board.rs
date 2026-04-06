//! @file board.rs
//! @brief Board-level HAL helpers for the servo driver
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
#[cfg(rp2040)]
use rp2040_hal as hal;
#[cfg(rp2350)]
use rp235x_hal as hal;

/// External crystal frequency in Hz (12 MHz).
pub(crate) const XTAL_FREQ_HZ: u32 = 12_000_000u32;

/// UART baud rate in bits per second.
pub(crate) const UART_BAUD: u32 = 115_200;

/// Angle increment per sweep step in degrees.
pub(crate) const STEP_DEGREES: i32 = 10;

/// Delay between sweep steps in milliseconds.
pub(crate) const STEP_DELAY_MS: u32 = 150;

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

/// Write 3-character right-justified angle digits into `buf`.
fn write_angle_digits(buf: &mut [u8], a: u32) -> usize {
    if a >= 100 {
        write_angle_hundreds(buf, a);
    } else if a >= 10 {
        write_angle_tens(buf, a);
    } else {
        write_angle_ones(buf, a);
    }
    3
}

/// Write digits for angles >= 100.
fn write_angle_hundreds(buf: &mut [u8], a: u32) {
    buf[0] = b'0' + (a / 100) as u8;
    buf[1] = b'0' + ((a / 10) % 10) as u8;
    buf[2] = b'0' + (a % 10) as u8;
}

/// Write digits for angles 10..99 with leading space.
fn write_angle_tens(buf: &mut [u8], a: u32) {
    buf[0] = b' ';
    buf[1] = b'0' + (a / 10) as u8;
    buf[2] = b'0' + (a % 10) as u8;
}

/// Write digit for angles 0..9 with leading spaces.
fn write_angle_ones(buf: &mut [u8], a: u32) {
    buf[0] = b' ';
    buf[1] = b' ';
    buf[2] = b'0' + a as u8;
}

/// Format an angle into "Angle: NNN deg\r\n".
///
/// # Arguments
///
/// * `buf` - Mutable byte slice (must be at least 20 bytes).
/// * `angle` - Angle in degrees (0..180).
///
/// # Returns
///
/// Number of bytes written into the buffer.
pub(crate) fn format_angle(buf: &mut [u8], angle: i32) -> usize {
    buf[..7].copy_from_slice(b"Angle: ");
    let mut pos = 7;
    let a = if angle < 0 { 0 } else { angle as u32 };
    pos += write_angle_digits(&mut buf[pos..], a);
    buf[pos..pos + 6].copy_from_slice(b" deg\r\n");
    pos + 6
}

/// Sweep the servo angle upward from 0 to 180 in STEP_DEGREES increments.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel implementing SetDutyCycle.
/// * `delay` - Delay provider for pause between steps.
/// * `buf` - Scratch buffer for formatting output.
pub(crate) fn sweep_angle_up(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 20],
) {
    let mut angle: i32 = 0;
    while angle <= 180 {
        apply_angle(uart, channel, delay, buf, angle);
        angle += STEP_DEGREES;
    }
}

/// Sweep the servo angle downward from 180 to 0 in STEP_DEGREES decrements.
///
/// # Arguments
///
/// * `uart` - UART peripheral for serial output.
/// * `channel` - PWM channel implementing SetDutyCycle.
/// * `delay` - Delay provider for pause between steps.
/// * `buf` - Scratch buffer for formatting output.
pub(crate) fn sweep_angle_down(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 20],
) {
    let mut angle: i32 = 180;
    while angle >= 0 {
        apply_angle(uart, channel, delay, buf, angle);
        angle -= STEP_DEGREES;
    }
}

/// Apply a single angle step: compute pulse, set PWM, format, print, delay.
fn apply_angle(
    uart: &EnabledUart,
    channel: &mut impl SetDutyCycle,
    delay: &mut cortex_m::delay::Delay,
    buf: &mut [u8; 20],
    angle: i32,
) {
    let level = compute_servo_level(angle) as u16;
    channel.set_duty_cycle(level).ok();
    let n = format_angle(buf, angle);
    uart.write_full_blocking(&buf[..n]);
    delay.delay_ms(STEP_DELAY_MS);
}

/// Compute the pulse width in microseconds for the given angle.
fn compute_pulse_us(angle: i32) -> u32 {
    crate::servo::angle_to_pulse_us(
        angle as f32,
        crate::servo::SERVO_DEFAULT_MIN_US,
        crate::servo::SERVO_DEFAULT_MAX_US,
    ) as u32
}

/// Compute the PWM level for a given angle using servo constants.
fn compute_servo_level(angle: i32) -> u32 {
    crate::servo::pulse_us_to_level(
        compute_pulse_us(angle),
        crate::servo::SERVO_WRAP,
        crate::servo::SERVO_HZ,
    )
}

/// Type alias for PWM slice 3 (servo on GPIO 6, channel A).
type PwmSlice3 = hal::pwm::Slice<hal::pwm::Pwm3, hal::pwm::FreeRunning>;

/// Initialise all peripherals and run the servo sweep demo.
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
    let mut pwm = init_servo_pwm(pac.PWM, &mut pac.RESETS, &clocks, pins.gpio6);
    announce_servo(&uart);
    servo_loop(&uart, &mut pwm, &mut delay)
}

/// Configure PWM slice 3 for 50 Hz servo output on channel A (GPIO 6).
///
/// # Arguments
///
/// * `pwm_pac` - PAC PWM peripheral singleton.
/// * `resets` - Mutable reference to the RESETS peripheral.
/// * `clocks` - Reference to the initialised clock configuration.
/// * `servo_pin` - Default GPIO 6 pin to bind to PWM channel A.
///
/// # Returns
///
/// Configured PWM slice 3 in free-running mode.
fn init_servo_pwm(
    pwm_pac: hal::pac::PWM,
    resets: &mut hal::pac::RESETS,
    clocks: &hal::clocks::ClocksManager,
    servo_pin: Pin<hal::gpio::bank0::Gpio6, FunctionNull, PullDown>,
) -> PwmSlice3 {
    let slices = hal::pwm::Slices::new(pwm_pac, resets);
    let mut slice = slices.pwm3;
    configure_servo_div(&mut slice, clocks);
    slice.enable();
    slice.channel_a.output_to(servo_pin);
    slice
}

/// Set the clock divider and wrap for a servo PWM slice.
///
/// # Arguments
///
/// * `slice` - Mutable reference to the PWM slice to configure.
/// * `clocks` - Reference to the initialised clock configuration.
fn configure_servo_div(slice: &mut PwmSlice3, clocks: &hal::clocks::ClocksManager) {
    let sys_hz = clocks.system_clock.freq().to_Hz();
    let div = crate::servo::calc_clk_div(sys_hz, crate::servo::SERVO_HZ, crate::servo::SERVO_WRAP);
    let div_int = div as u8;
    slice.set_div_int(div_int);
    slice.set_div_frac((((div - div_int as f32) * 16.0) as u8).min(15));
    slice.set_top(crate::servo::SERVO_WRAP as u16);
}

/// Print the servo initialisation banner over UART.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
fn announce_servo(uart: &EnabledUart) {
    uart.write_full_blocking(b"Servo driver initialized on GPIO 6\r\n");
    uart.write_full_blocking(b"Sweeping 0 -> 180 -> 0 degrees in 10-degree steps\r\n");
}

/// Run the servo angle sweep loop forever.
///
/// # Arguments
///
/// * `uart` - Reference to the enabled UART peripheral for serial output.
/// * `pwm` - Mutable reference to the configured PWM slice.
/// * `delay` - Mutable reference to the blocking delay provider.
fn servo_loop(uart: &EnabledUart, pwm: &mut PwmSlice3, delay: &mut cortex_m::delay::Delay) -> ! {
    let mut buf = [0u8; 20];
    loop {
        sweep_angle_up(uart, &mut pwm.channel_a, delay, &mut buf);
        sweep_angle_down(uart, &mut pwm.channel_a, delay, &mut buf);
    }
}

// End of file
