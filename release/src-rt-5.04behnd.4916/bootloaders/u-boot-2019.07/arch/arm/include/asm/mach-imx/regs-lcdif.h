/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale i.MX28/6SX/6UL/7D LCDIF Register Definitions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright 2008-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#ifndef __IMX_REGS_LCDIF_H__
#define __IMX_REGS_LCDIF_H__

#ifndef	__ASSEMBLY__
#include <asm/mach-imx/regs-common.h>

struct mxs_lcdif_regs {
	mxs_reg_32(hw_lcdif_ctrl)		/* 0x00 */
	mxs_reg_32(hw_lcdif_ctrl1)		/* 0x10 */
#if defined(CONFIG_MX28) || defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6SL) || defined(CONFIG_MX6SLL) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL) || \
	defined(CONFIG_MX7) || defined(CONFIG_MX7ULP) || \
	defined(CONFIG_IMX8M)
	mxs_reg_32(hw_lcdif_ctrl2)		/* 0x20 */
#endif
	mxs_reg_32(hw_lcdif_transfer_count)	/* 0x20/0x30 */
	mxs_reg_32(hw_lcdif_cur_buf)		/* 0x30/0x40 */
	mxs_reg_32(hw_lcdif_next_buf)		/* 0x40/0x50 */

#if defined(CONFIG_MX23)
	uint32_t	reserved1[4];
#endif

	mxs_reg_32(hw_lcdif_timing)		/* 0x60 */
	mxs_reg_32(hw_lcdif_vdctrl0)		/* 0x70 */
	mxs_reg_32(hw_lcdif_vdctrl1)		/* 0x80 */
	mxs_reg_32(hw_lcdif_vdctrl2)		/* 0x90 */
	mxs_reg_32(hw_lcdif_vdctrl3)		/* 0xa0 */
	mxs_reg_32(hw_lcdif_vdctrl4)		/* 0xb0 */
	mxs_reg_32(hw_lcdif_dvictrl0)		/* 0xc0 */
	mxs_reg_32(hw_lcdif_dvictrl1)		/* 0xd0 */
	mxs_reg_32(hw_lcdif_dvictrl2)		/* 0xe0 */
	mxs_reg_32(hw_lcdif_dvictrl3)		/* 0xf0 */
	mxs_reg_32(hw_lcdif_dvictrl4)		/* 0x100 */
	mxs_reg_32(hw_lcdif_csc_coeffctrl0)	/* 0x110 */
	mxs_reg_32(hw_lcdif_csc_coeffctrl1)	/* 0x120 */
	mxs_reg_32(hw_lcdif_csc_coeffctrl2)	/* 0x130 */
	mxs_reg_32(hw_lcdif_csc_coeffctrl3)	/* 0x140 */
	mxs_reg_32(hw_lcdif_csc_coeffctrl4)	/* 0x150 */
	mxs_reg_32(hw_lcdif_csc_offset)	/* 0x160 */
	mxs_reg_32(hw_lcdif_csc_limit)		/* 0x170 */

#if defined(CONFIG_MX23)
	uint32_t	reserved2[12];
#endif
	mxs_reg_32(hw_lcdif_data)		/* 0x1b0/0x180 */
	mxs_reg_32(hw_lcdif_bm_error_stat)	/* 0x1c0/0x190 */
#if defined(CONFIG_MX28) || defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6SL) || defined(CONFIG_MX6SLL) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL) || \
	defined(CONFIG_MX7) || defined(CONFIG_MX7ULP) || \
	defined(CONFIG_IMX8M)
	mxs_reg_32(hw_lcdif_crc_stat)		/* 0x1a0 */
#endif
	mxs_reg_32(hw_lcdif_lcdif_stat)		/* 0x1d0/0x1b0 */
	mxs_reg_32(hw_lcdif_version)		/* 0x1e0/0x1c0 */
	mxs_reg_32(hw_lcdif_debug0)		/* 0x1f0/0x1d0 */
	mxs_reg_32(hw_lcdif_debug1)		/* 0x200/0x1e0 */
	mxs_reg_32(hw_lcdif_debug2)		/* 0x1f0 */
#if defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6SL) || defined(CONFIG_MX6SLL) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL) || \
	defined(CONFIG_MX7) || defined(CONFIG_MX7ULP) || \
	defined(CONFIG_IMX8M)
	mxs_reg_32(hw_lcdif_thres)
	mxs_reg_32(hw_lcdif_as_ctrl)
	mxs_reg_32(hw_lcdif_as_buf)
	mxs_reg_32(hw_lcdif_as_next_buf)
	mxs_reg_32(hw_lcdif_as_clrkeylow)
	mxs_reg_32(hw_lcdif_as_clrkeyhigh)
	mxs_reg_32(hw_lcdif_as_sync_delay)
	mxs_reg_32(hw_lcdif_as_debug3)
	mxs_reg_32(hw_lcdif_as_debug4)
	mxs_reg_32(hw_lcdif_as_debug5)
#endif
};
#endif

