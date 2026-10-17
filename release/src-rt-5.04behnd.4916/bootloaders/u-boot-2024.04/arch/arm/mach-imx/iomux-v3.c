// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on the iomux-v3.c from Linux kernel:
 * Copyright (C) 2008 by Sascha Hauer <kernel@pengutronix.de>
 * Copyright (C) 2009 by Jan Weitzel Phytec Messtechnik GmbH,
 *                       <armlinux@phytec.de>
 *
 * Copyright (C) 2004-2011 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sys_proto.h>

static void *base = (void *)IOMUXC_BASE_ADDR;

/*
 * configures a single pad in the iomuxer
 */
void imx_iomux_v3_setup_pad(iomux_v3_cfg_t pad)
{
	u32 mux_ctrl_ofs = (pad & MUX_CTRL_OFS_MASK) >> MUX_CTRL_OFS_SHIFT;
	u32 mux_mode = (pad & MUX_MODE_MASK) >> MUX_MODE_SHIFT;
	u32 sel_input_ofs =
		(pad & MUX_SEL_INPUT_OFS_MASK) >> MUX_SEL_INPUT_OFS_SHIFT;
	u32 sel_input =
		(pad & MUX_SEL_INPUT_MASK) >> MUX_SEL_INPUT_SHIFT;
	u32 pad_ctrl_ofs =
		(pad & MUX_PAD_CTRL_OFS_MASK) >> MUX_PAD_CTRL_OFS_SHIFT;
	u32 pad_ctrl = (pad & MUX_PAD_CTRL_MASK) >> MUX_PAD_CTRL_SHIFT;

#if defined(CONFIG_MX6SL) || defined(CONFIG_MX6SLL)
	/* Check whether LVE bit needs to be set */
	if (pad_ctrl & PAD_CTL_LVE) {
		pad_ctrl &= ~PAD_CTL_LVE;
		pad_ctrl |= PAD_CTL_LVE_BIT;
	}
#endif

#ifdef CONFIG_IOMUX_LPSR
	u32 lpsr = (pad & MUX_MODE_LPSR) >> MUX_MODE_SHIFT;

#ifdef CONFIG_MX7
	if (lpsr == IOMUX_CONFIG_LPSR) {
		base = (void *)IOMUXC_LPSR_BASE_ADDR;
		mux_mode &= ~IOMUX_CONFIG_LPSR;
		/* set daisy chain sel_input */
		if (sel_input_ofs)
			sel_input_ofs += IOMUX_LPSR_SEL_INPUT_OFS;
	}
#else
	if (is_mx6ull() || is_mx6sll()) {
		if (lpsr == IOMUX_CONFIG_LPSR) {
			base = (void *)IOMUXC_SNVS_BASE_ADDR;
			mux_mode &= ~IOMUX_CONFIG_LPSR;
		}
	}
#endif
#endif

	if (is_mx7() || is_mx6ull() || is_mx6sll() || mux_ctrl_ofs)
		__raw_writel(mux_mode, base + mux_ctrl_ofs);

	if (sel_input_ofs)
		__raw_writel(sel_input, base + sel_input_ofs);

#ifdef CONFIG_IOMUX_SHARE_CONF_REG
	if (!(pad_ctrl & NO_PAD_CTRL))
		__raw_writel((mux_mode << PAD_MUX_MODE_SHIFT) | pad_ctrl,
			base + pad_ctrl_ofs);
#else
	if (!(pad_ctrl & NO_PAD_CTRL) && pad_ctrl_ofs)
		__raw_writel(pad_ctrl, base + pad_ctrl_ofs);
#if defined(CONFIG_MX6SLL)
	else if ((pad_ctrl & NO_PAD_CTRL) && pad_ctrl_ofs)
		clrbits_le32(base + pad_ctrl_ofs, PAD_CTL_IPD_BIT);
#endif
#endif

#ifdef CONFIG_IOMUX_LPSR
	if (lpsr == IOMUX_CONFIG_LPSR)
		base = (void *)IOMUXC_BASE_ADDR;
#endif

}

/* configures a list of pads within declared with IOMUX_PADS macro */
void imx_iomux_v3_setup_multiple_pads(iomux_v3_cfg_t const *pad_list,
				      unsigned count)
{
	iomux_v3_cfg_t const *p = pad_list;
	int stride;
	int i;

#if defined(CONFIG_MX6QDL)
	stride = 2;
	if (!is_mx6dq() && !is_mx6dqp())
		p += 1;
#else
	stride = 1;
#endif
	for (i = 0; i < count; i++) {
		imx_iomux_v3_setup_pad(*p);
		p += stride;
	}
}

void imx_iomux_set_gpr_register(int group, int start_bit,
					int num_bits, int value)
{
	int i = 0;
	u32 reg;
	reg = readl(base + group * 4);
	while (num_bits) {
		reg &= ~(1<<(start_bit + i));
		i++;
		num_bits--;
	}
	reg |= (value << start_bit);
	writel(reg, base + group * 4);
}

#ifdef CONFIG_IOMUX_SHARE_CONF_REG
void imx_iomux_gpio_set_direction(unsigned int gpio,
				unsigned int direction)
{
	u32 reg;
	/*
	 * Only on Vybrid the input/output buffer enable flags
	 * are part of the shared mux/conf register.
	 */
	reg = readl(base + (gpio << 2));

	if (direction)
		reg |= 0x2;
	else
		reg &= ~0x2;

	writel(reg, base + (gpio << 2));
}

void imx_iomux_gpio_get_function(unsigned int gpio, u32 *gpio_state)
{
	*gpio_state = readl(base + (gpio << 2)) &
		((0X07 << PAD_MUX_MODE_SHIFT) | PAD_CTL_OBE_IBE_ENABLE);
}
#endif
