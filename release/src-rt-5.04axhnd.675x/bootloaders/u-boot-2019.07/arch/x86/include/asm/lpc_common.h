/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __ASM_LPC_COMMON_H
#define __ASM_LPC_COMMON_H

#define PCH_RCBA_BASE		0xf0

#define RC		0x3400	/* 32bit */
#define GCS		0x3410	/* 32bit */

#define PMBASE			0x40
#define ACPI_CNTL		0x44

#define LPC_IO_DEC		0x80 /* IO Decode Ranges Register */
#define  COMB_DEC_RANGE		(1 << 4)  /* 0x2f8-0x2ff (COM2) */
#define  COMA_DEC_RANGE		(0 << 0)  /* 0x3f8-0x3ff (COM1) */
#define LPC_EN			0x82 /* LPC IF Enables Register */
#define  CNF2_LPC_EN		(1 << 13) /* 0x4e/0x4f */
#define  CNF1_LPC_EN		(1 << 12) /* 0x2e/0x2f */
#define  MC_LPC_EN		(1 << 11) /* 0x62/0x66 */
#define  KBC_LPC_EN		(1 << 10) /* 0x60/0x64 */
#define  GAMEH_LPC_EN		(1 << 9)  /* 0x208/0x20f */
#define  GAMEL_LPC_EN		(1 << 8)  /* 0x200/0x207 */
#define  FDD_LPC_EN		(1 << 3)  /* LPC_IO_DEC[12] */
#define  LPT_LPC_EN		(1 << 2)  /* LPC_IO_DEC[9:8] */
#define  COMB_LPC_EN		(1 << 1)  /* LPC_IO_DEC[6:4] */
#define  COMA_LPC_EN		(1 << 0)  /* LPC_IO_DEC[3:2] */
#define LPC_GEN1_DEC		0x84 /* LPC IF Generic Decode Range 1 */
#define LPC_GEN2_DEC		0x88 /* LPC IF Generic Decode Range 2 */
#define LPC_GEN3_DEC		0x8c /* LPC IF Generic Decode Range 3 */
#define LPC_GEN4_DEC		0x90 /* LPC IF Generic Decode Range 4 */
#define LPC_GENX_DEC(x)		(0x84 + 4 * (x))
#define  GEN_DEC_RANGE_256B	0xfc0000  /* 256 Bytes */
#define  GEN_DEC_RANGE_128B	0x7c0000  /* 128 Bytes */
#define  GEN_DEC_RANGE_64B	0x3c0000  /* 64 Bytes */
#define  GEN_DEC_RANGE_32B	0x1c0000  /* 32 Bytes */
#define  GEN_DEC_RANGE_16B	0x0c0000  /* 16 Bytes */
#define  GEN_DEC_RANGE_8B	0x040000  /* 8 Bytes */
#define  GEN_DEC_RANGE_4B	0x000000  /* 4 Bytes */
#define  GEN_DEC_RANGE_EN	(1 << 0)  /* Range Enable */

/**
 * lpc_common_early_init() - Set up common LPC init
 *
 * This sets up the legacy decode areas, GEN_DEC, SPI prefetch and Port80. It
 * also puts the RCB in the correct place so that RCB_REG() works.
 *
 * @dev:	LPC device (a child of the PCH)
 * @return 0 on success, -ve on error
 */
int lpc_common_early_init(struct udevice *dev);

int lpc_set_spi_protect(struct udevice *dev, int bios_ctrl, bool protect);

#endif