#define	LCDIF_CTRL_SFTRST					(1 << 31)
#define	LCDIF_CTRL_CLKGATE					(1 << 30)
#define	LCDIF_CTRL_YCBCR422_INPUT				(1 << 29)
#define	LCDIF_CTRL_READ_WRITEB					(1 << 28)
#define	LCDIF_CTRL_WAIT_FOR_VSYNC_EDGE				(1 << 27)
#define	LCDIF_CTRL_DATA_SHIFT_DIR				(1 << 26)
#define	LCDIF_CTRL_SHIFT_NUM_BITS_MASK				(0x1f << 21)
#define	LCDIF_CTRL_SHIFT_NUM_BITS_OFFSET			21
#define	LCDIF_CTRL_DVI_MODE					(1 << 20)
#define	LCDIF_CTRL_BYPASS_COUNT					(1 << 19)
#define	LCDIF_CTRL_VSYNC_MODE					(1 << 18)
#define	LCDIF_CTRL_DOTCLK_MODE					(1 << 17)
#define	LCDIF_CTRL_DATA_SELECT					(1 << 16)
#define	LCDIF_CTRL_INPUT_DATA_SWIZZLE_MASK			(0x3 << 14)
#define	LCDIF_CTRL_INPUT_DATA_SWIZZLE_OFFSET			14
#define	LCDIF_CTRL_CSC_DATA_SWIZZLE_MASK			(0x3 << 12)
#define	LCDIF_CTRL_CSC_DATA_SWIZZLE_OFFSET			12
#define	LCDIF_CTRL_LCD_DATABUS_WIDTH_MASK			(0x3 << 10)
#define	LCDIF_CTRL_LCD_DATABUS_WIDTH_OFFSET			10
#define	LCDIF_CTRL_LCD_DATABUS_WIDTH_16BIT			(0 << 10)
#define	LCDIF_CTRL_LCD_DATABUS_WIDTH_8BIT			(1 << 10)
#define	LCDIF_CTRL_LCD_DATABUS_WIDTH_18BIT			(2 << 10)
#define	LCDIF_CTRL_LCD_DATABUS_WIDTH_24BIT			(3 << 10)
#define	LCDIF_CTRL_WORD_LENGTH_MASK				(0x3 << 8)
#define	LCDIF_CTRL_WORD_LENGTH_OFFSET				8
#define	LCDIF_CTRL_WORD_LENGTH_16BIT				(0 << 8)
#define	LCDIF_CTRL_WORD_LENGTH_8BIT				(1 << 8)
#define	LCDIF_CTRL_WORD_LENGTH_18BIT				(2 << 8)
#define	LCDIF_CTRL_WORD_LENGTH_24BIT				(3 << 8)
#define	LCDIF_CTRL_RGB_TO_YCBCR422_CSC				(1 << 7)
#define	LCDIF_CTRL_LCDIF_MASTER					(1 << 5)
#define	LCDIF_CTRL_DATA_FORMAT_16_BIT				(1 << 3)
#define	LCDIF_CTRL_DATA_FORMAT_18_BIT				(1 << 2)
#define	LCDIF_CTRL_DATA_FORMAT_24_BIT				(1 << 1)
#define	LCDIF_CTRL_RUN						(1 << 0)

