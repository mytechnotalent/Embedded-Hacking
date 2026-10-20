# pico-gdb-memory.gdb - Restrict GDB memory access to valid RP2350 / Pico 2 W memory ranges.
#
# Prevents Binary Ninja register-value preview dereferences and out-of-bounds reads
# from triggering OpenOCD SWD faults and debug port reconnects (SWD DPIDR storms).
#
# In Binary Ninja's Debugger Console (with GDB MI adapter connected), run:
#   source pico-gdb-memory.gdb
# (or provide the full path to this file).

delete mem
mem 0x10000000 0x10400000 ro
mem 0x20000000 0x20082000 rw
set mem inaccessible-by-default on
info mem
