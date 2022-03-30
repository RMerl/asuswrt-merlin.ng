/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ARM64 header for proper chain-loading with Little Kernel.
 *
 * Little Kernel shipped with Dragonboard410C boots standard Linux images for
 * ARM64. This file adds header that is required to boot U-Boot properly.
 *
 * For details see:
 * https://www.kernel.org/doc/Documentation/arm64/booting.txt
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <config.h>

.global _arm64_header
_arm64_header:
	b _start
	.word 0
	/* Image load offset from start of RAM, little-endian */
	.quad   CONFIG_SYS_TEXT_BASE-PHYS_SDRAM_1
	/* Effective size of kernel image, little-endian */
	.quad   0 /* 0x60000 - ignored */
	/* Informative flags, little-endian */
	.quad   0
	.quad   0                               /* reserved */
	.quad   0                               /* reserved */
	.quad   0                               /* reserved */
	.byte   0x41                            /* Magic number, "ARM\x64" */
	.byte   0x52
	.byte   0x4d
	.byte   0x64
	.word   0                               /* reserved */
