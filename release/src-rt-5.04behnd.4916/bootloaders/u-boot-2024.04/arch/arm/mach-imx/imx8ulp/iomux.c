// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>

static void *base = (void *)IOMUXC_BASE_ADDR;
static void *base_mports = (void *)(0x280A1000);

/*
 * configures a single pad in the iomuxer
 */
void imx8ulp_iomux_setup_pad(iomux_cfg_t pad)
{
	u32 mux_ctrl_ofs = (pad & MUX_CTRL_OFS_MASK) >> MUX_CTRL_OFS_SHIFT;
	u32 mux_mode = (pad & MUX_MODE_MASK) >> MUX_MODE_SHIFT;
	u32 sel_input_ofs =
		(pad & MUX_SEL_INPUT_OFS_MASK) >> MUX_SEL_INPUT_OFS_SHIFT;
	u32 sel_input =
		(pad & MUX_SEL_INPUT_MASK) >> MUX_SEL_INPUT_SHIFT;
	u32 pad_ctrl_ofs = mux_ctrl_ofs;
	u32 pad_ctrl = (pad & MUX_PAD_CTRL_MASK) >> MUX_PAD_CTRL_SHIFT;

	if (mux_mode & IOMUX_CONFIG_MPORTS) {
		mux_mode &= ~IOMUX_CONFIG_MPORTS;
		base = base_mports;
	} else {
		base = (void *)IOMUXC_BASE_ADDR;
	}

	__raw_writel(((mux_mode << IOMUXC_PCR_MUX_ALT_SHIFT) &
		     IOMUXC_PCR_MUX_ALT_MASK), base + mux_ctrl_ofs);

	if (sel_input_ofs)
		__raw_writel((sel_input << IOMUXC_PSMI_IMUX_ALT_SHIFT), base + sel_input_ofs);

	if (!(pad_ctrl & NO_PAD_CTRL))
		__raw_writel(((mux_mode << IOMUXC_PCR_MUX_ALT_SHIFT) &
			     IOMUXC_PCR_MUX_ALT_MASK) |
			     (pad_ctrl & (~IOMUXC_PCR_MUX_ALT_MASK)),
			     base + pad_ctrl_ofs);
}

/* configures a list of pads within declared with IOMUX_PADS macro */
void imx8ulp_iomux_setup_multiple_pads(iomux_cfg_t const *pad_list, u32 count)
{
	iomux_cfg_t const *p = pad_list;
	int i;

	for (i = 0; i < count; i++) {
		imx8ulp_iomux_setup_pad(*p);
		p++;
	}
}