#define	LCDIF_CTRL1_COMBINE_MPU_WR_STRB				(1 << 27)
#define	LCDIF_CTRL1_BM_ERROR_IRQ_EN				(1 << 26)
#define	LCDIF_CTRL1_BM_ERROR_IRQ				(1 << 25)
#define	LCDIF_CTRL1_RECOVER_ON_UNDERFLOW			(1 << 24)
#define	LCDIF_CTRL1_INTERLACE_FIELDS				(1 << 23)
#define	LCDIF_CTRL1_START_INTERLACE_FROM_SECOND_FIELD		(1 << 22)
#define	LCDIF_CTRL1_FIFO_CLEAR					(1 << 21)
#define	LCDIF_CTRL1_IRQ_ON_ALTERNATE_FIELDS			(1 << 20)
#define	LCDIF_CTRL1_BYTE_PACKING_FORMAT_MASK			(0xf << 16)
#define	LCDIF_CTRL1_BYTE_PACKING_FORMAT_OFFSET			16
#define	LCDIF_CTRL1_OVERFLOW_IRQ_EN				(1 << 15)
#define	LCDIF_CTRL1_UNDERFLOW_IRQ_EN				(1 << 14)
#define	LCDIF_CTRL1_CUR_FRAME_DONE_IRQ_EN			(1 << 13)
#define	LCDIF_CTRL1_VSYNC_EDGE_IRQ_EN				(1 << 12)
#define	LCDIF_CTRL1_OVERFLOW_IRQ				(1 << 11)
#define	LCDIF_CTRL1_UNDERFLOW_IRQ				(1 << 10)
#define	LCDIF_CTRL1_CUR_FRAME_DONE_IRQ				(1 << 9)
#define	LCDIF_CTRL1_VSYNC_EDGE_IRQ				(1 << 8)
#define	LCDIF_CTRL1_BUSY_ENABLE					(1 << 2)
#define	LCDIF_CTRL1_MODE86					(1 << 1)
#define	LCDIF_CTRL1_RESET					(1 << 0)

#define	LCDIF_CTRL2_OUTSTANDING_REQS_MASK			(0x7 << 21)
#define	LCDIF_CTRL2_OUTSTANDING_REQS_OFFSET			21
#define	LCDIF_CTRL2_OUTSTANDING_REQS_REQ_1			(0x0 << 21)
#define	LCDIF_CTRL2_OUTSTANDING_REQS_REQ_2			(0x1 << 21)
#define	LCDIF_CTRL2_OUTSTANDING_REQS_REQ_4			(0x2 << 21)
#define	LCDIF_CTRL2_OUTSTANDING_REQS_REQ_8			(0x3 << 21)
#define	LCDIF_CTRL2_OUTSTANDING_REQS_REQ_16			(0x4 << 21)
#define	LCDIF_CTRL2_BURST_LEN_8					(1 << 20)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_MASK			(0x7 << 16)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_OFFSET			16
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_RGB			(0x0 << 16)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_RBG			(0x1 << 16)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_GBR			(0x2 << 16)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_GRB			(0x3 << 16)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_BRG			(0x4 << 16)
#define	LCDIF_CTRL2_ODD_LINE_PATTERN_BGR			(0x5 << 16)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_MASK			(0x7 << 12)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_OFFSET			12
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_RGB			(0x0 << 12)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_RBG			(0x1 << 12)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_GBR			(0x2 << 12)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_GRB			(0x3 << 12)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_BRG			(0x4 << 12)
#define	LCDIF_CTRL2_EVEN_LINE_PATTERN_BGR			(0x5 << 12)
#define	LCDIF_CTRL2_READ_PACK_DIR				(1 << 10)
#define	LCDIF_CTRL2_READ_MODE_OUTPUT_IN_RGB_FORMAT		(1 << 9)
#define	LCDIF_CTRL2_READ_MODE_6_BIT_INPUT			(1 << 8)
#define	LCDIF_CTRL2_READ_MODE_NUM_PACKED_SUBWORDS_MASK		(0x7 << 4)
#define	LCDIF_CTRL2_READ_MODE_NUM_PACKED_SUBWORDS_OFFSET	4
#define	LCDIF_CTRL2_INITIAL_DUMMY_READ_MASK			(0x7 << 1)
#define	LCDIF_CTRL2_INITIAL_DUMMY_READ_OFFSET			1

