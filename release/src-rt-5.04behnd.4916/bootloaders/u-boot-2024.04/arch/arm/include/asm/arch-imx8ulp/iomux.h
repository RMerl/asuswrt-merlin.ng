/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __MACH_IMX8ULP_IOMUX_H__
#define __MACH_IMX8ULP_IOMUX_H__

typedef u64 iomux_cfg_t;

#define MUX_CTRL_OFS_SHIFT	0
#define MUX_CTRL_OFS_MASK	((iomux_cfg_t)0xffff << MUX_CTRL_OFS_SHIFT)
#define MUX_SEL_INPUT_OFS_SHIFT	16
#define MUX_SEL_INPUT_OFS_MASK	((iomux_cfg_t)0xffff << MUX_SEL_INPUT_OFS_SHIFT)

#define MUX_MODE_SHIFT		32
#define MUX_MODE_MASK		((iomux_cfg_t)0x3f << MUX_MODE_SHIFT)
#define MUX_SEL_INPUT_SHIFT	38
#define MUX_SEL_INPUT_MASK	((iomux_cfg_t)0xf << MUX_SEL_INPUT_SHIFT)
#define MUX_PAD_CTRL_SHIFT	42
#define MUX_PAD_CTRL_MASK	((iomux_cfg_t)0x7ffff << MUX_PAD_CTRL_SHIFT)

#define MUX_PAD_CTRL(x)		((iomux_cfg_t)(x) << MUX_PAD_CTRL_SHIFT)

#define IOMUX_PAD(pad_ctrl_ofs, mux_ctrl_ofs, mux_mode, sel_input_ofs, sel_input, pad_ctrl) \
	(((iomux_cfg_t)(mux_ctrl_ofs) << MUX_CTRL_OFS_SHIFT)     |	\
	((iomux_cfg_t)(mux_mode)      << MUX_MODE_SHIFT)         |	\
	((iomux_cfg_t)(pad_ctrl)      << MUX_PAD_CTRL_SHIFT)     |	\
	((iomux_cfg_t)(sel_input_ofs) << MUX_SEL_INPUT_OFS_SHIFT) |	\
	((iomux_cfg_t)(sel_input)     << MUX_SEL_INPUT_SHIFT))

#define NEW_PAD_CTRL(cfg, pad)	(((cfg) & ~MUX_PAD_CTRL_MASK) | MUX_PAD_CTRL(pad))

#define IOMUX_CONFIG_MPORTS       0x20
#define MUX_MODE_MPORTS           ((iomux_v3_cfg_t)IOMUX_CONFIG_MPORTS << \ MUX_MODE_SHIFT)

/* Bit definition below needs to be fixed acccording to ulp rm */

#define NO_PAD_CTRL		BIT(18)
#define PAD_CTL_OBE_ENABLE	BIT(17)
#define PAD_CTL_IBE_ENABLE      BIT(16)
#define PAD_CTL_DSE		BIT(6)
#define PAD_CTL_ODE		BIT(5)
#define PAD_CTL_SRE_FAST	(0 << 2)
#define PAD_CTL_SRE_SLOW	BIT(2)
#define PAD_CTL_PUE		BIT(1)
#define PAD_CTL_PUS_UP		(BIT(0) | PAD_CTL_PUE)
#define PAD_CTL_PUS_DOWN	((0 << 0) | PAD_CTL_PUE)

#define IOMUXC_PCR_MUX_ALT0		(0 << 8)
#define IOMUXC_PCR_MUX_ALT1		(1 << 8)
#define IOMUXC_PCR_MUX_ALT2		(2 << 8)
#define IOMUXC_PCR_MUX_ALT3		(3 << 8)
#define IOMUXC_PCR_MUX_ALT4		(4 << 8)
#define IOMUXC_PCR_MUX_ALT5		(5 << 8)
#define IOMUXC_PCR_MUX_ALT6		(6 << 8)
#define IOMUXC_PCR_MUX_ALT7		(7 << 8)
#define IOMUXC_PCR_MUX_ALT8		(8 << 8)
#define IOMUXC_PCR_MUX_ALT9		(9 << 8)
#define IOMUXC_PCR_MUX_ALT10		(10 << 8)
#define IOMUXC_PCR_MUX_ALT11		(11 << 8)
#define IOMUXC_PCR_MUX_ALT12		(12 << 8)
#define IOMUXC_PCR_MUX_ALT13		(13 << 8)
#define IOMUXC_PCR_MUX_ALT14		(14 << 8)
#define IOMUXC_PCR_MUX_ALT15		(15 << 8)

#define IOMUXC_PSMI_IMUX_ALT0		(0x0)
#define IOMUXC_PSMI_IMUX_ALT1		(0x1)
#define IOMUXC_PSMI_IMUX_ALT2		(0x2)
#define IOMUXC_PSMI_IMUX_ALT3		(0x3)
#define IOMUXC_PSMI_IMUX_ALT4		(0x4)
#define IOMUXC_PSMI_IMUX_ALT5		(0x5)
#define IOMUXC_PSMI_IMUX_ALT6		(0x6)
#define IOMUXC_PSMI_IMUX_ALT7		(0x7)

#define IOMUXC_PCR_MUX_ALT_SHIFT	(8)
#define IOMUXC_PCR_MUX_ALT_MASK	(0xF00)
#define IOMUXC_PSMI_IMUX_ALT_SHIFT	(0)

void imx8ulp_iomux_setup_pad(iomux_cfg_t pad);
void imx8ulp_iomux_setup_multiple_pads(iomux_cfg_t const *pad_list, unsigned int count);
#endif
