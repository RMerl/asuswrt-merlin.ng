/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on Linux i.MX iomux-v3.h file:
 * Copyright (C) 2009 by Jan Weitzel Phytec Messtechnik GmbH,
 *			<armlinux@phytec.de>
 *
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#ifndef __MACH_IOMUX_H__
#define __MACH_IOMUX_H__

/*
 *	build IOMUX_PAD structure
 *
 * This iomux scheme is based around pads, which are the physical balls
 * on the processor.
 *
 * - Each pad has a pad control register (IOMUXC_SW_PAD_CTRL_x) which controls
 *   things like driving strength and pullup/pulldown.
 * - Each pad can have but not necessarily does have an output routing register
 *   (IOMUXC_SW_MUX_CTL_PAD_x).
 * - Each pad can have but not necessarily does have an input routing register
 *   (IOMUXC_x_SELECT_INPUT)
 *
 * The three register sets do not have a fixed offset to each other,
 * hence we order this table by pad control registers (which all pads
 * have) and put the optional i/o routing registers into additional
 * fields.
 *
 * The naming convention for the pad modes is SOC_PAD_<padname>__<padmode>
 * If <padname> or <padmode> refers to a GPIO, it is named GPIO_<unit>_<num>
 *
 * IOMUX/PAD Bit field definitions
 *
 * MUX_CTRL_OFS:	                0..15 (16)
 * SEL_INPUT_OFS:	               16..31 (16)
 * MUX_MODE:	                   32..37  (6)
 * SEL_INP:		                   38..41  (4)
 * PAD_CTRL + NO_PAD_CTRL:         42..60 (19)
 * reserved:		               61-63      (3)
*/

typedef u64 iomux_cfg_t;

#define MUX_CTRL_OFS_SHIFT	     0
#define MUX_CTRL_OFS_MASK	((iomux_cfg_t)0xffff << MUX_CTRL_OFS_SHIFT)
#define MUX_SEL_INPUT_OFS_SHIFT	16
#define MUX_SEL_INPUT_OFS_MASK	((iomux_cfg_t)0xffff << \
	MUX_SEL_INPUT_OFS_SHIFT)

#define MUX_MODE_SHIFT		32
#define MUX_MODE_MASK		((iomux_cfg_t)0x3f << MUX_MODE_SHIFT)
#define MUX_SEL_INPUT_SHIFT	38
#define MUX_SEL_INPUT_MASK	((iomux_cfg_t)0xf << MUX_SEL_INPUT_SHIFT)
#define MUX_PAD_CTRL_SHIFT	42
#define MUX_PAD_CTRL_MASK	((iomux_cfg_t)0x7ffff << MUX_PAD_CTRL_SHIFT)

#define MUX_PAD_CTRL(x)		((iomux_cfg_t)(x) << MUX_PAD_CTRL_SHIFT)

#define IOMUX_PAD(pad_ctrl_ofs, mux_ctrl_ofs, mux_mode, sel_input_ofs,	\
		sel_input, pad_ctrl)					\
	(((iomux_cfg_t)(mux_ctrl_ofs) << MUX_CTRL_OFS_SHIFT)     |	\
	((iomux_cfg_t)(mux_mode)      << MUX_MODE_SHIFT)         |	\
	((iomux_cfg_t)(pad_ctrl)      << MUX_PAD_CTRL_SHIFT)     |	\
	((iomux_cfg_t)(sel_input_ofs) << MUX_SEL_INPUT_OFS_SHIFT)|	\
	((iomux_cfg_t)(sel_input)     << MUX_SEL_INPUT_SHIFT))

#define NEW_PAD_CTRL(cfg, pad)	(((cfg) & ~MUX_PAD_CTRL_MASK) | \
					MUX_PAD_CTRL(pad))


#define IOMUX_CONFIG_MPORTS       0x20
#define MUX_MODE_MPORTS           ((iomux_v3_cfg_t)IOMUX_CONFIG_MPORTS << \
				MUX_MODE_SHIFT)

/* Bit definition below needs to be fixed acccording to ulp rm */

#define NO_PAD_CTRL		     (1 << 18)
#define PAD_CTL_OBE_ENABLE	 (1 << 17)
#define PAD_CTL_IBE_ENABLE	 (1 << 16)
#define PAD_CTL_DSE          (1 << 6)
#define PAD_CTL_ODE          (1 << 5)
#define PAD_CTL_SRE_FAST     (0 << 2)
#define PAD_CTL_SRE_SLOW     (1 << 2)
#define PAD_CTL_PUE          (1 << 1)
#define PAD_CTL_PUS_UP       ((1 << 0) | PAD_CTL_PUE)
#define PAD_CTL_PUS_DOWN     ((0 << 0) | PAD_CTL_PUE)


void mx7ulp_iomux_setup_pad(iomux_cfg_t pad);
void mx7ulp_iomux_setup_multiple_pads(iomux_cfg_t const *pad_list,
				      unsigned count);
#endif	/* __MACH_IOMUX_H__*/
