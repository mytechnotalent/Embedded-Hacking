/**
 * FILE: image_def.s
 *
 * DESCRIPTION:
 * PALE HORSE Stage-2 IMAGE_DEF Block.
 *
 * BRIEF:
 * Minimal PICOBIN metadata so the RP2350 boot ROM recognises the image
 * as a valid program. Must appear within the first 4 kB of the flash image.
 *
 * AUTHOR: Kevin Thomas
 * CREATION DATE: November 13, 2026
 * UPDATE DATE: November 13, 2026
 */

.section .embedded_block, "a"                    // place IMAGE_DEF block first

.word  0xffffded3                                // PICOBIN_BLOCK_MARKER_START
.byte  0x42                                      // PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE
.byte  0x1                                       // item is 1 word in size
.hword 0b0001000000100001                        // SECURE mode (0x1021)
.byte  0xff                                      // PICOBIN_BLOCK_ITEM_2BS_LAST
.hword 0x0001                                    // item is 1 word in size
.byte  0x0                                       // pad
.word  0x0                                       // relative pointer to next block
.word  0xab123579                                // PICOBIN_BLOCK_MARKER_END
