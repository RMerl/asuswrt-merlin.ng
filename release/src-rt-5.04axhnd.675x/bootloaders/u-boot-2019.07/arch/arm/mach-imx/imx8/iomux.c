// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sci/sci.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * configures a single pad in the iomuxer
 */
void imx8_iomux_setup_pad(iomux_cfg_t pad)
{
	sc_pad_t pin_id = pad & PIN_ID_MASK;
	int ret;

	u32 val = (u32)((pad & MUX_PAD_CTRL_MASK) >> MUX_PAD_CTRL_SHIFT);

	val |= PADRING_IFMUX_EN_MASK;
	val |= PADRING_GP_EN_MASK;

	ret = sc_pad_set(-1, pin_id, val);
	if (ret)
		printf("sc_pad_set failed!, pin: %u, val: 0x%x\n", pin_id, val);

	debug("iomux: pin %d, val = 0x%x\n", pin_id, val);
}

/* configures a list of pads within declared with IOMUX_PADS macro */
void imx8_iomux_setup_multiple_pads(iomux_cfg_t const *pad_list, u32 count)
{
	iomux_cfg_t const *p = pad_list;
	int i;

	for (i = 0; i < count; i++) {
		imx8_iomux_setup_pad(*p);
		p++;
	}
}