#define	LCDIF_TRANSFER_COUNT_V_COUNT_MASK			(0xffff << 16)
#define	LCDIF_TRANSFER_COUNT_V_COUNT_OFFSET			16
#define	LCDIF_TRANSFER_COUNT_H_COUNT_MASK			(0xffff << 0)
#define	LCDIF_TRANSFER_COUNT_H_COUNT_OFFSET			0

#define	LCDIF_CUR_BUF_ADDR_MASK					0xffffffff
#define	LCDIF_CUR_BUF_ADDR_OFFSET				0

#define	LCDIF_NEXT_BUF_ADDR_MASK				0xffffffff
#define	LCDIF_NEXT_BUF_ADDR_OFFSET				0

#define	LCDIF_TIMING_CMD_HOLD_MASK				(0xff << 24)
#define	LCDIF_TIMING_CMD_HOLD_OFFSET				24
#define	LCDIF_TIMING_CMD_SETUP_MASK				(0xff << 16)
#define	LCDIF_TIMING_CMD_SETUP_OFFSET				16
#define	LCDIF_TIMING_DATA_HOLD_MASK				(0xff << 8)
#define	LCDIF_TIMING_DATA_HOLD_OFFSET				8
#define	LCDIF_TIMING_DATA_SETUP_MASK				(0xff << 0)
#define	LCDIF_TIMING_DATA_SETUP_OFFSET				0

#define	LCDIF_VDCTRL0_VSYNC_OEB					(1 << 29)
#define	LCDIF_VDCTRL0_ENABLE_PRESENT				(1 << 28)
#define	LCDIF_VDCTRL0_VSYNC_POL					(1 << 27)
#define	LCDIF_VDCTRL0_HSYNC_POL					(1 << 26)
#define	LCDIF_VDCTRL0_DOTCLK_POL				(1 << 25)
#define	LCDIF_VDCTRL0_ENABLE_POL				(1 << 24)
#define	LCDIF_VDCTRL0_VSYNC_PERIOD_UNIT				(1 << 21)
#define	LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_UNIT			(1 << 20)
#define	LCDIF_VDCTRL0_HALF_LINE					(1 << 19)
#define	LCDIF_VDCTRL0_HALF_LINE_MODE				(1 << 18)
#define	LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_MASK			0x3ffff
#define	LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_OFFSET			0

#define	LCDIF_VDCTRL1_VSYNC_PERIOD_MASK				0xffffffff
#define	LCDIF_VDCTRL1_VSYNC_PERIOD_OFFSET			0

#if defined(CONFIG_MX23)
#define	LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH_MASK			(0xff << 24)
#define	LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH_OFFSET			24
#else
#define	LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH_MASK			(0x3fff << 18)
#define	LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH_OFFSET			18
#endif
#define	LCDIF_VDCTRL2_HSYNC_PERIOD_MASK				0x3ffff
#define	LCDIF_VDCTRL2_HSYNC_PERIOD_OFFSET			0

#define	LCDIF_VDCTRL3_MUX_SYNC_SIGNALS				(1 << 29)
#define	LCDIF_VDCTRL3_VSYNC_ONLY				(1 << 28)
#define	LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT_MASK			(0xfff << 16)
#define	LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT_OFFSET		16
#define	LCDIF_VDCTRL3_VERTICAL_WAIT_CNT_MASK			(0xffff << 0)
#define	LCDIF_VDCTRL3_VERTICAL_WAIT_CNT_OFFSET			0

#define	LCDIF_VDCTRL4_DOTCLK_DLY_SEL_MASK			(0x7 << 29)
#define	LCDIF_VDCTRL4_DOTCLK_DLY_SEL_OFFSET			29
#define	LCDIF_VDCTRL4_SYNC_SIGNALS_ON				(1 << 18)
#define	LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT_MASK		0x3ffff
#define	LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT_OFFSET		0

#endif /* __IMX_REGS_LCDIF_H__ */
