#!/usr/bin/env python3
"""Convert and explain IEEE 754 float/hex operations step-by-step."""

import argparse
import struct
import sys


def _exp_str(e_val: int, bias: int, is_64: bool) -> str:
    """
    Get accurate true exponent string accounting for IEEE 754 edge cases.

    Parameters
    ----------
    e_val : int
        The stored exponent value.
    bias : int
        The exponent bias (127 or 1023).
    is_64 : bool
        True if 64-bit precision.

    Returns
    -------
    str
        The formatted true exponent explanation string.
    """
    if e_val == 0:
        return f"0 (Zero/Subnormal, True Exp: {1 - bias})"
    if e_val == (2047 if is_64 else 255):
        return f"{e_val} (Inf/NaN flag)"
    return f"{e_val} - {bias} = {e_val - bias}"


def _print_encode_steps(val: float, b: int) -> None:
    """
    Print the math steps for encoding a float to hex.

    Parameters
    ----------
    val : float
        The float value to encode.
    b : int
        The bit size (32 or 64).

    Returns
    -------
    None
    """
    f1, f2, bias = ("<Q", "<d", 1023) if b == 64 else ("<I", "<f", 127)
    bstr = f"{struct.unpack(f1, struct.pack(f2, val))[0]:0{b}b}"
    s, e, m = (
        bstr[0],
        bstr[1 : 1 + (11 if b == 64 else 8)],
        bstr[1 + (11 if b == 64 else 8) :],
    )
    hex_val = f"0x{int(bstr, 2):0{b//4}X}"
    print(f"\n[ENCODE {val} to {b}-bit]\n1. Sign: {s} (0=Pos, 1=Neg)")
    print(f"2. Exp: {_exp_str(int(e, 2), bias, b == 64)}\n3. Mantissa: {m}")
    print(f"4. Full: {s} {e} {m}\n5. Hex: {hex_val}")
    if b == 64:
        raw_hex = f"{int(bstr, 2):016X}"
        r3 = f"0x{raw_hex[:8]}"
        r2 = f"0x{raw_hex[8:]}"
        print(f"6. ARM Regs: r3 (high) = {r3}, r2 (low) = {r2} (e.g. in printf)")
    else:
        print(f"6. ARM Reg: Single 32-bit register ({hex_val})")


def _print_decode_steps(hex_str: str) -> None:
    """
    Print the math steps for decoding a hex string to float.

    Parameters
    ----------
    hex_str : str
        The hex string to decode.

    Returns
    -------
    None
    """
    c = hex_str.lower().removeprefix("0x")
    b, f1, f2, bias = (64, "<Q", "<d", 1023) if len(c) > 8 else (32, "<I", "<f", 127)
    bstr = f"{int(c, 16):0{b}b}"
    s, e, m = (
        bstr[0],
        bstr[1 : 1 + (11 if b == 64 else 8)],
        bstr[1 + (11 if b == 64 else 8) :],
    )
    print(f"\n[DECODE 0x{c.zfill(b//4).upper()} ({b}-bit)]\n1. Binary: {s} {e} {m}")
    print(f"2. Sign: {s}\n3. Exp: {_exp_str(int(e, 2), bias, b == 64)}")
    print(f"4. Value: {struct.unpack(f2, struct.pack(f1, int(c, 16)))[0]}")
    if b == 64:
        raw_hex = c.zfill(16).upper()
        r3 = f"0x{raw_hex[:8]}"
        r2 = f"0x{raw_hex[8:]}"
        print(f"5. ARM Regs: r3 (high) = {r3}, r2 (low) = {r2} (e.g. in printf)")
    else:
        print(f"5. ARM Reg: Single 32-bit register (0x{c.zfill(8).upper()})")


def _get_args() -> str:
    """
    Parse CLI arguments.

    Parameters
    ----------
    None

    Returns
    -------
    str
        The parsed input value string.
    """
    parser = argparse.ArgumentParser(description="Float/Hex step converter.")
    parser.add_argument("val", help="Hex (0x...) or Float value to convert.")
    return parser.parse_args().val.strip()


def _process_conversion(val_str: str) -> None:
    """
    Execute the conversion and print the output.

    Parameters
    ----------
    val_str : str
        The raw input string to process.

    Returns
    -------
    None
    """
    if val_str.lower().startswith("0x"):
        _print_decode_steps(val_str)
    else:
        _print_encode_steps(float(val_str), 32)
        _print_encode_steps(float(val_str), 64)


def main() -> int:
    """
    Execute the conversion pipeline based on CLI arguments.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on successful conversion, otherwise non-zero.
    """
    try:
        _process_conversion(_get_args())
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
