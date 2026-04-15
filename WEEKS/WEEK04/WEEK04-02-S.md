# Embedded Systems Reverse Engineering
[Repository](https://github.com/mytechnotalent/Embedded-Hacking)

## Week 4
Variables in Embedded Systems: Debugging and Hacking Variables w/ GPIO Output Basics

### Non-Credit Practice Exercise 2 Solution: Patch Binary to Change Variable Value

#### Answers

##### Patch Details

| Item               | Original      | Patched       |
|--------------------|-------------- |---------------|
| Instruction        | movs r1, #0x2b | movs r1, #0x46 |
| Hex bytes          | 21 2b         | 21 46         |
| Decimal value      | 43            | 70            |
| Output             | age: 43       | age: 70       |

##### Patching Steps

1. Located `movs r1, #0x2b` in Ghidra Listing view
2. Right-click → **Patch Instruction** → changed `#0x2b` to `#0x46`
3. Verified in Decompile window: `printf("age: %d\r\n", 0x46)`
4. Exported: **File → Export Program → Binary** → `0x0005_intro-to-variables-h.bin`
5. Converted to UF2:
   ```powershell
   python ..\uf2conv.py build\0x0005_intro-to-variables-h.bin --base 0x10000000 --family 0xe48bff59 --output build\hacked.uf2
   ```
6. Flashed via BOOTSEL → RPI-RP2 drive

##### Serial Output

```
age: 70
age: 70
age: 70
...
```

#### Reflection Answers

1. **Why do we need to convert to UF2 format instead of flashing the raw .bin file?**
   The RP2350 bootloader (accessed via BOOTSEL) expects **UF2 (USB Flashing Format)** files. UF2 is a container format that includes metadata: the target flash address for each 256-byte block, a family ID (`0xe48bff59` for RP2350), and checksums. A raw `.bin` file contains only code bytes with no addressing information—the bootloader wouldn't know where in flash to write the data. UF2 also supports partial updates and is self-describing, making it safer for USB mass storage flashing.

2. **What is the significance of the base address 0x10000000 in the conversion command?**
   `0x10000000` is the **XIP (Execute In Place) flash base address** on the RP2350. This tells the UF2 converter that byte 0 of the binary should be written to flash address `0x10000000`. The CPU fetches instructions directly from this address range via the XIP controller. If the base address were wrong (e.g., `0x20000000` for RAM), the code would be written to the wrong location and the processor would fail to boot because the vector table wouldn't be found at the expected address.

3. **What would happen if you patched the wrong instruction by mistake?**
   The consequences depend on what was changed: (a) Patching a different `movs` might corrupt an unrelated function parameter, causing incorrect behavior or a crash. (b) Patching opcode bytes (not just the immediate) could create an invalid instruction, triggering a HardFault or UsageFault. (c) Patching inside a multi-byte instruction could split it into two unintended instructions, corrupting the entire subsequent instruction stream. The program would likely crash, output garbage, or hang—requiring reflashing with the original UF2 to recover.

4. **How can you verify a patch was applied correctly before exporting?**
   Multiple verification methods: (a) Check the **Decompile window**—it should reflect the new value (e.g., `printf("age: %d\r\n", 0x46)`). (b) Inspect the **Listing window** bytes—confirm the instruction bytes changed from `21 2b` to `21 46`. (c) Use **right-click → Highlight → Highlight Difference** to see patched bytes highlighted. (d) Compare the patched instruction against the ARM Thumb encoding reference to verify the encoding is valid. (e) Check surrounding instructions are unchanged—patches should not accidentally modify adjacent code.
