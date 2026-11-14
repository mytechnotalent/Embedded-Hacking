/**
 * FILE: linker.ld
 *
 * DESCRIPTION:
 * PALE HORSE Ghost Linker Script.
 *
 * BRIEF:
 * Links the ghost stage to run at 0x20040000 in SRAM. The stage-2 loader
 * decrypts it into that address and the application branches into it.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

ENTRY(Ghost_Entry)

MEMORY
{
  GHOST (rwx) : ORIGIN = 0x20040000, LENGTH = 0x1000
}

SECTIONS
{
  . = ORIGIN(GHOST);

  .text :
  {
    . = ALIGN(4);
    KEEP(*(.text.Ghost_Entry))
    *(.text*)
    *(.rodata*)
    *(.ARM.attributes)
  } > GHOST

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    *(.bss*)
    *(COMMON)
  } > GHOST

  ASSERT((SIZEOF(.text) <= 0x1000), "Ghost does not fit in its 4KB SRAM region")
}
