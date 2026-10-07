#!/usr/bin/env python3
# MIT License
#
# Copyright (c) 2026 Kevin Thomas
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Author:  Kevin Thomas
# Email:   kevin@mytechnotalent.com
# GitHub:  https://github.com/mytechnotalent
# File:    lora_console.py
# Desc:    Interactive REYAX RYLR998 terminal for the FT232RL ground station.
# Created: 2026

"""
Interactive LoRa terminal for the REYAX RYLR998 ground station.

Sends AT commands as complete bursts terminated with a carriage return and line
feed to avoid the RYLR998 inter-character timeout error, and prints incoming
packets from the airborne node as they arrive.
"""

import argparse
import sys
import threading
import time

try:
    import serial
except ImportError:
    serial = None


def _reader_thread(ser: "serial.Serial") -> None:
    """
    Continuously read and display incoming packets from the radio.

    Parameters
    ----------
    ser : serial.Serial
        Open serial connection to the ground RYLR998.

    Returns
    -------
    None
    """
    while True:
        try:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line:
                print(f"\n[LORA RX] {line}\n> ", end="", flush=True)
        except Exception:
            break


def _parse_args() -> argparse.Namespace:
    """
    Parse command line arguments for the LoRa console.

    Parameters
    ----------
    None

    Returns
    -------
    argparse.Namespace
        Parsed arguments with the serial port and baud rate.
    """
    parser = argparse.ArgumentParser(description="REYAX RYLR998 console")
    parser.add_argument("--port", default="/dev/cu.usbserial-A50285BI")
    parser.add_argument("--baud", type=int, default=115200)
    return parser.parse_args()


def main() -> None:
    """
    Open the ground station radio and forward operator input.

    Parameters
    ----------
    None

    Returns
    -------
    None
    """
    if serial is None:
        sys.exit("pyserial required: pip install pyserial")
    args = _parse_args()

    print(f"[*] Opening REYAX RYLR998 on {args.port} @ {args.baud}...")
    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.5)
    except Exception as e:
        sys.exit(f"[-] Failed to open {args.port}: {e}")

    thread = threading.Thread(target=_reader_thread, args=(ser,), daemon=True)
    thread.start()

    time.sleep(0.1)
    ser.write(b"AT\r\n")

    print("[+] Connected. Type AT commands (e.g. AT, AT+BAND?, AT+NETWORKID?).")
    print("[+] Incoming airborne packets print as [LORA RX] +RCV=...")
    print("[+] Press Ctrl-C or Ctrl-D to exit.\n")

    try:
        while True:
            cmd = input("> ").strip()
            if not cmd:
                continue
            ser.write(cmd.encode("utf-8") + b"\r\n")
    except (KeyboardInterrupt, EOFError):
        print("\n[*] Exiting LoRa console.")
        ser.close()


if __name__ == "__main__":
    main()
