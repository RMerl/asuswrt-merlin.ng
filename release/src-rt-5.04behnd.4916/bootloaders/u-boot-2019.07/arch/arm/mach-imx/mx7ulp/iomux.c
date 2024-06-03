// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>

static void *base = (void *)IOMUXC_BASE_ADDR;

/*
 * iomuxc0 base address. In imx7ulp-pins.h,
 * the offsets of pins in iomuxc0 are from 0xD000,
 * so we set the base address to (0x4103D000 - 0xD000 = 0x41030000)
 */
static void *base_mports = (void *)(AIPS0_BASE + 0x30000);

/*
 * configures a single pad in the iomuxer
 */
void mx7ulp_iomux_setup_pad(iomux_cfg_t pad)
{
	u32 mux_ctrl_ofs = (pad & MUX_CTRL_OFS_MASK) >> MUX_CTRL_OFS_SHIFT;
	u32 mux_mode = (pad & MUX_MODE_MASK) >> MUX_MODE_SHIFT;
	u32 sel_input_ofs =
		(pad & MUX_SEL_INPUT_OFS_MASK) >> MUX_SEL_INPUT_OFS_SHIFT;
	u32 sel_input =
		(pad & MUX_SEL_INPUT_MASK) >> MUX_SEL_INPUT_SHIFT;
	u32 pad_ctrl_ofs = mux_ctrl_ofs;
	u32 pad_ctrl = (pad & MUX_PAD_CTRL_MASK) >> MUX_PAD_CTRL_SHIFT;

	debug("[PAD CFG] = 0x%16llX \r\n\tmux_ctl = 0x%X(0x%X) sel_input = 0x%X(0x%X) pad_ctrl = 0x%X(0x%X)\r\n",
	      pad, mux_ctrl_ofs, mux_mode, sel_input_ofs, sel_input,
	      pad_ctrl_ofs, pad_ctrl);

	if (mux_mode & IOMUX_CONFIG_MPORTS) {
		mux_mode &= ~IOMUX_CONFIG_MPORTS;
		base = base_mports;
	} else {
		base = (void *)IOMUXC_BASE_ADDR;
	}

	__raw_writel(((mux_mode << IOMUXC_PCR_MUX_ALT_SHIFT) &
		     IOMUXC_PCR_MUX_ALT_MASK), base + mux_ctrl_ofs);

	if (sel_input_ofs)
		__raw_writel((sel_input << IOMUXC_PSMI_IMUX_ALT_SHIFT),
			base + sel_input_ofs);

	if (!(pad_ctrl & NO_PAD_CTRL))
		__raw_writel(((mux_mode << IOMUXC_PCR_MUX_ALT_SHIFT) &
			     IOMUXC_PCR_MUX_ALT_MASK) |
			     (pad_ctrl & (~IOMUXC_PCR_MUX_ALT_MASK)),
			     base + pad_ctrl_ofs);
}

/* configures a list of pads within declared with IOMUX_PADS macro */
void mx7ulp_iomux_setup_multiple_pads(iomux_cfg_t const *pad_list,
				      unsigned count)
{
	iomux_cfg_t const *p = pad_list;
	int i;

	for (i = 0; i < count; i++) {
		mx7ulp_iomux_setup_pad(*p);
		p++;
	}
}
